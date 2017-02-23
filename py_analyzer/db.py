# -*- coding: utf-8 -*-
__author__ = 'liudy'

from pprint import pprint
from pymongo import MongoClient, ASCENDING, DESCENDING
from pymongo.errors import BulkWriteError

class DB:

    def __init__(self, db_ = 'final', col_ = 'imagenet-1x-lr2_ImgTestInfo', address = 'localhost', port = 27017 ):
        client = MongoClient(address, port)
        # client = MongoClient(address, port, maxPoolSize=250, waitQueueMultiple=500, waitQueueTimeoutMS=5000)
        self.db = client[db_]
        self.col = client[db_][col_]

        print (address + ':' + str(port) + '/' + db_ + '.' + col_)

    def getDb(self):
        return self.db

    def getCol(self):
        return self.col

    def writeMany(self, data, col):
        self.db[col].insert_many(data)

    def write(self, data, col):
        i = 1
        if (isinstance(data, dict)):
            for (k, v) in data.items():
                self.db[col].insert(v)
                i += 1
                if (i % 10000 == 0):
                    print i
        elif (isinstance(data, list)):
            for v in data:
                self.db[col].insert(v)
                i += 1
                if (i % 10000 == 0):
                    print i

    def writeBulk(self, data, col, batch = 50, type = 'unordered'):
        # init
        if type == 'unordered':
            bulk = self.db[col].initialize_unordered_bulk_op()
        elif type == 'ordered':
            bulk = self.db[col].initialize_ordered_bulk_op()

        print ('start writing data to ' + col)
        # do insertion
        i = 1
        finished = False
        if (isinstance(data, dict)):
            for (k, v) in data.items():
                bulk.insert(v)
                finished = False
                if (i % batch == 0):
                    try:
                        bulk.execute()
                        finished = True
                    except BulkWriteError as bwe:
                        pprint(bwe.details)
                    if type == 'unordered':
                        bulk = self.db[col].initialize_unordered_bulk_op()
                    elif type == 'ordered':
                        bulk = self.db[col].initialize_ordered_bulk_op()
                i += 1
                if (i % 10000 == 0):
                    print i
        elif (isinstance(data, list)):
            for v in data:
                bulk.insert(v)
                finished = False
                if (i % batch == 0):
                    try:
                        bulk.execute()
                        finished = True
                    except BulkWriteError as bwe:
                        pprint(bwe.details)
                    if type == 'unordered':
                        bulk = self.db[col].initialize_unordered_bulk_op()
                    elif type == 'ordered':
                        bulk = self.db[col].initialize_ordered_bulk_op()
                i += 1
                if (i % 10000 == 0):
                    print i

        if (not finished):
            try:
                bulk.execute()
            except BulkWriteError as bwe:
                pprint(bwe.details)

    def createIndex(self, col, type):
        if (type == 'imgClsStat'):
            self.db[col].create_index([('iter', ASCENDING)])
            self.db[col].create_index([('cls', ASCENDING)])
            self.db[col].create_index([('iter', ASCENDING), ('cls', ASCENDING)])
        if (type == 'imgStat'):
            self.db[col].create_index([('iter', ASCENDING)])
        # self.db[col].create_index([('iter', ASCENDING, 'batch', ASCENDING)])
        # col.find({}).sort('iter').explain()['cursor']
        # col.find({}).sort('iter').explain()['nscanned']


# if __name__ == '__main__':
#     son = DB()
#     print('类型帮助信息: ', DB.__doc__)
#     print('类型名称:', DB.__name__)
#     print('类型所继承的基类:', DB.__bases__)
#     print('类型字典:', DB.__dict__)
#     print('类型所在模块:', DB.__module__)
#     print('实例类型:', DB().__class__)

