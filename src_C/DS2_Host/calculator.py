def memory_calculator_orig(K, L, N, T, n):
    print("\n\n --- K = {}, L = {}, N = {}, T = {}, n = {} ---".format(K, L, N, T, n))

    M_static = 4*N*(K*(L+(n+2)) + L + + (n+1)*K*K)
    M_stack = 4*N*((K+1)*T + 5*L + 7*K + 1) + 3*N*K*(2+K)
    M_heap = (K+1)*3*N*T
    M_sign = 4*N*(K+L+K*T)

    print("\n-- old implementation --")
    print("M_static = {} KB".format(M_static /1024))
    print("M_stack = {} KB".format(M_stack / 1024))
    print("M_heap = {} KB".format(M_heap / 1024))
    print("M_sign = {} KB".format(M_sign / 1024))
    print("Total = {} KB".format((M_static + M_stack + M_heap + M_sign) / 1024))

    M_static1 = 4*N*(K*L+K*(n+2) + L + (n+2)*K*K) #*(n+1)
    M_sign1 = 4 * N * (K + L)
    M_stack1 = 4*N*(5*L + 7*K  + 3) + 3*N*K*(2+K)
    M_heap1 = 0

    print("\n-- Modification 1 --")
    print("M_static = {} KB".format(M_static1 /1024))
    print("M_stack = {} KB".format(M_stack1 / 1024))
    print("M_heap = {} KB".format(M_heap1 / 1024))
    print("M_sign = {} KB".format(M_sign1 / 1024))
    print("Total = {} KB".format((M_static1 + M_stack1 + M_heap1 + M_sign1) / 1024))

    M_static2 = 4*N*(K*L + 6*L + 8*K + 3)
    M_sign2 = 4 * N * (K + L)
    M_stack2 = 4*N*(2*K + L + K*K) + 3*N*(2*K + L + K*K)
    M_heap2 = 0

    print("\n-- Modification 2 --")
    print("M_static = {} KB".format(M_static2 / 1024))
    print("M_stack = {} KB".format(M_stack2 / 1024))
    print("M_heap = {} KB".format(M_heap2 / 1024))
    print("M_sign = {} KB".format(M_sign2 / 1024))
    print("Total = {} KB".format((M_static2 + M_stack2 + M_heap2 + M_sign2) / 1024))


    M_static3 = 4*N*(5*L + 7*K + 3)
    M_sign3 = 4 * N * (K + L)
    M_stack3 = 4*N*(2*K + L + K*K + 3) + 3*N*(2*K + L + K*K)
    M_heap3 = 0

    print("\n-- Modification 3 --")
    print("M_static = {} KB".format(M_static3 / 1024))
    print("M_stack = {} KB".format(M_stack3 / 1024))
    print("M_heap = {} KB".format(M_heap3 / 1024))
    print("M_sign = {} KB".format(M_sign3 / 1024))
    print("Total = {} KB".format((M_static3 + M_stack3 + M_heap3 + M_sign3) / 1024))


for i in range(2, 6):
    memory_calculator_orig(K=i, L=i, N=256, T=69, n=10)