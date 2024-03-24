import sys
sys.path.insert(0, '.\\lib')

import ds2


signer  = ds2.ds2_host()

print("ds2_host party number: ", signer.get_parties_num())

msg_str = "0"*64

msg = bytes(msg_str.encode("ascii"))

signer.set_pi_commit(0,msg)