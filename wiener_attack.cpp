/**
 * @file wiener_attack.cpp
 * @brief Wiener's attack on RSA with small private exponent using NTL (ZZ).
 *
 * This program attempts to recover the private exponent (d) or a factor (q)
 * of an RSA modulus N given the public exponent e, using continued fractions
 * and convergents (Wiener's attack).
 *
 * Usage:
 *   ./wiener_attack <e> <N>
 *
 * Example:
 *   ./wiener_attack 17993 90581
 *
 *
 * @author kpochodyla
 */

#include <iostream>
#include <vector>
#include <optional>
#include <chrono>
#include <string>

#include <NTL/ZZ.h>

using namespace std;
using namespace NTL;

/**
 * @brief Compute the simple continued fraction terms of the rational e/N.
 *
 * The algorithm repeatedly divides numerator by denominator:
 *   q = num / den; rem = num - q * den; then swap(num=den, den=rem)
 *
 * @param e The numerator (public exponent).
 * @param N The denominator (modulus).
 * @return Vector of ZZ terms representing the continued fraction expansion.
 */
vector<ZZ> computeContinuedFraction(const ZZ &e, const ZZ &N) {
    vector<ZZ> cf;
    ZZ num = e;
    ZZ den = N;

    while (!IsZero(den)) {
        ZZ q = num / den;
        cf.push_back(q);
        ZZ rem = num - q * den;
        num = den;
        den = rem;
    }
    return cf;
}

/**
 * @brief Build convergents P (numerators) and Q (denominators) from CF terms.
 *
 * Given continued fraction terms a_0, a_1, ..., the convergents are:
 *   P_0 = a_0, Q_0 = 1
 *   P_1 = a_1*a_0 + 1, Q_1 = a_1
 *   P_n = a_n*P_{n-1} + P_{n-2}
 *   Q_n = a_n*Q_{n-1} + Q_{n-2}
 *
 * @param cf Input continued fraction terms.
 * @param P [out] Numerator convergents (P[i]).
 * @param Q [out] Denominator convergents (Q[i]).
 */
void buildConvergents(const vector<ZZ> &cf, vector<ZZ> &P, vector<ZZ> &Q) {
    P.clear();
    Q.clear();
    P.reserve(cf.size());
    Q.reserve(cf.size());

    for (size_t i = 0; i < cf.size(); ++i) {
        if (i == 0) {
            P.push_back(cf[0]);
            Q.push_back(ZZ(1));
        } else if (i == 1) {
            P.push_back(cf[1] * cf[0] + 1);
            Q.push_back(cf[1]);
        } else {
            P.push_back(cf[i] * P[i - 1] + P[i - 2]);
            Q.push_back(cf[i] * Q[i - 1] + Q[i - 2]);
        }
    }
}

/**
 * @brief Check whether a ZZ value is a perfect square.
 *
 * @param n Input value to test.
 * @param root [out] If true, root will contain the integer sqrt(n).
 * @return true if n is a perfect square, false otherwise.
 */
bool isPerfectSquare(const ZZ &n, ZZ &root) {
    if (n < 0) return false;
    root = SqrRoot(n);
    return (root * root == n);
}

/**
 * @brief Attempt to recover a factor q (and the private exponent d) from a convergent (k, d).
 *
 * For each convergent k/d we check whether (e*d - 1) is divisible by k. If so
 * phi(N) = (e*d - 1)/k is candidate Euler totient. Solve quadratic:
 *    x^2 - (N - phiN + 1) * x + N = 0
 * If the discriminant is a perfect square and the resulting root divides N,
 * we have recovered a factor q (root) and the d is likely correct.
 *
 * @param k numerator of convergent (candidate k)
 * @param d denominator of convergent (candidate d)
 * @param e RSA public exponent
 * @param N RSA modulus
 * @return optional<pair<q, d>> if successful, nullopt otherwise
 */
optional<pair<ZZ, ZZ>> tryRecoverFromConvergent(const ZZ &k, const ZZ &d, const ZZ &e, const ZZ &N) {
    if (IsZero(k)) return nullopt;

    // Check divisibility
    ZZ numer = e * d - 1;
    if (!divide(numer, k)) return nullopt;
    ZZ phiN = numer / k;

    // Quadratic coefficients for x^2 - ((N - phiN) + 1) x + N = 0
    ZZ a = ZZ(1);
    ZZ b = -((N - phiN) + 1);
    ZZ c = N;

    ZZ delta = b * b - 4 * a * c;
    if (delta < 0) return nullopt;

    ZZ sqrtDelta;
    if (!isPerfectSquare(delta, sqrtDelta)) return nullopt;

    ZZ twoA = 2 * a;
    ZZ r1 = -b + sqrtDelta;
    ZZ r2 = -b - sqrtDelta;

    for (ZZ rootCand : {r1, r2}) {
        if (!divide(rootCand, twoA)) continue;
        ZZ root = rootCand / twoA;
        if (root <= 0) continue;
        if (!divide(N, root)) continue;
        ZZ q = root;
        return make_pair(q, d);
    }

    return nullopt;
}

/**
 * @brief Perform Wiener's attack: try all convergents until a valid (q, d) is found.
 *
 * @param e Public exponent.
 * @param N RSA modulus.
 * @return optional<pair<q, d>> if successful; nullopt otherwise.
 */
[[nodiscard]] optional<pair<ZZ, ZZ>> wienerAttack(const ZZ &e, const ZZ &N) {
    auto cf = computeContinuedFraction(e, N);
    if (cf.empty()) return nullopt;

    vector<ZZ> P, Q;
    buildConvergents(cf, P, Q);

    for (size_t i = 0; i < P.size(); ++i) {
        ZZ k = P[i];
        ZZ d = Q[i];
        auto res = tryRecoverFromConvergent(k, d, e, N);
        if (res.has_value()) return res;
    }
    return nullopt;
}

/**
 * @brief Print usage helper.
 * @param prog Program name (argv[0]).
 */
void printUsage(const string &prog) {
    cerr << "Usage: " << prog << " <e> <N>\n"
         << "Both e and N are integers. Example:\n"
         << "  " << prog << " 17993 90581\n";
}

/**
 * @brief Main entry point: parse args, run attack, and print results.
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }

    ZZ e, N;
    try {
        e = conv<ZZ>(argv[1]);
        N = conv<ZZ>(argv[2]);
    } catch (...) {
        cerr << "Error: invalid integer input.\n";
        return 2;
    }

    if (e <= 0 || N <= 0) {
        cerr << "Error: e and N must be positive integers.\n";
        return 2;
    }

    auto t0 = chrono::steady_clock::now();
    auto result = wienerAttack(e, N);
    auto t1 = chrono::steady_clock::now();

    if (!result.has_value()) {
        cerr << "Wiener attack failed: no (q,d) pair found.\n";
        return 3;
    }

    ZZ q = result->first;
    ZZ d = result->second;
    if (q == 0 || d == 0) {
        cerr << "Wiener attack returned invalid values.\n";
        return 3;
    }

    if (!divide(N, q)) {
        cerr << "Recovered q does not divide N. Abort.\n";
        return 4;
    }
    ZZ p = N / q;
    ZZ phiN = (p - 1) * (q - 1);

    ZZ check = MulMod(e, d, phiN);
    if (check != 1) {
        cerr << "Verification failed: e * d mod phi(N) != 1. (value = " << check << ")\n";
        return 5;
    }

    cout << "Wiener attack successful!\n";
    cout << "p = " << p << "\n";
    cout << "q = " << q << "\n";
    cout << "d = " << d << "\n";
    cout << "Time (us) = " << chrono::duration_cast<chrono::microseconds>(t1 - t0).count() << "\n";
    return 0;
}
