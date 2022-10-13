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


    def regUser(self, name : str):
        try:
            self.lock.acquire()

            for key, value in self.clients:
                if value.name == name:
                    raise Exception("Client already exists")

            ID = self.db.insertClient(name)
            self.clients[ID] = Client(ID, name)
        finally:
            self.lock.release()
    
    def signPublicKey(self, ID : int, name : str, key):
        try:
            self.lock.acquire()

            if ID not in self.clients:
                raise Exception("Client does not exist")
            elif self.clients[ID].name != name:
                raise  Exception("Wrong name")
            else:
                self.clients[ID].publicKey = key
                self.db.updateUser("PublicKey", key, ID)
        finally:
            self.lock.release()