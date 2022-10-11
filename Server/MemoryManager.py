from dbManager import *
import _thread

class MemoryManager:
    db : dbManager
    clients : dict
    files : list
    lock : _thread.LockType
    
    def __init__(self) -> None:
        self.db = dbManager()
        clients, self.files = self.db.loadDB()
        for client in clients:
            self.clients[client.id] = client
        self.lock = _thread.allocate_lock()

    def debug(self):
        try:
            self.lock.acquire
            print(self.clients)
        finally:
            self.lock.release()

    def regUser(self, ID : int, name : str):
        try:
            self.lock.acquire
            if ID in self.clients:
                raise "Client already exists"
            self.clients[ID] = Client(ID, name)
            #self.db.
        finally:
            self.lock.release()
    
    def signPublicKey(self, ID : int, name : str, key):
        try:
            self.lock.acquire

            if ID not in self.clients:
                raise "Client does not exist"
            elif self.clients[ID].name != name:
                raise "Wrong name"
            else:
                self.clients[ID].publicKey = key
                self.db.updateUser("PublicKey", key, ID)
        finally:
            self.lock.release()