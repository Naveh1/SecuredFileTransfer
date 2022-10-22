from dbManager import *
import _thread

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


    def regUser(self, name : str):
        try:
            self.lock.acquire()
            for value in self.clients.values():
                if value.name == name:
                    raise Exception("Client already exists")

            ID = self.db.insertClient(name)
            self.clients[ID] = Client(ID, name)
            return ID
        finally:
            self.lock.release()
    
    def signPublicKey(self, ID : str, name : str, key : bytes):
        try:
            self.lock.acquire()
            tmpID = ID.hex()

            if tmpID not in self.clients.keys():
                raise Exception("Client does not exist")
            elif self.clients[tmpID].name != name.decode('utf-8'): #removing '0' from the end
                raise  Exception("Wrong name")
            else:
                self.clients[tmpID].publicKey = key
                self.db.updateUser("PublicKey", key, tmpID)
        finally:
            self.lock.release()

    def signAESKey(self, ID : str, key : bytes):
        try:
            self.lock.acquire()
            tmpID = ID.hex()
            if tmpID not in self.clients.keys():
                raise Exception("Client does not exist")
            else:
                self.clients[tmpID].AESKey = key
                self.db.updateUser("AES_KEY", key, tmpID)
        finally:
            self.lock.release()