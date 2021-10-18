# run this program in sage 
end_size = 8192
bin_size = 64

print("bin_size\tp\tq\tN\tphiN\te\td")
while (bin_size <= end_size):
    for i in range(0, 100):
        p = random_prime(pow(2, bin_size // 2), False, pow(2, (bin_size // 2) - 1))
        q = random_prime(pow(2, bin_size // 2), False, pow(2, (bin_size // 2) - 1))
        while(p == q):
            q = random_prime(pow(2, bin_size // 2), False, pow(2, (bin_size // 2) - 1))
        N = p*q
        phiN = (p - 1) * (q - 1)
        wiener = floor((N ** (1 / 4)) / 3)
        d = randint(2, wiener)
        while (gcd(d, phiN) != 1):
            d = randint(2, wiener)
        e = inverse_mod(d, phiN)
        print("{}\t{}\t{}\t{}\t{}\t{}\t{}".format(bin_size, p, q, N, phiN, e, d))
        bin_size *= 2
