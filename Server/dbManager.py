import sqlite3
from sqlite3.dbapi2 import Connection, Cursor
from urllib.request import pathname2url

DB_NAME = "server.db"
BITS = 8

ID_SIZE = 16
NAME_SIZE = 127
PUBLIC_KEY_SIZE = 160
AES_KEY_SIZE = 256 / BITS
FILE_NAME_SIZE = 256
FILE_PATH_SIZE = 256

class dbManager():


    def __init__(self, dbName : str = DB_NAME) -> None:
        try:
            dburi = str('file:{}?mode=rw').format(pathname2url(DB_NAME))
            conn = sqlite3.connect(dburi, uri=True)
            conn.close()
        except sqlite3.OperationalError:
            self.createDB(dbName)

    def createDB(self, dbName : str = DB_NAME):
        conn = sqlite3.connect()
        cur = conn.cursor()

        clientsTable = """CREATE TABLE clients(
        ID CHAR({ID_SIZE}) NOT NULL PRIMARY KEY,
        Name CHAR({NAME_SIZE}) NOT NULL,
        PublicKey char({PUBLIC_KEY_SIZE}),
        LastSeen DATE,
        AES_KEY BINARY({AES_KEY_SIZE})
        );"""

        filesTable = """CREATE TABLE files(
        ID CHAR({ID_SIZE}) NOT NULL,
        FileName CHAR({FILE_NAME_SIZE}) NOT NULL,
        FilePath CHAR({FILE_PATH_SIZE}) NOT NULL,
        Verified BOOLEAN,
        
        PRIMARY KEY(ID, FileName, FilePath),
        FOREIGN KEY(ID) REFERENCES clients(ID)
        );"""


        cur.execute(clientsTable)
        cur.execute(filesTable)

        conn.commit()
        conn.close()

