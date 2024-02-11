import threading
import tkinter as tk
from tkinter import messagebox
from tkinter import filedialog as fd
from tkinter import ttk
from threading import Thread
import serial
import time


class Sniffer:
    def __init__(self):
        self.root = tk.Tk()
        self.main_frame = tk.Frame(self.root)
        self.ser = None
        self.msg_string = ""
        self.thread_run = False
        self.read_thread = None
        self.index = 0
        self._lock = threading.Lock()

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

        self.main_frame.grid(row=0, column=0, sticky="nswe")

    def tk_mainloop(self):
        self.root.mainloop()

    def port_open(self):
        self.listbox.delete(0, tk.END)
        self.index = 0
        if self.read_thread is None:
            self.thread_run = True
            self.listbox.insert(self.index, "Thread start")
            self.read_thread = Thread(target=self.read_serial_thread)
            self.read_thread.start()
            self.index += 1

    def port_close(self):
        self.thread_run = False
        if self.read_thread is not None:
            self.read_thread.join()
            self.read_thread = None
            self.listbox.insert(self.index, "THREAD is closed")
            self.index += 1

    def save_log_file(self):
        file = fd.asksaveasfile(initialfile='Untitled.log',defaultextension=".log",filetypes=[("All Files","*.*"),("Logs","*.log")])
        file_text = self.listbox.get(2, tk.END)
        for line in file_text:
            file.write(line)
        file.close()

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
            data_str = ''

            while self.thread_run:
                if self.ser.inWaiting() > 0:
                    data_str += self.ser.readline().decode('latin-1').replace('\0', '')
                    print(data_str)
                    self.listbox.insert(self.index, data_str)
                    self.index += 1
                    data_str = ''
                time.sleep(0.1)

            print('loop break')

        except Exception as err:
            messagebox.showerror('Error', str(err))
        finally:
            print('thread end')
            if self.ser is not None:
                #self.listbox.insert(self.index, "port is closed")
                #self.index += 1
                self.ser.close()
            self.ser = None


if __name__ == "__main__":
    sniffer = Sniffer()
    try:
        sniffer.tk_mainloop()
    finally:
        sniffer.port_close()