import socket
import _thread

PORT_FILE_NAME = "port.info"
DEFAULT_PORT = 1234
LOCAL_HOST = "127.0.0.1"

class Server:
    host : str
    port : int

    def __init__(self, portFile = PORT_FILE_NAME) -> None:
        self.host = LOCAL_HOST
        try:
            with open(portFile, "r") as f:
                self.port = int(f.read())
        except FileNotFoundError:
            print("Warning: " + portFile + " does not exist, using the default port: " + str(DEFAULT_PORT))
            self.port = DEFAULT_PORT
        except Exception:
            print("Warning: error reading " + portFile + ", using the default port: " + str(DEFAULT_PORT))
            self.port = DEFAULT_PORT

    def listen(self):
        with socket.socket() as sock:
            sock.bind((self.host, self.port))
            sock.listen(5)
            print("Listening on port " + str(self.port))
            while True:
                Client, address = sock.accept()
                print('Connected to: ' + address[0] + ':' + str(address[1]))
                _thread.start_new_thread(self.manageClient, (Client, ))

    def manageClient(self, connection):
        print("Client accepted")
        connection.close()



def main():
    server = Server()
    server.listen()
            
if __name__ == "__main__":
    main()