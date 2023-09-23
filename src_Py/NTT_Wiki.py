from bit_reverse import *
from brute_force_root_of_unity import *
from tables import Q, N, G


def NTT_ct(a: list, w: int, n: int, q: int):
    a_ = bit_reverse_shuffle(a)
    for s in range(8):
        m = 2 << s
        w_m = pow(w, n//m, q)
        for k in range(0, n, m):
            w_k = 1
            for j in range(0, m//2, 1):
                t = (w_k * a_[k + j + (m//2)]) % q
                u = a_[k+j]
                a_[k + j] = (u + t) % q
                a_[k + j + (m // 2)] = (u - t) % q
                w_k = (w_k * w_m) % q

    return a_

def INTT_ct(a: list, w: int, n: int, q: int):
    a_ = bit_reverse_shuffle(a)
    for s in range(8):
        m = 2 << s
        w_m = pow(w, -n//m, q)
        for k in range(0, n, m):
            w_k = 1
            for j in range(0, m//2, 1):
                t = (w_k * a_[k + j + (m//2)]) % q
                u = a_[k+j]
                a_[k + j] = (u + t) % q
                a_[k + j + (m // 2)] = (u - t) % q
                w_k = (w_k * w_m) % q

    n_1 = pow(n, -1, q)
    for i in range(n):
        a_[i] = (a_[i] * n_1) % q

    return a_


if __name__ == "__main__":

    # a = x^1 + 2x^2 + 3x^3 + 4x^4 ... (n-1)x^(n-1)
    a = [x for x in range(N)]

    ntt_a1 = NTT_slow(a, Q, N, G)
    ntt_a2 = NTT_ct(a, G, N, Q)

    print(ntt_a1)
    print(ntt_a2)

    a_1 = INTT_ct(ntt_a2, G, N, Q)
    print(a_1)