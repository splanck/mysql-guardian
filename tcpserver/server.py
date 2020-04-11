import socket
import select

class Server():
    header_length = 10

    IP = "127.0.0.1"
    port = 1234
    sockets_list = [] 
    clients = {}

    def __init__(self):
        print("Starting server.")
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        server_socket.bind((self.IP, self.port))
        server_socket.listen(5)

        self.sockets_list.append(server_socket)

        print("Listening for connections.")
        self.ProcessIncomingData(server_socket)

    def ProcessIncomingData(self, server_socket):
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
                    self.clients[client_socket] = user

                    print("New connection")

                else:
                    message = self.receive_message(notified_socket)

                    if message is False:
                        print("disconnected")
                        #sockets_list.remove(notified_socket)
                        #del clients[notified_socket]
                        continue

                    user = self.clients[notified_socket]
                    username = user['data'].decode('utf-8')
                    msg = message['data'].decode('utf-8')

                    print("Received message: ")
                    print(username)
                    print(msg)

                    for client_socket in self.clients:
                        client_socket.send(user['header'] + user['data'] + 
                                           message['header'] + message['data'])

            for notified_socket in exception_sockets:
                self.sockets_list.remove(notified_socket)
                del self.clients[notified_socket]

    def receive_message(self, client_socket):
        try:
            message_header = client_socket.recv(self.header_length)

            if not len(message_header):
                return False

            message_length = int(message_header.decode('utf-8'))

            return {'header': message_header, 'data': 
                    client_socket.recv(message_length)}

        except:
            return False

server = Server()
