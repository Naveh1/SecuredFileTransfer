import struct
from MemoryManager import *
from Crypto.PublicKey import RSA
from Crypto.Random import get_random_bytes
from Crypto.Cipher import AES, PKCS1_OAEP

TOTAL_LEN_WITHOUT_PAYLOAD = 23
FILE_NAME_LEN = 255
FILE_PREFIX_LEN = FILE_NAME_LEN + 4 + ID_SIZE

AES_KEY_LEN = 16
import traceback
class Request:
    clientID : str
    version : int
    code : int
    payloadSize : int
    payload : bytes

    def __init__(self, req : bytes) -> None:
        try:
            print(req)
            unpacked = struct.unpack("<%dsBHI" % (ID_SIZE), req[:TOTAL_LEN_WITHOUT_PAYLOAD])
            self.clientID = unpacked[0]
            self.version = unpacked[1]
            self.code = unpacked[2]
            self.payloadSize = unpacked[3]
            print("len: " + str(len(req[TOTAL_LEN_WITHOUT_PAYLOAD:])))
            self.payload = struct.unpack("<%ds" % (self.payloadSize), req[TOTAL_LEN_WITHOUT_PAYLOAD:])[0];
            #self.payload = unpacked[4]

            #print("Debug:")
            #print("ClientID: " + self.clientID)
            #print("Version: " + str(self.version))
            #print("Code: " + str(self.code))
            #print("PayloadSize: " + str(self.payloadSize))
            #print("Payload: " + str(self.payload))
            if self.payloadSize != len(self.payload):
                raise  Exception("Unreliable payload size")
        except Exception as e:
            traceback.print_exc()
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
        name = self.req.payload[:NAME_LEN]
        pkey = self.req.payload[NAME_LEN:]

        self.memMngr.signPublicKey(self.req.clientID, name, pkey)
        return pkey

    def genAESKey(self, pkey : str):
        #print("Hex: " + pkey.hex())
        recipient_key = RSA.importKey(pkey)
        session_key = get_random_bytes(AES_KEY_LEN)

        self.memMngr.signAESKey(self.req.clientID, session_key)

        cipher_rsa = PKCS1_OAEP.new(recipient_key)
        enc_session_key = cipher_rsa.encrypt(session_key)
        #print("enc_session_key in hex: " + str(enc_session_key.hex()))
        return enc_session_key

    def saveFileRequest(self):
        if self.req.payloadSize <= FILE_PREFIX_LEN:
            raise Exception("Illegal file packet size")
        fileInfo = struct.unpack("<%dsI%ds" % (ID_SIZE, FILE_NAME_LEN), self.req.payload[:FILE_PREFIX_LEN])
        print(self.req.payload[FILE_PREFIX_LEN:])
        print(str(len(self.req.payload[FILE_PREFIX_LEN:])) + '\t' + str(fileInfo[1]))
        fileEncContent = struct.unpack("<%ds" % (fileInfo[1]) , self.req.payload[FILE_PREFIX_LEN:])[0]

        key = self.memMngr.clients[self.req.clientID.hex()].AESKey

        cipher = AES.new(key, AES.MODE_CBC)
        plaintext = cipher.decrypt(fileEncContent)

        #return self.memMngr.saveFile(self.req.clientID, fileInfo[2] , plaintext)
        return (self.req.clientID, fileInfo[1], fileInfo[2], self.memMngr.saveFile(self.req.clientID, fileInfo[2] , plaintext))
    
    def crcRequestPayloadProcessor(payload, payloadSize : int):
        if payloadSize != ID_SIZE + FILE_NAME_LEN:
            raise Exception("Illegal crc payload size")
        return struct.unpack("%ds%ds" % (ID_SIZE, FILE_NAME_LEN), payload)
    
    def approveFile(self):
        ClientID, fileName = RequestProcessor.crcRequestPayloadProcessor(self.req.payload)

        self.memMngr.approveFile(self.req.clientID, fileName)

    def wrongFileCrc(self):
        ClientID, fileName = RequestProcessor.crcRequestPayloadProcessor(self.req.payload, self.req.payloadSize)

        self.memMngr.removeFile(self.req.clientID, fileName)


    def procReq(self):
        code = self.req.code
        if code == REGISTRATION:
            return self.regUser()
        elif code == PUBLIC_KEY:
            return self.genAESKey(self.signKey())
        elif code == SEND_FILE:
            return self.saveFileRequest()
        elif code == CRC_OK:
            self.approveFile()
        elif code == CRC_NOT_OK:
            print("CRC not okay")
        elif code == CRC_ERROR:
            self.wrongFileCrc()
        else:
            raise Exception("Invalid code")