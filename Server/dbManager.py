import sqlite3
from sqlite3.dbapi2 import Connection, Date
from typing import final
from urllib.request import pathname2url
from Crypto.Cipher import AES
from Crypto.PublicKey import RSA
import datetime
import uuid


DB_NAME = "server.db"
BITS = 8

ID_SIZE = 16
NAME_SIZE = 127
PUBLIC_KEY_SIZE = 160
AES_KEY_SIZE = 256 / BITS
FILE_NAME_SIZE = 256
FILE_PATH_SIZE = 256


class Client:
    ID : str
    name : str
    publicKey : bytes
    lastSeen : str
    AESKey : bytes

    def __init__(self, ID : str, name : str, publicKey = b"\0", lastSeen = datetime.datetime.now().strftime("%d/%m/%Y %H:%M:%S") ,AESKey = b"\0") -> None:
        self.ID = ID
        self.name = name
        self.publicKey = publicKey
        self.lastSeen = lastSeen
        self.AESKey = AESKey

class File:
    ID : int
    FileName : str
    PathName : str
    Verified : bool

    def __init__(self, id : int, fileName : str, pathName : str, verified : bool = False) ->None:
        self.ID = id
        self.fileName = fileName
        self.pathName = pathName
        self.verified = verified

class dbManager:
    conn : Connection
    def __init__(self, dbName : str = DB_NAME) -> None:
        try:
            self.conn = sqlite3.connect('file:%s?mode=rw' % (dbName), uri=True, check_same_thread=False)
        except Exception:
            self.createDB()

    def createDB(self, dbName : str = DB_NAME):
        self.conn = sqlite3.connect(dbName, check_same_thread=False)
        cur = self.conn.cursor()

        clientsTable = """CREATE TABLE IF NOT EXISTS clients(
        ID CHAR( %d ) NOT NULL PRIMARY KEY,
        Name CHAR( %d ) NOT NULL,
        PublicKey BLOB( %d ),
        LastSeen DATE NOT NULL,
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
            cur.close()
            self.conn.commit()
        

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

    def insertClient(self, name):
        cur = self.conn.cursor()
        error : bool = True

        while error:
            try:
                cur.execute("INSERT INTO clients(ID, Name, LastSeen) VALUES(?, ?, datetime('now','localtime'))", (uuid.uuid4().hex, name))
                error = False
            except Exception:
                error = True
        #print("Name: " + str(name))
        cur.execute("SELECT ID from clients WHERE NAME = ?", (name, ))
        ID = bytes.fromhex(cur.fetchall()[0][0])

        cur.close()
        self.conn.commit()

        return ID

    def insertFile(self, ID, FileName, FilePath, verified : bool = False):
        cur = self.conn.cursor()
        
        cur.execute("INSERT INTO clients(ID, FileName, FilePath, Verified) VALUES(?, ?, datetime('now','localtime'))", (ID, FileName, FilePath, verified))
        #print("Name: " + str(name))
        cur.close()
        self.conn.commit()
    
    def updateUser(self, column : str, value, key):
        cur = self.conn.cursor()
        cur.execute("UPDATE clients SET %s = ? , LastSeen = datetime('now','localtime') WHERE ID = ?" % (column), (value, key))
        cur.close()
        self.conn.commit()
        self.conn.commit()
             
    def getClient(self, ID) -> str:
        cur = self.conn.cursor()
        #print("Name: " + str(name))
        cur.execute("SELECT Name from clients WHERE ID = ?", (ID.hex(), ))

        try:
            return cur.fetchall()[0][0]
        except Exception:
            return ""
        finally:
            cur.close()

