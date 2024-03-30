import sys
sys.path.insert(0, '.\\lib')

import ds2


signer  = ds2.ds2_host()

print("ds2_host party number: ", signer.get_parties_num())

#pi_val = b'\x01'*64

#signer.set_pi_val(0, pi_val)


rho, CPU_cycles = signer.get_rho()

print(CPU_cycles, len(rho), rho)