__author__ = 'dongyu'

# import numpy as np
from db import DB

def getData(db):
    col = db.getCol();
    data = {'iters': [], 'img': {}}
    for d in col.find({}, {'iter': 1, 'file': 1, 'label': 1, 'answer': 1}):
        data['iters'].append(d['iter'])
        size = len(d['file'])
        i = 0
        while (i < size):
            file = d['file'][i]
            label = d['label'][i]
            answer = d['answer'][i]
            correct = (label == answer)
            if (data['img'].has_key(file) == False):
                data['img'][file] = []

            data['img'][file].append(correct);
            i += 1
    return data

def calc(data):
    # print (data['iters'])
    print 'start calc'
    i = 0
    step = 100
    res = []
    length = len(data['iters'])
    # iterate all iterations
    while i < length:
        bin = {'iter': data['iters'][i], 'left': [0]*step, 'right': [0]*step}
        values = data['img'].values()
        j = 0
        # iterate all imgages
        while j < len(values):
            v = values[j]
            # go left
            # if ( (i - 1 >= 0) and (v[i - 1] != v[i]) ):
            #     k = i - 1
            #     left = max(0, i - step)
            #     while k >= left:
            #         if (v[k] == v[i]):
            #             break;
            #         bin['left'][i - k - 1] += 1
            #         k -= 1
            # go right
            if ( (i + 1 < length) and (v[i + 1] != v[i])):
                k = i + 1
                right = min(i + step, length - 1)
                while k <= right:
                    if (v[k] == v[i]):
                        break;
                    bin['right'][k - i - 1] += 1
                    k += 1
            j += 1

        j = step - 2
        # iterate step to accumulate bin
        while j >= 0:
            bin['left'][j] += bin['left'][j+1]
            bin['right'][j] += bin['right'][j+1]
            j -= 1

        res.append(bin)
        i += 1
        if (i % 100 == 0):
            print ('calc ', i)

    return res

def disp(data):
    meanL = [0]*100
    meanR = [0]*100
    sumL = [0]*100
    sumR = [0]*100
    maxL = [-1]*100
    maxLi = [-1]*100
    maxR = [-1]*100
    maxRi = [-1]*100
    i = 0
    while i < len(data):
        for j in range(100):
            sumL[j] += data[i]['left'][j]
            sumR[j] += data[i]['right'][j]
            if data[i]['left'][j] > maxL[j]:
                maxL[j] = data[i]['left'][j]
                maxLi[j] = data[i]['iter']
            if data[i]['right'][j] > maxR[j]:
                maxR[j] = data[i]['right'][j]
                maxRi[j] = data[i]['iter']
        i += 1
    for i in range(100):
        print ('L' + str(i), maxLi[i], sumL[i] / len(data), maxL[i])
    for i in range(100):
        print ('R' + str(i), maxRi[i], sumR[i] / len(data), maxR[i])

if __name__ == '__main__':
    db = DB('final', 'imagenet-1x-lr2_ImgTestInfo')
    data = getData(db)
    dataToWrite = calc(data)
    disp(dataToWrite)
    # db.writeBulk(dataToWrite, 'imagenet-1x-lr2_ImgTestStat')
    # db.createIndex('imagenet-1x-lr2_ImgTestStat')


