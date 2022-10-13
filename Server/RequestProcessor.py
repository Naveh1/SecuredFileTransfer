import struct
from MemoryManager import *

TOTAL_LEN_WITHOUT_PAYLOAD = 23

class Request:
    clientID : str
    version : int
    code : int
    payloadSize : int
    payload : bytes

    def __init__(self, req : bytes) -> None:
        try:
            print(req[:TOTAL_LEN_WITHOUT_PAYLOAD])
            print(req)
            unpacked = struct.unpack("<16sBHI", req[:TOTAL_LEN_WITHOUT_PAYLOAD])
            self.clientID = unpacked[0].decode("utf-8")
            self.version = unpacked[1]
            self.code = unpacked[2]
            self.payloadSize = unpacked[3]

            self.payload = struct.unpack("<%ds" % (self.payloadSize), req[TOTAL_LEN_WITHOUT_PAYLOAD:])[0];
            #self.payload = unpacked[4]

            print("Debug:")
            print("ClientID: " + self.clientID)
            print("Version: " + str(self.version))
            print("Code: " + str(self.code))
            print("PayloadSize: " + str(self.payloadSize))
            print("Payload: " + str(self.payload))
            if self.payloadSize != len(self.payload):
                raise  Exception("Unreliable payload size")
        except Exception as e:
            raise  Exception("Bad request")

REGISTRATION = 1100
PUBLIC_KEY = 1101
SEND_FILE = 1103
CRC_OK = 1104
CRC_NOT_OK = 1105
CRC_ERROR = 1106

NAME_LEN = 255
PUBLIC_KEY_LEN = 160

class RequestProcessor:
    req : Request
    memMngr : MemoryManager
    
    def __init__(self, req, memMngr : MemoryManager) -> None:
        self.req = Request(req);
        self.memMngr = memMngr

    def getCode(self):
        return self.req.code

    def regUser(self):
        if self.req.payloadSize != NAME_LEN:
            raise  Exception("Illegal register size")
        return self.memMngr.regUser(self.req.payload.decode('utf-8'))

    def signKey(self):
        if self.req.payloadSize != NAME_LEN + PUBLIC_KEY_LEN:
            raise  Exception("Illegal register size")
        name = self.req.payload.decode('utf-8')[:NAME_LEN]
        pkey = self.req.payload[NAME_LEN:]

        self.memMngr.signPublicKey(self.req.clientID, name, pkey)

    def procReq(self):
        code = self.req.code
        if code == REGISTRATION:
            return self.regUser()
        elif code == PUBLIC_KEY:
            self.signKey()
        elif code == SEND_FILE:
            pass
        elif code == CRC_OK:
            pass
        elif code == CRC_NOT_OK:
            pass
        elif code == CRC_ERROR:
            pass
        else:
            raise Exception("Invalid code")