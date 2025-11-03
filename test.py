#!/usr/bin/env python3
import secrets, subprocess, re, sys
import gmpy2
from gmpy2 import mpz

TIME_RE = re.compile(r'Time.*?(\d+)\s*(?:\[?µs\]?|us)?', re.IGNORECASE)
D_RE = re.compile(r'\bd\s*=\s*(\d+)\b')

def rand_odd(bits):
    return mpz(secrets.randbits(bits - 1) | (1 << (bits - 1)) | 1)

def gen_prime(bits):
    return gmpy2.next_prime(rand_odd(bits))

def fourth_root(n):
    r, _ = gmpy2.iroot(n, 4)
    return r

def one_test(bin_size):
    hb = bin_size // 2
    p = gen_prime(hb)
    q = gen_prime(hb)
    while q == p:
        q = gen_prime(hb)
    N = p * q
    phi = (p - 1) * (q - 1)
    fourth = fourth_root(N)
    wiener = max(mpz(2), fourth // 3)
    d = mpz(2) if wiener <= 2 else mpz(secrets.randbelow(int(wiener - 1)) + 2)
    while gmpy2.gcd(d, phi) != 1:
        d += 1
        if d > wiener:
            d = mpz(2)
    e = gmpy2.invert(d, phi)

    # run binary
    try:
        proc = subprocess.run([BINARY, str(e), str(N)], capture_output=True, text=True, timeout=15)
        out = proc.stdout + "\n" + proc.stderr
    except Exception:
        print(f"{bin_size}\tMISMATCH")
        return

    # parse recovered d and time
    m = D_RE.search(out)
    mtime = TIME_RE.search(out)
    rec = mpz(m.group(1)) if m else None
    time_us = mtime.group(1) if mtime else None

    if rec is not None and rec == d and time_us is not None:
        print(f"{bin_size}\tOK\t{time_us}")
    elif rec is not None and rec == d:
        # recovered d ok, but time not found — still report OK without time
        print(f"{bin_size}\tOK\t-")
    else:
        print(f"{bin_size}\tMISMATCH")

def main():
    import argparse
    p = argparse.ArgumentParser(add_help=False)
    p.add_argument("-b", "--binary", default="./wiener_attack")
    p.add_argument("-n", "--count", type=int, default=100)
    p.add_argument("-s", "--start", type=int, default=64)
    p.add_argument("-e", "--end", type=int, default=8192)
    args, _ = p.parse_known_args()
    global BINARY, TESTS_PER_SIZE
    BINARY = args.binary
    TESTS_PER_SIZE = args.count

    b = args.start
    while b <= args.end:
        for _ in range(TESTS_PER_SIZE):
            one_test(b)
        b *= 2

if __name__ == "__main__":
    main()
