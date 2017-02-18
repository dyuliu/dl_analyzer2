__author__ = 'dongyu'

# import numpy as np
from db import DB
from guppy import hpy
import pprint
import copy
from pymongo import ASCENDING, DESCENDING



def memory():
    h = hpy()
    print h.heap()

def fetchDataFromDB(db):
    col = db.getCol();
    one = col.find_one({}, {'_id': 0, 'iter': 1});
    result = {'iter': one['iter'], 'images': []}
    cursor = col.find({}, {'_id': 0});
    count = 0
    for d in cursor:
        result['images'].append(d);
        count += 1
        if (count % 5000 == 0):
            print count
    print 'fetch over'
    return result


def classify(data):
    i = 0
    size = len(data['images'])
    result = {'iter': data['iter'], 'images': {}}
    while i < size:
        d = data['images'][i]
        if (not result['images'].has_key(d['cls'])):
            result['images'][d['cls']] = {}
        if (not result['images'][d['cls']].has_key(d['file'])):
            result['images'][d['cls']][d['file']] = {'label': d['label'], 'correct': []};
        j = 0
        size_j = len(d['answer'])
        while j < size_j:
            cr = 1 if (d['label'] == d['answer'][j]) else 0
            result['images'][d['cls']][d['file']]['correct'].append(cr)
            j += 1
        i += 1
        if (i % 5000 == 0):
            print i
    print 'classified'
    return result

def calcAbnormal(data):
    print 'start calc abnormal'
    result = []
    count = 0
    for (key, val) in data['images'].items():
        step = 100
        i = 0
        size = len(data['iter'])
        while i < size:
            iter = data['iter'][i]
            tmp = {'cls': key, 'iter': iter, 'abLeft': [0]*step, 'abRight': [0]*step}
            for (imgkey, img) in val.items():
                # calc abnormal left
                v = img['correct']
                if ( (i - 1 >= 0) and (v[i - 1] != v[i])):
                    k = i - 1
                    left = max(0, i - step)
                    while k >= left:
                        if (v[k] == v[i]):
                            break;
                        tmp['abLeft'][i - k - 1] += 1
                        k -= 1
                # calc abnormal right
                if ( (i + 1 < size) and (v[i + 1] != v[i])):
                    k = i + 1
                    right = min(i + step, size - 1)
                    while k <= right:
                        if (v[k] == v[i]):
                            break;
                        tmp['abRight'][k - i - 1] += 1
                        k += 1
            result.append(tmp)
            i += 1
        count += 1
        print (count, key + ' done')

    return result

if __name__ == '__main__':

    db = DB('final', 'imagenet-8x-1_ImgTestData', 'localhost', 27017)
    # data = fetchDataFromDB(db)
    # data = classify(data)
    # data = calcAbnormal(data)
    # db.writeBulk(data, 'imagenet-8x-1_ImgTestStat2', 500)
    db.createIndex('imagenet-8x-1_ImgTestStat2', 'imgStat')



    # data = {
    #     'iters': [0,1,2,3,4,5,6,7,8],
    #     'img': {
    #         '1': [0,0,1,1,0,1,0,0,1],
    #         '2': [0,1,0,1,0,1,1,1,1],
    #         '3': [0,0,1,1,0,1,0,0,1],
    #         '4': [0,0,1,1,1,1,1,0,1],
    #         '5': [0,0,0,0,0,1,0,0,1]
    #     }
    # }
    # dataToWrite = calc(data)
    # disp(dataToWrite)
    # db.writeBulk(dataToWrite, 'imagenet-8x-1_ImgTestStat')
    # db.createIndex('imagenet-8x-1_ImgTestStat')


