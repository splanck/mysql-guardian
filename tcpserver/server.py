#!/usr/bin/python3
import socket
import select

class Client():
    def __init__(self, cs):
        self.client_socket = cs
        self.username = None
        self.hostname = None
        self.authenticated = False

class Server():
    header_length = 10

    IP = "127.0.0.1"
    port = 1234

    def __init__(self):
        self.sockets_list = []
        self.clients = []
        
        print("Starting server.")

    def Start(self):
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        server_socket.bind((self.IP, self.port))
        server_socket.listen(5)

        self.sockets_list.append(server_socket)

        print("Listening for connections.")
        self.GetIncomingData(server_socket)

    def GetIncomingData(self, server_socket):
        while True:
            read_sockets, _, exception_sockets = select.select(
                self.sockets_list, [], self.sockets_list)
            
            for notified_socket in read_sockets:
                print("New notified socket.")
                
                if notified_socket == server_socket:
                    client_socket, client_address = server_socket.accept()
                    print("Socket accepted")

                    user = self.receive_message(client_socket)

                    if user is False:
                        continue

                    self.sockets_list.append(client_socket)
                    # self.clients[client_socket] = user

                    c = Client(client_socket)
                    c.username = user
                    self.clients.append(c)

                    print("New connection")

                else:
                    message = self.receive_message(notified_socket)

                    if message is False:
                        print("disconnected")
                        continue

                    username = user['data'].decode('utf-8')
                    msg = message['data'].decode('utf-8')

                    print("Received message: ")
                    print(username)
                    print(msg)

                    client = self.GetClientFromSocket(notified_socket)
                    self.ProcessCommand(client, msg)

            for notified_socket in exception_sockets:
                self.sockets_list.remove(notified_socket)
                #del self.clients[notified_socket]

                for c in clients:
                    if c.client_socket == notified_socket:
                        self.clients.remove(c)

    def ProcessCommand(self, client, command):
        cmd = command[0:2]

        if cmd == "00":
            self.hostname = cmd[2:]
            return
        
        print(cmd)
        if cmd == "10":
            for c in self.clients:
                self.send_message(c, command)

            return

    def send_message(self, client, message):
        msg = message.encode("utf-8")
        message_header = f"{len(msg):<{self.header_length}}".encode("utf-8")
        client.client_socket.send(message_header + msg)

    def receive_message(self, client_socket):
        try:
            message_header = client_socket.recv(self.header_length)

            if not len(message_header):
                return False

            message_length = int(message_header.decode('utf-8'))
            message = client_socket.recv(message_length)

            return {'header': message_header, 'data': message}

        except:
            return False

    def GetClientFromSocket(self, client_socket):
        myclient = None

        for c in self.clients:
            if c.client_socket == client_socket:
                myclient = c

        return myclient

server = Server()
server.Start()
