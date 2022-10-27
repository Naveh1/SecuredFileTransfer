from dbManager import *
import _thread
import os
import crc

SERVER_DIR = "serverFiles"

class MemoryManager:
    db : dbManager
    clients : dict
    files : list
    lock : _thread.LockType

    
    def __init__(self) -> None:
        self.db = dbManager()
        self.clients = dict()
        self.files = dict()
        clients, self.files = self.db.loadDB()
        for client in clients:
            self.clients[client.ID] = client
        self.lock = _thread.allocate_lock()
        MemoryManager.createDir()
    
    def createDir(path : str = SERVER_DIR):
        if not os.path.exists(path):
            os.makedirs(path)
        else:
            print("File already exists: " + path)

    def writeFile(path : str, content):
        with open(path, "wb") as f:
            f.write(content)

    def deleteFile(path : str):
        os.remove(path)

    def existsFile(path):
        return os.path.exists(path) 

    def regUser(self, name : str):
        try:
            self.lock.acquire()
            for value in self.clients.values():
                if value.name == name:
                    raise Exception("Client already exists")

            ID = self.db.insertClient(name)
            #name = self.db.getClient(ID)
            self.clients[ID.hex()] = Client(ID.hex(), name)
            #self.client[ID] = Client(ID, self.db.getClient(ID))
            return ID
        finally:
            self.lock.release()
    
    def signPublicKey(self, ID : str, name : str, key : bytes):
        #print("Public key: " + str(key))
        try:
            self.lock.acquire()
            tmpID = ID.hex()

            if tmpID not in self.clients.keys():
                if len(self.db.getClient(ID)) == 0:
                    raise Exception("Client does not exist")
            elif self.clients[tmpID].name != name.decode('utf-8'): #removing '0' from the end
                raise Exception("Wrong name")

            self.clients[tmpID].publicKey = key
            self.db.updateUser("PublicKey", key, tmpID)
        finally:
            self.lock.release()

    def signAESKey(self, ID : str, key : bytes):
        try:
            self.lock.acquire()
            tmpID = ID.hex()
            if tmpID not in self.clients.keys():
                if len(self.db.getClient(ID)) == 0:
                    raise Exception("Client does not exist")

            self.clients[tmpID].AESKey = key
            self.db.updateUser("AES_KEY", key, tmpID)
        finally:
            self.lock.release()
    
    def saveFile(self, ID : str, fileName : str, content):
        try:
            self.lock.acquire()
            tmpID = ID.hex()
            if tmpID not in self.clients.keys():
                if len(self.db.getClient(ID)) == 0:
                    raise Exception("Client does not exist")
            count = len(self.files)
            currFile = str(count)
            fullPath = SERVER_DIR + "/" + str(tmpID) + "/" + currFile
            while MemoryManager.existsFile(fullPath):
                count += 1
                currFile = str(count)
                fullPath = SERVER_DIR + "/" + str(tmpID) + "/" + currFile

            found = False
            for f in self.files:
                if f.ID == tmpID and f.fileName == fileName:
                    if f.verified:
                        raise Exception("File already exists")
                    found = True
                    break
            if not found:
                MemoryManager.createDir(SERVER_DIR + "/" + tmpID)
            
            #fullPath = SERVER_DIR + "/" + str(tmpID) + "/" + bytes.fromhex(fileName.hex().rstrip('00')).decode("utf-8")
            #fullPath = SERVER_DIR + "/" + str(tmpID) + "/" + str(fileName)

            MemoryManager.writeFile(fullPath, content)
            if not found:
                self.db.insertFile(tmpID, fileName, fullPath)
                self.files.append(File(tmpID, fileName, fullPath))

            crcVal : crc.crc32 = crc.crc32()
            crcVal.update(content)
            return crcVal.digest()
            #return binascii.crc32(content)
        finally:
            self.lock.release()

    def approveFile(self, ID : str, fileName : str):
        try:
            self.lock.acquire()
            tmpID = ID.hex()
            if tmpID not in self.clients.keys():
                if len(self.db.getClient(ID)) == 0:
                    raise Exception("Client does not exist")
            
            found = False
            for f in self.files:
                if f.ID == tmpID and f.fileName == fileName:
                    f.verified = True
                    found = True
                    break
            if not found:
                raise Exception("File doesn't exist")

            self.db.validateFile(ID, fileName)
        finally:
            self.lock.release()

    def removeFile(self, ID : str, fileName : str):
        try:
            self.lock.acquire()
            tmpID = ID.hex()
            if tmpID not in self.clients.keys():
                if len(self.db.getClient(ID)) == 0:
                    raise Exception("Client does not exist")
            
            fullPath = str()
            found = False
            for f in self.files:
                if f.ID == tmpID and f.fileName == fileName:
                    found = True
                    fullPath = f.pathName
                    break
            if not found:
                raise Exception("File doesn't exist")

            MemoryManager.deleteFile(fullPath)

            self.db.removeFile(ID, fileName)
        finally:
            self.lock.release()