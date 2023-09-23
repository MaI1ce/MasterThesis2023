from brute_force_root_of_unity import *
from bit_reverse import *
from tables import *
from NTT_Wiki import *
import numpy as np

#  bitrev_shuffle256(a);
#  ntt_ct_rev2std_v1(a, 256, psi_powers_ntt256_12289);
def NTT_gs(a: list, p: list) -> list:
    a_ = bit_reverse_shuffle(a)
    # a_ = a
    t = N >> 1
    while t > 0:
        for j in range(t):
            w = p[t + j]  # w_t ^ j
            s = j
            while s < N:
                x = a_[s + t]
                a_[s + t] = ((a_[s] - x) * w) % Q
                a_[s] = (a_[s] + x) % Q
                s += 2*t

        t >>= 1

    return a_



def NTT_mult(a: list, b: list) -> list:
    a_ = [0]*N
    b_ = [0]*N

    for i in range(N):
        a_[i] = (a[i] * pow(phi, i, Q)) % Q

    for i in range(N):
        b_[i] = (b[i] * pow(phi, i, Q)) % Q

    a_ntt = NTT_ct(a_, G, N, Q)
    b_ntt = NTT_ct(b_, G, N, Q)

    c_ntt = [0] * N  # ???
    for i in range(N):
        c_ntt[i] = (a_ntt[i]*b_ntt[i]) % Q

    c_ = INTT_ct(c_ntt, G, N, Q)
    c = [0] * N
    for i in range(N):
        c[i] = (c_[i] * pow(phi, -i, Q)) % Q

    return c


if __name__ == "__main__":

    print('G = {} is a primitive {}-th root of unity for Q = {} - {}'.format(G, N, Q, is_primitive_root(G, N, Q)))
    print('phi = {} is a primitive {}-th root of unity for Q = {} - {}'.format(phi, 2*N, Q, is_primitive_root(phi, 2*N, Q)))

    # a = x^1 + 2x^2 + 3x^3 + 4x^4 ... (n-1)x^(n-1)
    a1 = [x for x in range(N)]
    a2 = [N-1-x for x in range(N)]
    b1 = [x for x in range(N)]
    b2 = [N-1-x for x in range(N)]

    x = [0]*(N+1)
    x[N] = 1
    x[0] = 1

    # print(a1)
    # a_ntt1 = NTT_slow(a1, Q, N, G)
    # print(a_ntt1)
    # a_1 = INTT_slow(a_ntt1, Q, N, G)
    # # print(a_)

    c1 = NTT_mult(a1, b1)
    print(c1)
    print('-------------')



