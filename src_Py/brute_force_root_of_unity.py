import copy


def factorize(num: int) -> list:
    prime_factors = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89]
    n = copy.copy(num) # just to be sure that we do not change num
    prime_f = list()
    for prime in prime_factors:
        if n == 0:
            break
        if n % prime == 0:
            prime_f.append(prime)
            while n % prime == 0 and n != 0:
                n = n // prime

    return prime_f


def find_generators(p: int) -> list:
    # random g in [2, p-2] is a generator if g^((p-1)/q) != 1 mod p
    # for each prime factor q in the factorization of p−1
    q = factorize(p-1)
    g_list = list()

    for g in range(2, p-1, 1):
        is_generator = True  # assumption
        for pf in q:
            if pow(g, (p-1)//pf, p) == 1:
                is_generator = False
                break

        if is_generator:
            g_list.append(g)

    return g_list


def is_primitive_root(x: int, n: int, q: int) -> bool:
    for i in range(1, n):
        if pow(x, i, q) == 1:
            print(i)
            return False

    return pow(x, n, q) == 1


def find_phi(w: int, q: int) -> int:
    for i in range(2, q, 1):
        if pow(i, 2, q) == w:
            return i

    return 0


def slow_poly_mul(a: list, b: list, q: int, Q: int) -> list:
    n = len(a)
    tab = list()
    for i in range(n):
        tab.append([0]*n)
        for j in range(n):
            tab[i][j] = a[i] * b[j]

    res_poly = list()

    for j in range(n):
        i = 0
        a_k = 0
        while j > -1:
            a_k += tab[i][j]
            j -= 1
            i += 1
        res_poly.append(a_k % q)

    for i in range(1, n):
        j = n-1
        a_k = 0
        while i < n:
            a_k += tab[i][j]
            j -= 1
            i += 1
        res_poly.append(a_k % q)

    # get remainder of division on polynomial x^Q + 1

    n2 = len(res_poly) - 1
    if Q > n2:
        return res_poly + [0]*(Q-n2-1)

    d = res_poly[n2]
    while d == 0 and n2 > 0:
        res_poly.pop(n2)
        n2 -= 1
        d = res_poly[n2]

    while n2 >= Q:
        c = res_poly.pop(n2)
        k = n2 - Q
        res_poly[k] -= c
        n2 -= 1

    return res_poly


def NTT_slow(a: list, q: int, n: int, g: int)->list:
    a_ntt = [0]*n

    for i in range(n):
        for j in range(n):
            a_ntt[i] += a[j] * pow(g, i*j, q)

        a_ntt[i] = a_ntt[i] % q

    return a_ntt


def INTT_slow(a_ntt: list, q: int, n: int, g: int) -> list:
    a = [0] * n

    for i in range(n):
        for j in range(n):
            a[i] =  (a[i] + a_ntt[j] * pow(g, -1 * i * j, q)) % q

        a[i] = (a[i] * pow(n, -1, q)) % q

    return a



if __name__ == "__main__":


    max_degree = 5 # x^5 + 1
    a = [1, 0, 1, 0, 0] # x^2 + 1
    b = [1, 0, 0, 0, 1] # x^4 + 1

    c = slow_poly_mul(a,b,200,max_degree)

    print(c)