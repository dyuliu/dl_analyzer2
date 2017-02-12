__author__ = 'liudy'

from pymongo import MongoClient

def connectDB(dbName, collectionName):
    client = MongoClient('localhost', 27017);
    db = client[dbName];
    collection = db[collectionName]
    return {'db': db, 'col': collection}
