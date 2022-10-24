import socket
import _thread
from MemoryManager import *
from RequestProcessor import *
from ResponseProcessor import *
import traceback

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

    def processManagingClient(self, req, expectedReq, repeats = 0):
        doRepeat = True
        reqProc : RequestProcessor = RequestProcessor(req, self.memMngr)
            if reqProc.getCode() == REGISTRATION:
                if expectedReq != REGISTRATION:
                    raise Exception("Unexpected registration")
                try:
                    ID = reqProc.procReq()
                    respProc = ResponseProcessor(VERSION, REGISTRATION_SUCCESS, ID_SIZE, ID)
                    connection.sendall(respProc.serializeResponse())
                    expectedReq = PUBLIC_KEY
                except Exception as e:
                    traceback.print_exc()
                    respProc = ResponseProcessor(VERSION, REGISTRATION_FAIL)
                    print("Pack: " + str(respProc.serializeResponse())) # debug

                    connection.sendall(respProc.serializeResponse())

                    print("User error: " + str(e))
            elif reqProc.getCode() == PUBLIC_KEY:
                if expectedReq != REGISTRATION and expectedReq != PUBLIC_KEY:
                    raise Exception("Unexpected sending of public key")
                try:
                    EncAesKey = reqProc.procReq()
                    info = (reqProc.req.clientID, EncAesKey)
                    respProc = ResponseProcessor(VERSION, SEND_AES, len(info[0]) + len(info[1]), info)

                    print("public key Pack: " + str(respProc.serializeResponse())) # debug
                    connection.sendall(respProc.serializeResponse())

                    expectedReq = SEND_FILE
                except Exception:
                    traceback.print_exc()
                    print("Error occurred with no response specification") #I DONT KNOW WHAT SHOULD I DO HERE 
            elif reqProc.getCode() == SEND_FILE:
                if expectedReq != SEND_FILE:
                    raise Exception("Unexpected sending of file")
                elif repeats >= TIMES:
                    raise Exception("Too many resends!")
                try:
                    payload = reqProc.procReq()
                    respProc = ResponseProcessor(VERSION, GOT_FILE_WITH_CRC, -1, payload)
                    connection.sendall(respProc.serializeResponse())
                    repeats += 1
                    expectedReq = CRC_OK
                except Exception as e:
                    traceback.print_exc()
                    print("User error: " + str(e))
                    doRepeat = False
            elif reqProc.getCode() == CRC_OK or reqProc.getCode() == CRC_NOT_OK or reqProc.getCode() == CRC_ERROR:
                if expectedReq != CRC_OK:
                    raise Exception("Unexpected sending of file")
                elif reqProc.getCode() == CRC_OK or reqProc.getCode() == CRC_ERROR:
                    doRepeat = False
                elif repeats >= TIMES:
                    raise Exception("Resending too many times")
                else:
                    repeats += 1
                    
                try:
                    reqProc.procReq()
                    respProc = ResponseProcessor(VERSION, RECEIVED_APPROVAL)
                    connection.sendall(respProc.serializeResponse())
                except Exception as e:
                    traceback.print_exc()
                    print("Crc error: " + str(e))
                    doRepeat = False
            return doRepeat, expectedReq, repeats

    def manageClient(self, connection, address):
        print(f"Connection by {address}")
        toContinue : bool = True
        with connection:
            while toContinue:
                try:
                    print("Receiving...")
                    req = connection.recv(1024)
                    
                    if len(req) == 0:
                        print("Client has disconnected")
                        break
                except Exception:
                    print("Error receiving request")
                    break
                #print(f"request received: {req}")
                
                try:
                    reqProc : RequestProcessor = RequestProcessor(req, self.memMngr)
                    if reqProc.getCode() == REGISTRATION:
                        try:
                            ID = reqProc.procReq()
                            respProc = ResponseProcessor(VERSION, REGISTRATION_SUCCESS, ID_SIZE, ID)
                            connection.sendall(respProc.serializeResponse())
                        except Exception as e:
                            traceback.print_exc()
                            respProc = ResponseProcessor(VERSION, REGISTRATION_FAIL)
                            print("Pack: " + str(respProc.serializeResponse())) # debug

                            connection.sendall(respProc.serializeResponse())

                            print("User error: " + str(e))
                    elif reqProc.getCode() == PUBLIC_KEY:
                        try:
                            EncAesKey = reqProc.procReq()
                            info = (reqProc.req.clientID, EncAesKey)
                            respProc = ResponseProcessor(VERSION, SEND_AES, len(info[0]) + len(info[1]), info)

                            print("public key Pack: " + str(respProc.serializeResponse())) # debug
                            connection.sendall(respProc.serializeResponse())
                        except Exception:
                            traceback.print_exc()
                            print("Error occurred with no response specification") #I DONT KNOW WHAT SHOULD I DO HERE 
                    elif reqProc.getCode() == SEND_FILE:
                        try:
                            payload = reqProc.procReq()
                            respProc = ResponseProcessor(VERSION, GOT_FILE_WITH_CRC, -1, payload)
                            connection.sendall(respProc.serializeResponse())
                        except Exception as e:
                            traceback.print_exc()
                            print("User error: " + str(e))
                            break
                except Exception as e:  
                    traceback.print_exc()
                    #print(e)
                    #pass #if code 2100 - REGISTRATION FAILED RESPONSE: CODE 2101
                #toContinue = False


def main():
    server = Server()
    server.listen()
            
if __name__ == "__main__":
    main()