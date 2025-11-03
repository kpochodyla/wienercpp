#!/usr/bin/env python3
"""
mt_test.py — multiprocess stress tester for wiener_attack.

Each worker generates a Wiener-vulnerable RSA key (gmpy2), invokes the C++ binary,
and reports one-line results: "bin_size<TAB>OK<TAB>time_us" or "bin_size<TAB>MISMATCH".

Usage example:
    python3 mt_test.py -b ./wiener_attack -s 64 128 -n 10 -w 8 --timeout 20
"""

import argparse
import secrets
import subprocess
import re
import sys
import os
from multiprocessing import Pool, cpu_count
from functools import partial

import gmpy2
from gmpy2 import mpz

# Regexes to parse C++ output
D_RE = re.compile(r'\bd\s*=\s*(\d+)\b')
TIME_RE = re.compile(r'Time.*?(\d+)\s*(?:\[?µs\]?|us|ms)?', re.IGNORECASE)

def rand_odd(bits: int) -> mpz:
    return mpz(secrets.randbits(bits - 1) | (1 << (bits - 1)) | 1)

def gen_prime(bits: int) -> mpz:
    return gmpy2.next_prime(rand_odd(bits))

def fourth_root(n: mpz) -> mpz:
    r, _ = gmpy2.iroot(n, 4)
    return r

def generate_wiener_keypair(mod_bits: int):
    """Return tuple (p,q,N,phi,e,d) as mpz"""
    hb = mod_bits // 2
    p = gen_prime(hb)
    q = gen_prime(hb)
    while q == p:
        q = gen_prime(hb)
    N = p * q
    phi = (p - 1) * (q - 1)
    fourth = fourth_root(N)
    wiener_limit = max(mpz(2), fourth // 3)
    if wiener_limit <= 2:
        d = mpz(2)
    else:
        d = mpz(secrets.randbelow(int(wiener_limit - 1)) + 2)
    while gmpy2.gcd(d, phi) != 1:
        d += 1
        if d > wiener_limit:
            d = mpz(2)
    e = gmpy2.invert(d, phi)
    return p, q, N, phi, e, d

def run_one_test(binary_path: str, bits: int, timeout: int):
    """
    Worker job: generate one key and run the binary.
    Returns a tuple: (bits, status_str, time_us_or_none)
    """
    try:
        p, q, N, phi, e, d_expected = generate_wiener_keypair(bits)
    except Exception as ex:
        return (bits, "MISMATCH", None)

    try:
        proc = subprocess.run([binary_path, str(e), str(N)],
                              capture_output=True, text=True, timeout=timeout)
        out = proc.stdout + "\n" + proc.stderr
    except subprocess.TimeoutExpired:
        return (bits, "MISMATCH", None)
    except Exception:
        return (bits, "MISMATCH", None)

    m = D_RE.search(out)
    mtime = TIME_RE.search(out)
    rec = mpz(m.group(1)) if m else None
    time_us = mtime.group(1) if mtime else None

    if rec is not None and rec == d_expected:
        # OK (if time is present include it, otherwise '-')
        return (bits, "OK", time_us if time_us is not None else "-")
    else:
        return (bits, "MISMATCH", None)

def parse_args():
    p = argparse.ArgumentParser(description="Parallel tester for wiener_attack")
    p.add_argument("-b", "--binary", default="./wiener_attack", help="path to wiener_attack binary")
    p.add_argument("-s", "--sizes", nargs="+", type=int, default=[64],
                   help="modulus sizes in bits (space separated), e.g. 64 128")
    p.add_argument("-n", "--count", type=int, default=1, help="tests per size")
    p.add_argument("-w", "--workers", type=int, default=cpu_count(), help="number of parallel workers (default: CPU count)")
    p.add_argument("--timeout", type=int, default=15, help="timeout seconds per binary call")
    return p.parse_args()

def main():
    args = parse_args()

    if not os.path.isfile(args.binary):
        print("Error: binary not found:", args.binary, file=sys.stderr)
        sys.exit(2)

    # prepare job list: list of (bits)
    jobs = []
    for bits in args.sizes:
        jobs.extend([bits] * args.count)

    # Partial func for pool
    worker = partial(run_one_test, args.binary, timeout=args.timeout)

    # Use multiprocessing Pool
    workers = max(1, min(args.workers, len(jobs)))
    with Pool(processes=workers) as pool:
        # imap_unordered yields results as they complete
        for result in pool.imap_unordered(worker, jobs):
            bits, status, t = result
            if status == "OK":
                print(f"{bits}\tOK\t{t}")
            else:
                print(f"{bits}\tMISMATCH")
            # flush so outputs appear in realtime if piped
            sys.stdout.flush()

if __name__ == "__main__":
    main()
