import struct
from RequestProcessor import *
from dbManager import ID_SIZE


REGISTRATION_SUCCESS = 2100
REGISTRATION_FAIL = 2101
SEND_AES = 2102
GOT_FILE_WITH_CRC = 2103
RECEIVED_APPROVAL = 2104

FILE_RECEIVED_PACK_SIZE = 279

class Response:
    version : int
    code : int
    payloadSize: int
    payload : any

    def __init__(self, version : int, code : int, payloadSize: int = 0, payload : any = "") -> None:
        self.version = version
        self.code = code
        self.payloadSize = payloadSize
        self.payload = payload

class ResponseProcessor:
    resp : Response
    def __init__(self, version : int, code : int, payloadSize: int = 0, info = ""):
        if code == REGISTRATION_FAIL or code == RECEIVED_APPROVAL:
            self.resp = Response(version, code, payloadSize)
        elif code == REGISTRATION_SUCCESS:
            self.resp = Response(version, code, payloadSize, info)
        elif code == SEND_AES:
            payload = info[0] + info[1]
            self.resp = Response(version, code, payloadSize, payload)
        elif code == GOT_FILE_WITH_CRC:
            self.resp = Response(version, code, FILE_RECEIVED_PACK_SIZE, struct.pack("<%dsI%dsI" % (ID_SIZE, FILE_NAME_LEN), info[0], info[1], info[2], info[3]))
        elif code == RECEIVED_APPROVAL:
            self.resp = Response(version, code)

    def serializeResponse(self) -> bytes:
        if self.resp.code == REGISTRATION_FAIL or self.resp.code == RECEIVED_APPROVAL:
            return struct.pack("<BHI", self.resp.version, self.resp.code, self.resp.payloadSize)
        elif self.resp.code == REGISTRATION_SUCCESS or self.resp.code == SEND_AES or self.resp.code == GOT_FILE_WITH_CRC:
            return struct.pack("<BHI%ds" % (self.resp.payloadSize), self.resp.version, self.resp.code, self.resp.payloadSize, self.resp.payload)