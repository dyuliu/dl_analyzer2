__author__ = 'dongyu'

# import numpy as np
from db import DB
# from guppy import hpy
import pprint
import copy
from pymongo import ASCENDING, DESCENDING



# def memory():
#     h = hpy()
#     print h.heap()

def fetchDataFromDB(db):
    col = db.getCol();
    one = col.find_one({}, {'_id': 0, 'iter': 1});
    result = {'iter': one['iter'], 'images': []}
    cursor = col.find({}, {'_id': 0});
    count = 0
    for d in cursor:
        result['images'].append(d);
        count += 1
        # if (count % 5000 == 0):
        #     print count
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
        # if (i % 5000 == 0):
        #     print i
    print 'classified'
    return result

def getClassList(data):
    result = []
    for (key, val) in data['images'].items():
        result.append({'cls': key})
    return result

def calcStat(data):
    print 'start calc stat'
    result = []
    count = 0
    for (key, val) in data['images'].items():
        step = 100
        i = 0
        size = len(data['iter'])
        while i < size:
            iter = data['iter'][i]
            tmp = {'cls': key, 'iter': iter, 'abLeft': [0]*step, 'abRight': [0]*step, 'testError': 0}
            correctCount = 0
            for (imgkey, img) in val.items():
                # calc abnormal left
                v = img['correct']
                correctCount += v[i]
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
            tmp['testError'] = 1 - 1.0 * correctCount / len(val.items())
            result.append(tmp)
            i += 1
        count += 1
        # print (count, key + ' done')

    return result

def aggStat(data):
    print 'start aggregate stat'
    result = {}
    size = len(data)
    step = 100
    i = 0
    while i < size:
        iter = data[i]['iter']
        if (not result.has_key(iter)):
            result[iter] = {'iter': iter, 'abLeft': [0]*step, 'abRight': [0]*step}
        j = 0
        while j < step:
            result[iter]['abLeft'][j] += data[i]['abLeft'][j]
            result[iter]['abRight'][j] += data[i]['abRight'][j]
            j += 1
        i += 1
    return result

if __name__ == '__main__':

    dbList = [
        'imagenet-8x-1',
        'imagenet-2x-lr2',
        'imagenet-2x-1',
        'imagenet-1x-m0',
        'imagenet-1x-lr2',
        'imagenet-1x-lr0.5',
        'imagenet-1x-1',
        'cifar-8x-1',
        'cifar-4x-1',
        'cifar-2x-lr2',
        'cifar-2x-lr0.5',
        'cifar-2x-1',
        'cifar-1x-m0',
        'cifar-1x-lr2',
        'cifar-1x-lr0.5',
        'cifar-1x-2',
        'cifar-1x-1'
    ]

    for dbname in dbList:
        print dbname
        db = DB('final', dbname + '_ImgTestData', 'msraiv', 5000)
        data = fetchDataFromDB(db)
        data = classify(data)

        clsList = getClassList(data)
        db.write(clsList, dbname + '_ClsInfo')

        clsData = calcStat(data)
        db.write(clsData, dbname + '_ImgTestClsStat')
        db.createIndex(dbname + '_ImgTestClsStat', 'imgClsStat')

        aggData = aggStat(clsData)
        db.write(aggData, dbname + '_ImgTestStat')
        db.createIndex(dbname + '_ImgTestStat', 'imgStat')
