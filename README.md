# Wiener Attack (C++)

Small project implementing **Wiener's attack** on RSA (C++ with NTL/GMP)

## What this does

- `wiener_attack` (C++): attempts to recover a small RSA private exponent `d` (or a factor `q`) from a public key `(e, N)` using continued fractions (Wiener, short `d` vulnerability).  
  - If successful the program prints recovered `p`, `q`, `d` and a `Time = <n>[µs]` measurement (microseconds).
- `test.py` (Python + `gmpy2`): test that:
  - generates RSA keypairs intentionally vulnerable to Wiener’s attack,
  - calls the `wiener_attack` binary for each generated key,
  - prints a single tab-separated line per test in the format:
    ```
    <bin_size>    OK    <time_us>
    ```
    or
    ```
    <bin_size>    MISMATCH
    ```

---

## Prerequisites

Install system packages and build tools:

```bash
# update & base build tools
sudo pacman -Syu
sudo pacman -S --needed base-devel gcc cmake make pkgconf

# libraries for NTL/GMP
sudo pacman -S --needed gmp mpfr ntl

```
---
## Build the C++ program
From the project directory (where wiener_attack.cpp lives):
```bash
g++ -std=c++17 -O2 wiener_attack.cpp -o wiener_attack -lntl -lgmp
```

Run the compiled program to see usage:

`./wiener_attack`
---
## Create Python virtual environment and install gmpy2
The tests uses gmpy2 for big-integer math. We recommend creating a Python virtual environment:
```bash
# create venv in project dir
python -m venv .venv

# activate (bash / zsh)
source .venv/bin/activate

# upgrade pip, then install gmpy2
python -m pip install --upgrade pip
python -m pip install gmpy2
```
