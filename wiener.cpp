#include <iostream>
#include <stdio.h>
#include <cassert>
#include <NTL/ZZ.h>
#include <vector>
#include <chrono>

NTL_CLIENT

#define assertm(exp, msg) assert(((void)msg, exp))
#define DEBUG 0


vector<ZZ> oblicz_wartosc_ulamka_lancuchowego(ZZ e, ZZ N)
{   
    vector<ZZ> wartosc_ulamka_lancuchowego;
    ZZ licznik, mianownik, iloraz, tmp;

    licznik = e;
    mianownik = N;

    while (!IsZero(mianownik))
    {
        iloraz = licznik / mianownik;
        wartosc_ulamka_lancuchowego.push_back(iloraz);
        tmp = licznik;
        licznik = mianownik;
        mianownik = tmp - (mianownik * iloraz); 
    }
    if(DEBUG)
    {
        for (int i = 0; i < wartosc_ulamka_lancuchowego.size(); i++)
        {
            cout << wartosc_ulamka_lancuchowego[i] << endl;
        }
    }
    return wartosc_ulamka_lancuchowego;
}


vector<ZZ> attack(ZZ e, ZZ N)
{
    vector<ZZ> P;
    vector<ZZ> Q;
    vector<ZZ> wartosc_ulamka = oblicz_wartosc_ulamka_lancuchowego(e, N);
    vector<ZZ> wynik;
    ZZ k, d, phiN, a, b, c, delta, p1, p2, q;


    for (int i = 0; i < wartosc_ulamka.size(); i++)
    {
        // Wyznacz P i Q dla tej iteracji
        if (i == 0)
        {
            P.push_back(wartosc_ulamka[i]);
            Q.push_back(conv<ZZ>("1"));
        }
        else if (i == 1)
        {
            P.push_back((wartosc_ulamka[i] * wartosc_ulamka[i - 1]) + 1);
            Q.push_back(wartosc_ulamka[i]);
        }
        else
        {
            P.push_back((wartosc_ulamka[i] * P[i - 1]) + P[i - 2]);
            Q.push_back((wartosc_ulamka[i] * Q[i - 1]) + Q[i - 2]);
        }
        // Sprawdzanie czy przy tej iteracji wyznaczone P i Q są szukanymi k i d 
        k = P[i];
        d = Q[i];
        if (!IsZero(k))
        {
            phiN = (e * d) - 1;
            if (divide(phiN, k))
            {
                phiN /= k;
                // Wyznaczenie delty równania
                a = 1;
                b = -((N - phiN) + 1);
                c = N;
                delta = power(b, 2) - 4 * (a * c); 
                // Wyznaczanie pierwiastków równania na podstawie delty
                if (sign(delta) > 0)
                {
                    p1 = -b + SqrRoot(delta);
                    p2 = -b - SqrRoot(delta);
                    if (divide(p1, 2*a)) // Czy pierwszy pierwiastek jest całkowity
                    {
                        p1 /= 2*a;
                        if (sign(p1) > 0 && divide(N, p1))
                        {
                            q = N / p1;
                            //cout << d << endl;
                            wynik.push_back(q);
                            wynik.push_back(d);
                            return wynik;
                        }
                    }
                    else if (divide(p2, 2*a)) // Czy drugi pierwiastek jest całkowity
                    {
                        p2 /= 2*a;
                        if (sign(p2) > 0 && divide(N, p2))
                        {
                            q = N / p2;
                            wynik.push_back(q);
                            wynik.push_back(d);
                            return wynik;
                        }
                    }
                }
                else if (sign(delta) == 0)
                {
                    if (divide(-b, 2*a)) // Czy pierwiastek jest całkowity
                    {
                        p1 /= 2*a;
                        if (sign(p1) > 0 && divide(N, p1))
                        {
                            q = N / p1;
                            wynik.push_back(q);
                            wynik.push_back(d);
                            return wynik;
                        }
                    }
                }
            }
        }
    
    }

    printf("Nic nie znalazlem :(\n");
    if (DEBUG)
    {
        cout << "Q = [ ";
        for (int i = 0; i < Q.size(); i++)
        {
            cout << Q[i] <<" ";
        }
        cout << "]" << endl << "P = [ ";
        for (int i = 0; i < P.size(); i++)
        {
            cout << P[i] << " ";
        }
        cout << "]" << endl << "a = [ ";
        for (int i = 0; i < wartosc_ulamka.size(); i++)
        {
            cout << wartosc_ulamka[i] << " ";
        }
        cout << "]" << endl;
    }
    wynik.push_back(conv<ZZ>("0"));
    return wynik;
}

int main(int argc, char *argv[])
{
    assertm(argc == 3, "Niepoprawna liczba argumentow");
    ZZ e = conv<ZZ>(argv[1]); // wykladnik publiczny 
    ZZ N = conv<ZZ>(argv[2]); // modulnik publiczny
    vector<ZZ> wynik;
    ZZ d, q, p, phiN;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    wynik = attack(e, N);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    q = wynik[0];
    d = wynik[1];
    p = N / q;
    phiN = (p - 1) * (q - 1);

    assertm(MulMod(e, d, phiN) == conv<ZZ>("1"), "Niepoprawny wynik");
    std::cout << "Time = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
    return 0;    
}