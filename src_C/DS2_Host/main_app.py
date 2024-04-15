import threading
import tkinter as tk
from tkinter import messagebox
from tkinter import filedialog as fd
from tkinter import ttk
from threading import Thread
import serial
import time
import sys
import base64
sys.path.insert(0, '.\\lib')

import ds2

def xorSign(msg):
    seed = 0x00
    for byte in msg:
        seed = byte^seed
    return seed


class BufferToBig(Exception):
    def __init__(self, err_msg, err_code):
        self.err_msg = err_msg
        self.err_code = err_code


class party:
    DS2_Pi_COMMIT_SIZE = 64
    DS2_Pi_VALUE_SIZE = 16
    DS2_Ti_COMMIT_SIZE = 64
    DS2_Ti_VALUE_SIZE = 2 * (11 * (256 >> 3))
    DS2_Fi_COMMIT_SIZE = 2*2*3*256
    DS2_Zi_1_VALUE_SIZE = 3*2*256
    DS2_Zi_2_VALUE_SIZE = 3*2*256
    DS2_Ri_VALUE_SIZE = 16
    
    DS2_ABORT = 0xC0
    DS2_ERROR_Pi_COMMIT                 = 0x03 | DS2_ABORT
    DS2_ERROR_Ti_COMMIT                 = 0x04 | DS2_ABORT
    DS2_ERROR_Fi_COMMIT                 = 0x05 | DS2_ABORT
    DS2_ERROR_Zi_REJECT                 = 0X06 | DS2_ABORT

    def __init__(self):
        self.reset()
        
    def reset(self):
        self.pi_commit = bytearray()
        self.pi_val = bytearray()
        self.ti_commit = bytearray()
        self.ti_val = bytearray()
        self.zi_1_val = bytearray()
        self.zi_2_val = bytearray()
        self.ri_val = bytearray()
        self.fi_commit = bytearray()
        
    def add_pi_c(self, chunk:bytearray):
        #TODO - Add check if packets are in correct order
        self.pi_commit += chunk
        
        if len(self.pi_commit) == self.DS2_Pi_COMMIT_SIZE:
            return True
        elif len(self.pi_commit) < self.DS2_Pi_COMMIT_SIZE:
            return False
        else:
            raise BufferToBig("pi_commit size exceeded max buffer size", self.DS2_ERROR_Pi_COMMIT)
        
    def add_pi_v(self, chunk:bytearray):
        #TODO - Add check if packets are in correct order
        self.pi_val += chunk
        
        if len(self.pi_val) == self.DS2_Pi_VALUE_SIZE:
            return True
        elif len(self.pi_val) < self.DS2_Pi_VALUE_SIZE:
            return False
        else:
            raise BufferToBig("pi_val size exceeded max buffer size", self.DS2_ERROR_Pi_COMMIT)
        
    def add_ti_c(self, chunk:bytearray):
        #TODO - Add check if packets are in correct order
        self.ti_commit += chunk
        
        if len(self.ti_commit) == self.DS2_Ti_COMMIT_SIZE:
            return True
        elif len(self.ti_commit) < self.DS2_Ti_COMMIT_SIZE:
            return False
        else:
            raise BufferToBig("ti_commit size exceeded max buffer size", self.DS2_ERROR_Ti_COMMIT)
        
    def add_ti_v(self, chunk:bytearray):
        #TODO - Add check if packets are in correct order
        self.ti_val += chunk
        
        if len(self.ti_val) == self.DS2_Ti_VALUE_SIZE:
            return True
        elif len(self.ti_val) < self.DS2_Ti_VALUE_SIZE:
            return False
        else:
            raise BufferToBig("ti_val size exceeded max buffer size", self.DS2_ERROR_Ti_COMMIT)
    
    def add_fi_c(self, chunk:bytearray):
        #TODO - Add check if packets are in correct order
        self.fi_commit += chunk
        
        if len(self.fi_commit) == self.DS2_Fi_COMMIT_SIZE:
            return True
        elif len(self.fi_commit) < self.DS2_Fi_COMMIT_SIZE:
            return False
        else:
            raise BufferToBig("fi_commit size exceeded max buffer size", self.DS2_ERROR_Fi_COMMIT)
        
    def add_zi_1_v(self, chunk:bytearray):
        #TODO - Add check if packets are in correct order
        self.zi_1_val += chunk
        
        if len(self.zi_1_val) == self.DS2_Zi_1_VALUE_SIZE:
            return True
        elif len(self.zi_1_val) < self.DS2_Zi_1_VALUE_SIZE:
            return False
        else:
            raise BufferToBig("zi_1_val size exceeded max buffer size", self.DS2_ERROR_Fi_COMMIT)
        
    def add_zi_2_v(self, chunk:bytearray):
        #TODO - Add check if packets are in correct order
        self.zi_2_val += chunk
        
        if len(self.zi_2_val) == self.DS2_Zi_2_VALUE_SIZE:
            return True
        elif len(self.zi_2_val) < self.DS2_Zi_2_VALUE_SIZE:
            return False
        else:
            raise BufferToBig("zi_2_val size exceeded max buffer size", self.DS2_ERROR_Fi_COMMIT)
        
    def add_ri_v(self, chunk:bytearray):
        #TODO - Add check if packets are in correct order
        self.ri_val += chunk
        
        if len(self.ri_val) == self.DS2_Ri_VALUE_SIZE:
            return True
        elif len(self.ri_val) < self.DS2_Ri_VALUE_SIZE:
            return False
        else:
            raise BufferToBig("ri_val size exceeded max buffer size", self.DS2_ERROR_Fi_COMMIT)



class Sniffer:
    
    NUMBER_OF_PARTIES = 1
    
    nodes = [party()] * NUMBER_OF_PARTIES
    
    DS2_Pi_COMMIT   = 0x00
    DS2_Pi_VALUE    = 0x01
    DS2_Ti_COMMIT   = 0x02
    DS2_Ti_VALUE    = 0x03
    DS2_Fi_COMMIT   = 0x04
    DS2_Ri_VALUE    = 0x05
    DS2_Zi_1_VALUE  = 0x06
    DS2_Zi_2_VALUE  = 0x07

    DS2_COORDINATOR_HELLO       = 0x08
    DS2_COORDINATOR_READY_RESET = 0x09

    DS2_Pi_COMMIT_ACK   = DS2_Pi_COMMIT | 0x40
    DS2_Pi_VALUE_ACK    = DS2_Pi_VALUE | 0x40
    DS2_Ti_COMMIT_ACK   = DS2_Ti_COMMIT | 0x40
    DS2_Ti_VALUE_ACK    = DS2_Ti_VALUE | 0x40
    DS2_Fi_COMMIT_ACK   = DS2_Fi_COMMIT | 0x40
    DS2_Ri_VALUE_ACK    = DS2_Ri_VALUE |0x40
    DS2_Zi_1_VALUE_ACK  = DS2_Zi_1_VALUE | 0x40
    DS2_Zi_2_VALUE_ACK  = DS2_Zi_2_VALUE | 0x40

    DS2_COORDINATOR_HELLO_ACK = DS2_COORDINATOR_HELLO | 0x40

    DS2_KEYGEN_START_TASK   = 0x80
    DS2_SIGN_START_TASK     = 0x81
    DS2_VERIFY_START_TASK   = 0x82

    DS2_ABORT = 0xC0
    DS2_ERROR_INVALID_NODE_ID           = 0x01 | DS2_ABORT
    DS2_ERROR_NODE_ID_ALREADY_IN_USE    = 0x02 | DS2_ABORT
    DS2_ERROR_Pi_COMMIT                 = 0x03 | DS2_ABORT
    DS2_ERROR_Ti_COMMIT                 = 0x04 | DS2_ABORT
    DS2_ERROR_Fi_COMMIT                 = 0x05 | DS2_ABORT
    DS2_ERROR_Zi_REJECT                 = 0X06 | DS2_ABORT

    DS2_UNKNOWN_ERROR = 0xfd
    DS2_CHECK_COMMIT = 0xfe
    DS2_DBG = 0xff

    DS2_Pi_COMMIT_FLAG      = (1 << DS2_Pi_COMMIT)
    DS2_Pi_VALUE_FLAG 		= (1 << DS2_Pi_VALUE)
    DS2_Ti_COMMIT_FLAG 		= (1 << DS2_Ti_COMMIT)
    DS2_Ti_VALUE_FLAG 		= (1 << DS2_Ti_VALUE)
    DS2_Fi_COMMIT_FLAG 		= (1 << DS2_Fi_COMMIT)
    DS2_Ri_VALUE_FLAG 		= (1 << DS2_Ri_VALUE)
    DS2_Zi_1_VALUE_FLAG 	= (1 << DS2_Zi_1_VALUE)
    DS2_Zi_2_VALUE_FLAG 	= (1 << DS2_Zi_2_VALUE)
    DS2_PARTY_ACTIVE   		= 0x80000000

    DS2_BROADCAST_ID        = 255
    DS2_COORDINATOR_ID      = 254
    
    DS2_MAX_PACKET_LEN      = 100
    SEED_BYTES              = 16

    def __init__(self):
        self.root = tk.Tk()
        self.main_frame = tk.Frame(self.root)
        self.ser = None
        self.msg_string = ""
        self.thread_run = False
        self.read_thread = None
        self.index = 0
        self._lock = threading.Lock()
        self.signer = ds2.ds2_host()
        
        self.keygen_time = 0
        self.sign_time = 0
        self.verify_time = 0
        
        self.signature = None
        self.hmsg = None
        
        self.fdc = self.signer.get_freq_coef()
        
        col = 0

        self.listbox_main = tk.Listbox(self.main_frame, width=100, height=30)
        self.listbox_main.grid(row=0, column=col, rowspan=10, padx=2, pady=2, sticky="nswe")
        col += 1
        scrollbar = tk.Scrollbar(self.main_frame)
        scrollbar.grid(row=0, column=col, rowspan=10, sticky="nswe")
        self.listbox_main.config(yscrollcommand=scrollbar.set)
        scrollbar.config(command=self.listbox_main.yview)
        col += 1
        
        self.listbox_dbg = tk.Listbox(self.main_frame, width=100, height=30)
        self.listbox_dbg.grid(row=0, column=col, rowspan=10, padx=2, pady=2, sticky="nswe")
        col += 1
        scrollbar = tk.Scrollbar(self.main_frame)
        scrollbar.grid(row=0, column=col, rowspan=10, sticky="nswe")
        self.listbox_dbg.config(yscrollcommand=scrollbar.set)
        scrollbar.config(command=self.listbox_dbg.yview)
        self.dbg_index = 0
        
        col += 1

        control_frame = tk.Frame(self.main_frame)

        port_frame = tk.LabelFrame(control_frame, text='Port')
        self.port_var = tk.StringVar()
        port_entry = tk.Entry(port_frame, textvariable=self.port_var)
        port_entry.grid(sticky="nswe")
        port_frame.grid(row=0, column=0, sticky="nswe", padx=2, pady=2)

        baudrate_frame = tk.LabelFrame(control_frame, text='Baudrate')
        self.baudrate_var = tk.StringVar()
        baudrate_entry = tk.Entry(baudrate_frame, textvariable=self.baudrate_var)
        baudrate_entry.grid(sticky="nswe")
        baudrate_frame.grid(row=1, column=0, sticky="nswe", padx=2, pady=2)

        btn_port_open = tk.Button(control_frame, text="Open port", command=self.port_open)
        btn_port_open.grid(row=2, column=0, sticky="nswe", padx=2, pady=2)

        btn_port_close = tk.Button(control_frame, text="Close port", command=self.port_close)
        btn_port_close.grid(row=3, column=0, sticky="nswe", padx=2, pady=2)

        btn_save_log = tk.Button(control_frame, text="Save log", command=self.save_log_file)
        btn_save_log .grid(row=4, column=0, sticky="nswe", padx=2, pady=2)
        
        btn_clear = tk.Button(control_frame, text="Clear screen", command=self.clear)
        btn_clear .grid(row=5, column=0, sticky="nswe", padx=2, pady=2)
        
        control_frame.grid(row=0, column=col, rowspan=10, sticky="nswe")

        ###############################################
        col += 1
        sign_control_frame = tk.Frame(self.main_frame)
        
        msg_frame = tk.LabelFrame(sign_control_frame, text='Message')
        self.msg_var = tk.StringVar()
        msg_entry = tk.Entry(msg_frame, textvariable=self.msg_var)
        msg_entry.grid(sticky="nswe")
        msg_frame.grid(row=0, column=0, sticky="nswe", padx=2, pady=2)
        
        btn_key_gen = tk.Button(sign_control_frame, text="New Keys", command=self.keygen)
        btn_key_gen.grid(row=1, column=0, sticky="nswe", padx=2, pady=2)
        
        btn_sign = tk.Button(sign_control_frame, text="Sign", command=self.sign)
        btn_sign.grid(row=2, column=0, sticky="nswe", padx=2, pady=2)
        
        btn_verify = tk.Button(sign_control_frame, text="Verify", command=self.verify)
        btn_verify.grid(row=3, column=0, sticky="nswe", padx=2, pady=2)
        
        btn_reset = tk.Button(sign_control_frame, text="Reset", command=self.reset)
        btn_reset.grid(row=4, column=0, sticky="nswe", padx=2, pady=2)
        
        btn_commit_test = tk.Button(sign_control_frame, text="Commit Test", command=self.commit_test)
        btn_commit_test.grid(row=5, column=0, sticky="nswe", padx=2, pady=2)

        sign_control_frame.grid(row=0, column=col, rowspan=10, sticky="nswe")
        self.main_frame.grid(row=0, column=0, sticky="nswe")
        
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        
    def on_closing(self):
        self.port_close()
        self.root.destroy()


    def tk_mainloop(self):
        self.root.mainloop()
        
    def clear(self):
        self.listbox_main.delete(0, tk.END)
        self.index = 0
        self.listbox_dbg.delete(0, tk.END)
        self.dbg_index = 0
        
    def keygen(self):
        self.response(self.DS2_KEYGEN_START_TASK, dst_node=0xff)
        self.listbox_main.insert(self.index, "START KEYGEN")
        self.index += 1

    
    def sign(self):
        msg_text = self.msg_var.get()
        self.hmsg = self.signer.hash_msg(msg_text)
        self.signer.set_msg(self.hmsg)
        self.listbox_main.insert(self.index, "START SIGN")
        self.index += 1
        self.response(self.DS2_SIGN_START_TASK, dst_node=0xff, data=self.hmsg)
        
    
    def verify(self):
        file = fd.askopenfile(initialfile='Signature.txt',defaultextension=".txt",filetypes=[("All Files","*.*"),("Text","*.txt")])
        str_data = file.read()
        byte_data = str_data.encode("utf-8")
        signature = base64.b64decode(byte_data)
        file.close()
        
        file = fd.askopenfile(initialfile='Public_key.txt',defaultextension=".txt",filetypes=[("All Files","*.*"),("Text","*.txt")])
        str_data = file.read()
        byte_data = str_data.encode("utf-8")
        pk = base64.b64decode(byte_data)
        file.close()

        state = self.signer.get_internal_state()
        
        before = {
            "c": xorSign(state["c"]),
            "poly_c": xorSign(state["poly_c"]),
            "ck_seed": xorSign(state["ck_seed"]),
            "rho": xorSign(state["rho"]),
            "tr": xorSign(state["tr"]),
            "z1": xorSign(state["z1"]),
            "z2": xorSign(state["z2"]),
            "t1": xorSign(state["t1"]),
            "A": xorSign(state["A"]),
            "r": xorSign(state["r"]),
            "msg": xorSign(state["msg"]),
            }
        
        K = 2 #z2
        L = 2 #z1
        N = 256
        
        offset = 0
        t1 = pk[offset: offset+4*N*K]
        offset += 4*N*K
        rho = pk[offset:offset+self.SEED_BYTES]
        offset += self.SEED_BYTES
        tr = pk[offset:offset+self.SEED_BYTES]
        
        self.signer.set_public_key(t1, rho, tr)

        offset = 0
        self.hmsg = signature[offset:256]
        offset += 256
        c = signature[offset:offset+self.SEED_BYTES]
        offset += self.SEED_BYTES
        z1 = signature[offset:offset+4*N*L]
        offset += 4*N*L
        z2 = signature[offset:offset+4*N*K]
        offset += 4*N*K
        ri = list()
        while offset < len(signature):
            ri.append(signature[offset:offset+self.SEED_BYTES])
            offset += self.SEED_BYTES
            
        self.signer.set_msg(self.hmsg)
        flag, self.verify_time = self.signer.verify(c, z1, z2, ri)
        if flag:
            self.listbox_main.insert(self.index, "VERIFICATION - OK, TIME SPENT: {}".format(self.verify_time))
            self.index += 1
        else:
            self.listbox_main.insert(self.index, "VERIFICATION - NOK, TIME SPENT: {}".format(self.verify_time))
            self.index += 1
            
        
        state = self.signer.get_internal_state()
        after = {
            "c": xorSign(state["c"]),
            "poly_c": xorSign(state["poly_c"]),
            "ck_seed": xorSign(state["ck_seed"]),
            "rho": xorSign(state["rho"]),
            "tr": xorSign(state["tr"]),
            "z1": xorSign(state["z1"]),
            "z2": xorSign(state["z2"]),
            "t1": xorSign(state["t1"]),
            "A": xorSign(state["A"]),
            "r": xorSign(state["r"]),
            "msg": xorSign(state["msg"]),
            }

        print(before)
        print(after)
        
    def commit_test(self):
        data = bytes([0xAA]*6176)
        ri = data[0:16]
        ck = data[16:32]
        fi = data[32: 4128]
        wi = data[4128:]

        flag, cpu_cycles = self.signer.check_commit(ri, ck, fi, wi)
        self.listbox_main.insert(self.index, "Commit (slow) time - {}".format( cpu_cycles))
        self.index += 1
                    
        flag, cpu_cycles = self.signer.check_commit2(ri, ck, fi, wi)
        self.listbox_main.insert(self.index, "Commit2 time - {}".format(cpu_cycles))
        self.index += 1

    
    def reset(self):
        for node in self.nodes:
            node.reset()
        self.signer.reset()
        self.keygen_time = 0
        self.sign_time = 0
        self.verify_time = 0
        self.abort(self.DS2_ABORT)
        self.listbox_main.insert(self.index, "KEYS ARE DELETED")
        self.index += 1

    def port_open(self):
        self.listbox_main.delete(0, tk.END)
        self.index = 0
        if self.read_thread is None:
            self.thread_run = True
            self.listbox_main.insert(self.index, "THREAD START")
            self.read_thread = Thread(target=self.read_serial_thread)
            self.read_thread.start()
            self.index += 1

    def port_close(self):
        self.thread_run = False
        if self.read_thread is not None:
            self.read_thread.join()
            self.read_thread = None
            if self.ser is not None:
                #print("port closing ...")
                self.ser.close()
                self.listbox_main.insert(self.index, "port is closed")
                self.index += 1

            self.ser = None
            self.listbox_main.insert(self.index, "THREAD STOP")
            self.index += 1

    def save_log_file(self):
        file = fd.asksaveasfile(initialfile='Untitled.log',defaultextension=".log",filetypes=[("All Files","*.*"),("Logs","*.log")])
        file_text = self.listbox_main.get(2, tk.END)
        for line in file_text:
            file.write(line)
        file.close()
        
    def abort(self, err_code:int):
        self.response(err_code)
            
    def response(self, msg_code:int, dst_node:int=0xff, data:bytes=None):
        if self.ser is not None and self.ser.is_open:
            header = msg_code.to_bytes(1, 'little') + dst_node.to_bytes(1, 'little')
            if data is None:
                msg = 0x00.to_bytes(4, 'little') +  header
                self.ser.write(msg)
                print("send ", ":".join("{:02x}".format(c) for c in msg))
            else:
                packet_len = len(data)
                msg = packet_len.to_bytes(4, 'little') +  header + data
                self.ser.write(msg)
                print("send ", ":".join("{:02x}".format(c) for c in msg))

                    
        
        
    def msg_parser(self, msg:bytes):
        dst_node_id = msg[0]
        src_node_id = msg[1]
        msg_code = msg[2]
        if len(msg) == 3:
            offset = 0
            data = None
            crc = 0
        else:
            offset = int.from_bytes(msg[3:7], byteorder='little', signed=False)
            data = msg[7:]
            crc = xorSign(data)
        try:
            match msg_code:
                case self.DS2_COORDINATOR_HELLO:
                    self.listbox_main.insert(self.index, "Party ID:{} connected".format(src_node_id))
                    self.index += 1

                case self.DS2_COORDINATOR_READY_RESET:
                    self.listbox_main.insert(self.index, "Coordinator ready !!!")
                    self.index += 1
            
                case self.DS2_Pi_COMMIT:
                    #self.listbox_main.insert(self.index, "Party ID:{} Pi commit CRC = {}".format(src_node_id, crc))
                    #self.index += 1
                    print("Party ID:{} Pi commit CRC = {}".format(src_node_id, crc))
                    flag = self.nodes[src_node_id].add_pi_c(data)
                    if flag is True:
                        self.signer.set_pi_commit(src_node_id, self.nodes[src_node_id].pi_commit)
                        self.response(self.DS2_Pi_COMMIT_ACK, dst_node=src_node_id)
                
                case self.DS2_Pi_VALUE:
                    #self.listbox_main.insert(self.index, "Party ID:{} Pi value CRC = {}".format(src_node_id, crc))
                    #self.index += 1
                    print("Party ID:{} Pi value CRC = {}".format(src_node_id, crc))
                    flag = self.nodes[src_node_id].add_pi_v(data)
                    if flag is True:
                        self.signer.set_pi_val(src_node_id,  self.nodes[src_node_id].pi_val)
                        if self.signer.is_flag_ready(self.DS2_Pi_COMMIT_FLAG | self.DS2_Pi_VALUE_FLAG):
                            rho, time_stamp = self.signer.get_rho()
                            self.keygen_time += time_stamp
                            crc = xorSign(rho)
                            self.listbox_main.insert(self.index, "KeyGen: send rho CRC {} - time {}".format(crc, self.keygen_time))
                            self.index += 1
                            self.response(self.DS2_Pi_VALUE_ACK, dst_node=0xff, data=rho)
                
                case self.DS2_Ti_COMMIT:
                    #self.listbox_main.insert(self.index, "Party ID:{} Ti commit CRC = {}".format(src_node_id, crc))
                    #self.index += 1
                    print("Party ID:{} Ti commit CRC = {}".format(src_node_id, crc))
                    flag = self.nodes[src_node_id].add_ti_c(data)
                    if flag is True:
                        self.signer.set_ti_commit(src_node_id,  self.nodes[src_node_id].ti_commit)
                        self.response(self.DS2_Ti_COMMIT_ACK, dst_node=src_node_id)
                
                case self.DS2_Ti_VALUE:
                    #self.listbox_main.insert(self.index, "Party ID:{} Ti value CRC {}".format(src_node_id, crc))
                    #self.index += 1
                    print("Party ID:{} Ti value CRC {}".format(src_node_id, crc))
                    flag = self.nodes[src_node_id].add_ti_v(data)
                    if flag is True:
                        self.signer.set_ti_val(src_node_id, self.nodes[src_node_id].ti_val)
                        if self.signer.is_flag_ready(self.DS2_Ti_COMMIT_FLAG | self.DS2_Ti_VALUE_FLAG):
                            tr, time_stamp = self.signer.get_tr()
                            self.keygen_time += time_stamp
                            crc = xorSign(tr)
                            self.listbox_main.insert(self.index, "KeyGen: send tr CRC {} - time {}".format(crc, time_stamp))
                            self.index += 1
                            self.response(self.DS2_Ti_VALUE_ACK, dst_node=0xff, data=tr)
                            self.listbox_main.insert(self.index, "KEYS GENERATED - TIME SPENT: {} FREQUENCY COEFFICIENT: {}".format(self.keygen_time, self.fdc))
                            self.index += 1
                            public_key = self.signer.get_public_key()
                            b64_public_key = base64.b64encode(public_key)
                            f = open("Public_key.txt", "w")
                            f.write(b64_public_key.decode("utf-8"))
                            f.close()
                    
                case self.DS2_Fi_COMMIT:
                    #self.listbox_main.insert(self.index, "Party ID:{} Fi commit value CRC {}".format(src_node_id, crc))
                    #self.index += 1
                    print("Party ID:{} Fi commit value CRC {}".format(src_node_id, crc))
                    flag = self.nodes[src_node_id].add_fi_c(data)
                    if flag is True:
                        self.signer.set_fi_commit(src_node_id, self.nodes[src_node_id].fi_commit)
                        if self.signer.is_flag_ready(self.DS2_Fi_COMMIT_FLAG):
                            sc, time_stamp = self.signer.get_c()
                            crc = xorSign(sc)
                            self.sign_time += time_stamp
                            self.listbox_main.insert(self.index, "Sign: send sc value CRC {} time {}".format(crc, time_stamp))
                            self.index += 1
                            self.response(self.DS2_Fi_COMMIT_ACK, dst_node=0xff, data=sc)
                    
                case self.DS2_Ri_VALUE:
                    #self.listbox_main.insert(self.index, "Party ID:{} Ri value CRC {}".format(src_node_id, crc))
                    #self.index += 1
                    print("Party ID:{} Ri value CRC {}".format(src_node_id, crc))
                    flag = self.nodes[src_node_id].add_ri_v(data)
                    if flag is True:
                        self.signer.set_ri_val(src_node_id, self.nodes[src_node_id].ri_val)
                        self.response(self.DS2_Ri_VALUE_ACK, dst_node=src_node_id)
                    
                case self.DS2_Zi_1_VALUE:
                    #self.listbox_main.insert(self.index, "Party ID:{} Zi_1 value CRC {}".format(src_node_id, crc))
                    #self.index += 1
                    print("Party ID:{} Zi_1 value CRC {}".format(src_node_id, crc))
                    flag = self.nodes[src_node_id].add_zi_1_v(data)
                    if flag is True:
                        self.signer.set_zi_1_val(src_node_id, self.nodes[src_node_id].zi_1_val)
                        self.response(self.DS2_Zi_1_VALUE_ACK, dst_node=src_node_id)

                case self.DS2_Zi_2_VALUE:
                    #self.listbox_main.insert(self.index, "Party ID:{} Zi_2 value CRC {}".format(src_node_id, crc))
                    #self.index += 1
                    print("Party ID:{} Zi_2 value CRC {}".format(src_node_id, crc))
                    flag = self.nodes[src_node_id].add_zi_2_v(data)
                    if flag is True:
                        self.signer.set_zi_2_val(src_node_id, self.nodes[src_node_id].zi_2_val)
                        if self.signer.is_flag_ready(self.DS2_Ri_VALUE_FLAG | self.DS2_Zi_1_VALUE_FLAG |self.DS2_Zi_2_VALUE_FLAG):
                            self.signature, time_stamp = self.signer.get_signature()
                            self.sign_time += time_stamp
                            self.response(self.DS2_Zi_2_VALUE_ACK, dst_node=0xff)
                            crc = xorSign(self.signature)
                            print("Signature: ", ":".join("{:02x}".format(c) for c in self.signature))
                            self.listbox_main.insert(self.index, "SIGNATURE GENERATED - CRC {} TIME SPENT: {} FREQUENCY COEFFICIENT: {}".format(crc, self.sign_time, self.fdc))
                            self.index += 1
                            #self.listbox_main.insert(self.index, "SIGNATURE GENERATED - CRC {} TIME SPENT: {} FREQUENCY COEFFICIENT: {}".format(crc, self.keygen_time, self.fdc))
                            #self.index += 1
                            b64_sign = base64.b64encode(self.hmsg + self.signature)
                            f = open("Signature.txt", "w")
                            f.write(b64_sign.decode("utf-8"))
                            f.close()
                            
                            
                case self.DS2_ERROR_Zi_REJECT:
                    self.listbox_main.insert(self.index, "Node[{}] rejected Z: ".format(src_node_id))
                    self.index += 1
                    self.nodes[src_node_id].fi_commit = bytearray()

                case self.DS2_DBG:
                    self.listbox_dbg.insert(self.dbg_index, data.decode(encoding='latin1'))
                    self.dbg_index += 1
                case _:
                    self.listbox_main.insert(self.index, "unknown msg_code:{} data:{}".format(msg_code, data))
                    self.index += 1
                    
        except ds2.DS2Exception as err:
            self.listbox_main.insert(self.index, "Error: {} ".format(str(err))) #TODO - paint red
            self.index += 1
            err_code = self.signer.translate_exception()
            self.abort(err_code)
        except BufferToBig as err:
            self.listbox_main.insert(self.index, "Error: {} ".format(err.err_msg)) #TODO - paint red
            self.index += 1
            self.abort(err.err_code)
        

    def read_serial_thread(self):
        try:
            self.ser = serial.Serial(port=self.port_var.get(),
                                     baudrate=int(self.baudrate_var.get()),
                                     bytesize=8,
                                     parity=serial.PARITY_NONE)
            #self.ser.open()
            if self.ser.is_open:
                self.listbox_main.insert(self.index, "port is opened")
                self.index += 1


                while self.thread_run:
                    read_len = self.ser.inWaiting()
                    if read_len >= 4:
                        frame_len = self.ser.read(1)
                        if frame_len[0] > 108:
                            continue
                        #data_len = int.from_bytes(frame_len, byteorder='little', signed=False)
                        frame = self.ser.read(frame_len[0]-1)
                        if frame[2] < 255:
                            print("msg len: {} msg code: {} node id: {}".format(frame_len[0], frame[2], frame[1]))
                            print(":".join("{:02x}".format(c) for c in (frame_len+frame)))
                        self.msg_parser(frame)
                        
                    #time.sleep(0.1)
        finally:
            pass
            #print("THREAD FINAL")
            #if self.ser is not None:
            #    print("port closing ...")
            #    self.ser.close()
            #    self.listbox.insert(self.index, "port is closed")
            #    self.index += 1

            #self.ser = None


if __name__ == "__main__":
    sniffer = Sniffer()
    try:
        sniffer.tk_mainloop()
    finally:
        sniffer.port_close()