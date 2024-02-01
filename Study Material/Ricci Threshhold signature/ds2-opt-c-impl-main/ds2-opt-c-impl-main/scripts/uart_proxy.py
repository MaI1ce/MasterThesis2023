import serial
import time
import socket
import threading

N_PARTIES = 10

class Proxy:
    def __init__(self):
        self.local_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
        self.local_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)

        self.clients = {}
        self.buffers = [b'' for _ in range(N_PARTIES)]
        self.threads = []

    def start_listening(self):
        self.local_socket.bind(('127.0.0.1', 9000))
        self.local_socket.listen()

    def accept_clients(self):
        for _ in range(N_PARTIES - 1):
            client, _ = self.local_socket.accept()

            client_id = client.recv(5).rstrip(b'\x00')
            client_id = int(client_id.decode())
            self.clients[client_id] = client

            thread = threading.Thread(target=self.handle_client, args=(client, client_id))

            thread.start()

            self.threads.append(thread)

    def handle_client(self, client, client_id):
        print(f'Handling client: {client_id}')
        while True:
            buffer = client.recv(16384)

            if len(buffer) == 0:
                break

            print(f'Received data from client {client_id}: {len(buffer)}')

            self.buffers[client_id - 1] += buffer

        print(f'Closing client: {client_id}')
        client.close()

    def connect_to_arduino(self):
        self.arduino = serial.Serial(port='/dev/cu.usbmodem11301', baudrate=115200, timeout=1)

        time.sleep(2)

    def run(self):
        self.arduino.write(b'Start')
        print(self.arduino.readline())

        while True:
            command = self.arduino.read_until(b';').decode()
            if len(command) == 0:
                break

            if command.startswith('gen_key') or command.startswith('sign') or command.startswith('verify') or command.startswith('ignore') or command.startswith('bench'):
                if len(command.split('\n')) > 1:
                    result, command = command.rsplit('\n', 1)
                else:
                    result, command = command, ''

                print(result)

            if len(command) == 0 or command == ';':
                continue

            print(f'Received command from Arduino: {command}')

            for command in command.split(';'):
                if len(command) == 0:
                    break
                client_id, rw, data_len = command.split(':')
                client_id = int(client_id)
                data_len = int(data_len)

                if rw == 'w':
                    while data_len != 0:
                        buffer = self.arduino.read_until(size=data_len)
                        while len(buffer) != data_len:
                            buffer += self.arduino.read_until(size=data_len - len(buffer))
                        print(f'Received data from Arduino: {len(buffer)}/{data_len}')
                        self.clients[client_id].send(buffer)
                        data_len -= len(buffer)
                else:
                    print(f'Buffer({client_id}) length: {len(self.buffers[client_id - 1])}')
                    data = self.buffers[client_id - 1][:data_len]
                    self.buffers[client_id - 1] = self.buffers[client_id - 1][data_len:]
                    print(f'Sending data to Arduino: {len(data)}/{data_len}')
                    self.arduino.write(data)


    def end(self):
        self.local_socket.shutdown(socket.SHUT_RDWR)

proxy = Proxy()

proxy.start_listening()

for i in range(20):
    print(f'Running iteration: {i}')
    proxy.accept_clients()
    if i == 0:
        proxy.connect_to_arduino()
    proxy.run()

proxy.end()

for thread in proxy.threads:
    thread.join()