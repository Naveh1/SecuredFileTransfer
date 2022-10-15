import struct


REGISTRATION_SUCCESS = 2100
REGISTRATION_FAIL = 2101
SEND_AES = 2102
RECEIVED_APPROVAL = 2104

class Response:
    version : int
    code : int
    payloadSize: int
    payload : any

    def __init__(self, version : int, code : int, payloadSize: int, payload : any = "") -> None:
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
            payload = info[0].encode('utf-8') + info[1]
            self.resp = Response(version, code, payloadSize, payload)

    def serializeResponse(self) -> bytes:
        if self.resp.code == REGISTRATION_FAIL or self.resp.code == RECEIVED_APPROVAL:
            return struct.pack("<BHI", self.resp.version, self.resp.code, self.resp.payloadSize)
        elif self.resp.code == REGISTRATION_SUCCESS or resp.code == SEND_AES:
            return struct.pack("<BHI%ds" % (self.resp.payloadSize), self.resp.version, self.resp.code, self.resp.payloadSize, self.resp.payload)