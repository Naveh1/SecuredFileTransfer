import sqlite3
from sqlite3.dbapi2 import Connection, Date
from urllib.request import pathname2url
from Crypto.Cipher import AES
from Crypto.PublicKey import RSA


DB_NAME = "server.db"
BITS = 8

ID_SIZE = 16
NAME_SIZE = 127
PUBLIC_KEY_SIZE = 160
AES_KEY_SIZE = 256 / BITS
FILE_NAME_SIZE = 256
FILE_PATH_SIZE = 256


class Client:
    ID : int
    name : str
    publicKey : RSA.RsaKey
    lastSeen : Date
    AESKey : bytes

    def __init__(self, id : int, name : str, publicKey, lastSeen, AESKey) -> None:
        self.ID = id
        self.name = name
        self.publicKey = publicKey
        self.lastSeen = lastSeen
        self.AESKey = AESKey

class file:
    ID : int
    FileName : str
    PathName : str
    Verified : bool

    def __init__(self, id : int, fileName : str, pathName : str, verified : bool) ->None:
        self.ID = id
        self.fileName = fileName
        self.pathName = pathName
        self.verified = verified

class dbManager:
    conn : Connection
    def __init__(self, dbName : str = DB_NAME) -> None:
        self.createDB()

    def createDB(self, dbName : str = DB_NAME):
        self.conn = sqlite3.connect(dbName)
        cur = self.conn.cursor()

        clientsTable = """CREATE TABLE IF NOT EXISTS clients(
        ID CHAR( %d ) NOT NULL PRIMARY KEY,
        Name CHAR( %d ) NOT NULL,
        PublicKey CHAR( %d ),
        LastSeen DATE,
        AES_KEY BINARY( %d )
        );""" % (ID_SIZE, NAME_SIZE, PUBLIC_KEY_SIZE, AES_KEY_SIZE)

        filesTable = """CREATE TABLE IF NOT EXISTS files(
        ID CHAR( %d ) NOT NULL,
        FileName CHAR( %d ) NOT NULL,
        FilePath CHAR( %d ) NOT NULL,
        Verified BOOLEAN,
        
        PRIMARY KEY(ID, FileName, FilePath),
        FOREIGN KEY(ID) REFERENCES clients(ID)
        );""" %  (ID_SIZE, FILE_NAME_SIZE, FILE_PATH_SIZE)

        try:
            cur.execute(clientsTable)
            cur.execute(filesTable)
        finally:
            self.conn.commit()
            cur.close()
        

    def loadDB(self) -> tuple:
        cur = self.conn.cursor()

        cur.execute("SELECT * FROM clients")
        clientsRows = cur.fetchall()

        cur.execute("SELECT * FROM files")
        filesRows = cur.fetchall()

        clients = list()
        files = list()

        for clientRow in clientsRows:
            clients.append(Client(clientRow[0], clientRow[1], clientRow[2], clientRow[3]))
        for fileRow in filesRows:
            files.append(File(fileRow[0], fileRow[1], fileRow[2], fileRow[3]))
        return clients, files