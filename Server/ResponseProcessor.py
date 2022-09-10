from dbManager import *
import _thread

class ResponseProcessor:
    db : dbManager
    clients : list
    files : list
    lock = pass
    
    def __init__(self) -> None:
        self.db = dbManager()
        self.clients, self.files = self.db.loadDB()
        self.lock = _thread.allocate_lock()