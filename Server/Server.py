import socket
import _thread
from MemoryManager import *
from RequestProcessor import *
from ResponseProcessor import *

PORT_FILE_NAME = "port.info"
DEFAULT_PORT = 1234
LOCAL_HOST = "127.0.0.1"

VERSION = 3

class Server:
    host : str
    port : int
    memMngr : MemoryManager

    def __init__(self, portFile = PORT_FILE_NAME) -> None:
        self.memMngr = MemoryManager()
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
                _thread.start_new_thread(self.manageClient, (Client, address))
    
    def manageClient(self, connection, address):
        print(f"Connection by {address}")
        with connection:
            self.memMngr.debug() #test

            req = connection.recv(1024)
            try:
                reqProc : RequestProcessor = RequestProcessor(req, self.memMngr)
            except Exception:
                pass #if code 2100 - REGISTRATION FAILED RESPONSE: CODE 2101
            if reqProc.getCode() == REGISTRATION:
                try:
                    ID = reqProc.procReq()
                    respProc = ResponseProcessor(VERSION, REGISTRATION_SUCCESS, ID_SIZE, ID)
                    connection.sendall(respProc.serializeResponse())
                except Exception:
                    respProc = ResponseProcessor(VERSION, REGISTRATION_FAIL)
                    connection.sendall(respProc.serializeResponse())
            elif reqProc.getCode() == PUBLIC_KEY:
                try:
                    reqProc.procReq()
                except Exception:
                    print("Error occurred with no response specification") #I DONT KNOW WHAT SHOULD I DO HERE 



def main():
    server = Server()
    server.listen()
            
if __name__ == "__main__":
    main()