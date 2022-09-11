from dbManager import *
import _thread

class MemoryManager:
    db : dbManager
    clients : list
    files : list
    lock : _thread.LockType
    
    def __init__(self) -> None:
        self.db = dbManager()
        self.clients, self.files = self.db.loadDB()
        self.lock = _thread.allocate_lock()

    def debug(self):
        try:
            self.lock.acquire
            print(self.clients)
        finally:
            self.lock.release()