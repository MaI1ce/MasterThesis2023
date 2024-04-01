import threading
import tkinter as tk
from tkinter import messagebox
from tkinter import filedialog as fd
from tkinter import ttk
from threading import Thread
import serial
import time
import sys
sys.path.insert(0, '.\\lib')

import ds2


class Sniffer:
    
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

    DS2_UNKNOWN_ERROR = 0xff


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
        self.sing_time = 0
        self.verify_time = 0;

        self.listbox = tk.Listbox(self.main_frame, width=80, height=30)
        self.listbox.grid(row=0, column=0, rowspan=10, padx=2, pady=2, sticky="nswe")
        scrollbar = tk.Scrollbar(self.main_frame)
        scrollbar.grid(row=0, column=1, rowspan=10, sticky="nswe")
        self.listbox.config(yscrollcommand=scrollbar.set)
        scrollbar.config(command=self.listbox.yview)

        port_frame = tk.LabelFrame(self.main_frame, text='Port')
        self.port_var = tk.StringVar()
        port_entry = tk.Entry(port_frame, textvariable=self.port_var)
        port_entry.grid(sticky="nswe")
        port_frame.grid(row=0, column=2, sticky="nswe", padx=2, pady=2)

        baudrate_frame = tk.LabelFrame(self.main_frame, text='Baudrate')
        self.baudrate_var = tk.StringVar()
        baudrate_entry = tk.Entry(baudrate_frame, textvariable=self.baudrate_var)
        baudrate_entry.grid(sticky="nswe")
        baudrate_frame.grid(row=1, column=2, sticky="nswe", padx=2, pady=2)

        datasize_frame = tk.LabelFrame(self.main_frame, text='Data size')
        self.datasize_var = tk.StringVar()
        datasize_entry = tk.Entry(datasize_frame, textvariable=self.datasize_var)
        datasize_entry.grid(sticky="nswe")
        datasize_frame.grid(row=2, column=2, sticky="nswe", padx=2, pady=2)

        parity_frame = tk.LabelFrame(self.main_frame, text='Parity')
        self.parity_var = tk.StringVar()
        parity_entry = tk.Entry(parity_frame, textvariable=self.parity_var)
        parity_entry.grid(sticky="nswe")
        parity_frame.grid(row=3, column=2, sticky="nswe", padx=2, pady=2)

        btn_port_open = tk.Button(self.main_frame, text="Open port", command=self.port_open)
        btn_port_open.grid(row=4, column=2, sticky="nswe", padx=2, pady=2)

        btn_port_close = tk.Button(self.main_frame, text="Close port", command=self.port_close)
        btn_port_close.grid(row=5, column=2, sticky="nswe", padx=2, pady=2)

        btn_save_log = tk.Button(self.main_frame, text="Save log", command=self.save_log_file)
        btn_save_log .grid(row=6, column=2, sticky="nswe", padx=2, pady=2)
        

        ###############################################
        sign_controll_frame = tk.Frame(self.main_frame)
        
        msg_frame = tk.LabelFrame(sign_controll_frame, text='Message')
        self.msg_var = tk.StringVar()
        msg_entry = tk.Entry(msg_frame, textvariable=self.msg_var)
        msg_entry.grid(sticky="nswe")
        msg_frame.grid(row=0, column=0, sticky="nswe", padx=2, pady=2)
        
        btn_key_gen = tk.Button(sign_controll_frame, text="New Keys", command=self.keygen)
        btn_key_gen.grid(row=1, column=0, sticky="nswe", padx=2, pady=2)
        
        btn_sign = tk.Button(sign_controll_frame, text="Sign", command=self.sign)
        btn_sign.grid(row=2, column=0, sticky="nswe", padx=2, pady=2)
        
        btn_verify = tk.Button(sign_controll_frame, text="Verify", command=self.verify)
        btn_verify.grid(row=3, column=0, sticky="nswe", padx=2, pady=2)
        
        btn_reset = tk.Button(sign_controll_frame, text="Reset", command=self.reset)
        btn_reset.grid(row=4, column=0, sticky="nswe", padx=2, pady=2)

        sign_controll_frame.grid(row=0, column=3, rowspan=10, sticky="nswe")
        self.main_frame.grid(row=0, column=0, sticky="nswe")
        
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        
    def on_closing(self):
        self.port_close()
        self.root.destroy()


    def tk_mainloop(self):
        self.root.mainloop()
        
    def keygen(self):
        msg = bytes([self.DS2_KEYGEN_START_TASK, 0])
        if self.ser is not None and self.ser.is_open:
            self.listbox.insert(self.index, "START KEYGEN")
            self.index += 1
            self.ser.write(msg)
    
    def sign(self):
        pass
    
    def verify(self):
        pass
    
    def reset(self):
        self.signer.reset()
        self.listbox.insert(self.index, "KEYS ARE DELETED")
        self.index += 1
        self.keygen_time = 0
        self.sing_time = 0
        self.verify_time = 0
        msg = bytes([self.DS2_ABORT, 0])
        if self.ser is not None and self.ser.is_open:
            self.ser.write(msg)

    def port_open(self):
        self.listbox.delete(0, tk.END)
        self.index = 0
        if self.read_thread is None:
            self.thread_run = True
            self.listbox.insert(self.index, "THREAD START")
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
                self.listbox.insert(self.index, "port is closed")
                self.index += 1

            self.ser = None
            self.listbox.insert(self.index, "THREAD STOP")
            self.index += 1

    def save_log_file(self):
        file = fd.asksaveasfile(initialfile='Untitled.log',defaultextension=".log",filetypes=[("All Files","*.*"),("Logs","*.log")])
        file_text = self.listbox.get(2, tk.END)
        for line in file_text:
            file.write(line)
        file.close()
        
    def abort(self, err_code:int):
        if self.ser is not None and self.ser.is_open:
            self.ser.write(err_code.to_bytes(1, 'little'))
            
    def response(self, msg_code, data:bytes=None):
        if self.ser is not None and self.ser.is_open:
            #msg_code + node_id
            wr_data = msg_code.to_bytes(1, 'little') + 0xff.to_bytes(1, 'little')
            if data is not None:
                wr_data += data
            data_len = len(wr_data)
            
            self.ser.write(data_len.to_bytes(4, 'little') + wr_data)
        
        
    def msg_parser(self, msg:bytes):
        msg_code = msg[0]
        node_id = msg[1]
        data = msg[2:]
        try:
            match msg_code:
                case self.DS2_COORDINATOR_HELLO:
                    self.listbox.insert(self.index, "Party ID:{} connected".format(node_id))
                    self.index += 1
            
                case self.DS2_Pi_COMMIT:
                    self.listbox.insert(self.index, "Party ID:{} Pi commit len {}".format(node_id, len(data)))
                    self.index += 1
                    self.signer.set_pi_commit(node_id, data)
                
                case self.DS2_Pi_VALUE:
                    self.listbox.insert(self.index, "Party ID:{} Pi value len {}".format(node_id, len(data)))
                    self.index += 1
                    self.signer.set_pi_val(node_id, data)
                    if self.signer.is_flag_ready(self.DS2_Pi_COMMIT_FLAG) and self.signer.is_flag_ready(self.DS2_Pi_VALUE_FLAG):
                        rho, cpu_cycles = self.signer.get_rho()
                        self.keygen_time += cpu_cycles
                        self.listbox.insert(self.index, "KeyGen: send rho value {} time {}".format(rho, self.keygen_time))
                        self.index += 1
                        self.response(self.DS2_Pi_VALUE_ACK, rho)
                
                case self.DS2_Ti_COMMIT:
                    self.listbox.insert(self.index, "Party ID:{} Ti commit len {}".format(node_id, len(data)))
                    self.index += 1
                    self.signer.set_ti_commit(node_id, data)
                
                case self.DS2_Ti_VALUE:
                    self.listbox.insert(self.index, "Party ID:{} Ti value len {}".format(node_id, len(data)))
                    self.index += 1
                    self.signer.set_ti_val(node_id, data)
                    if self.signer.is_flag_ready(self.DS2_Ti_COMMIT_FLAG) and self.signer.is_flag_ready(self.DS2_Ti_VALUE_FLAG):
                        tr, cpu_cycles = self.signer.get_tr()
                        self.keygen_time += cpu_cycles
                        self.listbox.insert(self.index, "KeyGen: send tr value {} time {}".format(tr, self.keygen_time))
                        self.index += 1
                        self.response(self.DS2_Ti_VALUE_ACK, tr)
                    
                case self.DS2_Fi_COMMIT:
                    self.listbox.insert(self.index, "Party ID:{} Fi commit value {}".format(node_id, data))
                    self.index += 1
                    self.signer.set_fi_commit(node_id, data)
                    if self.signer.is_flag_ready(self.DS2_Fi_COMMIT_FLAG):
                        sc, cpu_cycles = self.signer.get_c()
                        self.sign_time += cpu_cycles
                        self.listbox.insert(self.index, "Sign: send sc value {} time {}".format(sc, self.sign_time))
                        self.index += 1
                        self.response(self.DS2_Fi_COMMIT_ACK, sc)
                    
                case self.DS2_Ri_VALUE:
                    self.listbox.insert(self.index, "Party ID:{} Ri value len {}".format(node_id, len(data)))
                    self.index += 1
                    self.signer.set_ri_val(node_id, data)
                    
                case self.DS2_Zi_1_VALUE:
                    self.listbox.insert(self.index, "Party ID:{} Zi_1 value len {}".format(node_id, len(data)))
                    self.index += 1
                    self.signer.set_zi_1_val(node_id, data)

                case self.DS2_Zi_2_VALUE:
                    self.listbox.insert(self.index, "Party ID:{} Zi_2 value len {}".format(node_id, len(data)))
                    self.index += 1
                    self.signer.set_zi_2_val(node_id, data)
                    if self.signer.is_flag_ready(self.DS2_Ri_VALUE_FLAG) and \
                        self.signer.is_flag_ready(self.DS2_Zi_1_VALUE_FLAG) and \
                        self.signer.is_flag_ready(self.DS2_Zi_2_VALUE_FLAG):
                        signature, cpu_cycles = self.signer.get_signature()
                        self.sign_time += cpu_cycles
                        self.listbox.insert(self.index, "Sign: signature value: {}".format(":".join("{:02x}".format(c) for c in signature)))
                        self.index += 1
                        #self.response(self.DS2_Zi_2_VALUE_FLAG)
            
                case _:
                    self.listbox.insert(self.index, "unknown msg_code:{} len {}".format(msg_code, node_id, len(data)))
                    self.index += 1
                    
        except ds2.DS2Exception as err:
             self.listbox.insert(self.index, "Error: {} node_id:{}".format(str(err), node_id)) #TODO - paint red
             self.index += 1
             err_code = self.signer.translate_exception()
             self.abort(err_code)
        

    def read_serial_thread(self):
        try:
            self.ser = serial.Serial(port=self.port_var.get(),
                                     baudrate=int(self.baudrate_var.get()),
                                     bytesize=8,
                                     parity=serial.PARITY_NONE,
                                     timeout=1)
            #self.ser.open()
            if self.ser.is_open:
                self.listbox.insert(self.index, "port is opened")
                self.index += 1


                while self.thread_run:
                    read_len = self.ser.inWaiting()
                    if read_len > 3:
                        frame_len = self.ser.read(4)
                        data_len = int.from_bytes(frame_len, byteprder='little', signed=False)
                        data = self.ser.read(data_len)
                        self.msg_parser(data)
                        print(data_len)
                        print(":".join("{:02x}".format(c) for c in data))

                    #time.sleep(0.1)

        except Exception as err:
            #messagebox.showerror('Error', str(err))
            print(str(err))
            self.listbox.insert(self.index, str(err))
            self.index += 1
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