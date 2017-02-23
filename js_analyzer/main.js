'use strict';

let mongodb = require('mongodb');
let _ = require('lodash');
let client = mongodb.MongoClient;
let url = 'mongodb://msraiv:5000/final';
let cols = [
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
];
let globalData;

client.connect(url, function(err, db) {
  if (err) {
    console.log('Unable to connect to the mongoDB server. Error:', err);
  } else {
    console.log('Connection established to', url);
    act(db, 0, function(res) {
      console.log(res);
      db.close();
    });
  }
});

function act(db, idx, cb) {
  console.log(cols[idx]);
  let rCol = db.collection(cols[idx] + '_' + 'ImgTestData');

  rCol.find({}).toArray(function(err, data) {
    if (err) {
      console.log(err);
    } else if (data.length) {
      let tmp = { 'iter': data[0].iter };
      tmp.images = _.map(data, d => {
        return {
          file: d.file,
          cls: d.cls,
          label: d.label,
          correct: _.map(d.answer, dd => dd === d.label ? 1 : 0)
        };
      });
      globalData = classify(tmp);
      writeBulk(db.collection(cols[idx] + '_' + 'ClsInfo'), getClassInfo(globalData), () => {
        globalData = calcStat(globalData);
        console.log(globalData.length);
        writeBulk(db.collection(cols[idx] + '_' + 'ImgTestClsStat'), globalData, () => {
          globalData = aggStat(globalData);
          console.log(globalData.length);
          writeBulk(db.collection(cols[idx] + '_' + 'ImgTestStat'), globalData, () => {
            idx += 1;
            if (idx < cols.length) { act(db, idx, cb); }
              else { cb('done'); }
          });
        });
      });

    }
  });
}

function classify(data) {
  console.log('start to classify data by cls');
  let size = data.images.length;
  let result = { iter: data.iter, images: {} };
  for (let i = 0; i < size; i += 1) {
    let d = data.images[i];
    if (!result.images[d.cls]) { result.images[d.cls] = {}; }
    if (!result.images[d.cls][d.file]) { result.images[d.cls][d.file] = { label: d.label, correct: d.correct } };
  }
  return result;
}

function getClassInfo(data) {
  let result = [];
  _.each(data.images, (val, key) => {
    result.push({
      name: key,
      size: _.size(val)
    });
  })
  return result;
}

function calcStat(data) {
  console.log('start to calc stat');
  let result = [];
  let step = 100;
  let size = data.iter.length;
  let count = 0;
  _.each(data.images, (val, key) => {
    for (let i = 0; i < size; i += 1) {
      let iter = data.iter[i];
      let tmp = { cls: key, iter, abLeft: Array(step).fill(0), abRight: Array(step).fill(0), testError: 0 };
      let correctCount = 0;
      _.each(val, (img, imgkey) => {
        let v = img.correct;
        correctCount += v[i];
        // left abnormal
        if ((i - 1 >= 0) && (v[i - 1] != v[i])) {
          let k = i - 1;
          let left = Math.max(0, i - step);
          while (k >= left) {
            if (v[k] === v[i]) break;
            tmp.abLeft[i - k - 1] += 1;
            k -= 1;
          }
        }
        // right abnormal
        if ((i + 1 < size) && (v[i + 1] != v[i])) {
          let k = i + 1;
          let right = Math.min(i + step, size - 1);
          while (k <= right) {
            if (v[k] === v[i]) break;
            tmp.abRight[k - i - 1] += 1;
            k += 1;
          }
        }
      });
      tmp.testError = 1 - 1.0 * correctCount / _.size(val);
      result.push(tmp);
    }
  });
  return result;
}

function aggStat(data) {
  console.log('start to aggregate stat');
  let result = {};
  let size = data.length;
  let step = 100;
  for (let i = 0; i < size; i += 1) {
    let iter = data[i].iter;
    if (!result[iter]) {
      result[iter] = { iter, abLeft: Array(step).fill(0), abRight: Array(step).fill(0) }
    }
    for (let j = 0; j < step; j += 1) {
      result[iter]['abLeft'][j] += data[i]['abLeft'][j]
      result[iter]['abRight'][j] += data[i]['abRight'][j]
    }
  }
  return _.values(result);
}

function writeBulk(col, data, cb) {
  col.insertMany(data).then(cb);
  // let batch;

  // batch = col.initializeUnorderedBulkOp({ useLegacyOps: true });
  // for (let d of data) { batch.insert(d); }
  // batch.execute(function(err, res) {
  //   if (err) {
  //     console.log(err);
  //   } else {
  //     cb();
  //   }
  // });
}