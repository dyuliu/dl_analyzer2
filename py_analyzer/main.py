__author__ = 'dongyu'

# import numpy as np
from db import DB

def getData():
    db = DB()
    # db = DB('final', 'cifar-1x-1_ImgTestInfo')
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
    while i < length:
        print i
        bin = [data['iters'][i], [[0]*step, [0]*step]]

        values = data['img'].values()
        j = 0
        while j < len(values):
            v = values[j]

            # go left
            k = i - 1
            while k >= 0:
                if (v[k] != v[i]):
                    break;
                if (k < i - step):
                    bin[1][0][step - 1] += 1
                else:
                    bin[1][0][i - k - 1] += 1
                k -= 1
            # go right
            k = i + 1
            while k < length:
                if (v[k] != v[i]):
                    break;
                if (k > i + step):
                    bin[1][1][step - 1] += 1
                else:
                    bin[1][1][k - i - 1] += 1
                k += 1

            j += 1

        res.append(bin)
        i += 1
        if (i % 100 == 0):
            print bin
            break

if __name__ == '__main__':
    data = getData()
    calc(data)
