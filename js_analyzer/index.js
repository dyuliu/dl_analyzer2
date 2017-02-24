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
  // 'imagenet-1x-1',
  // 'cifar-8x-1',
  // 'cifar-4x-1',
  // 'cifar-2x-lr2',
  // 'cifar-2x-lr0.5',
  // 'cifar-2x-1',
  // 'cifar-1x-m0',
  // 'cifar-1x-lr2',
  // 'cifar-1x-lr0.5',
  // 'cifar-1x-2',
  // 'cifar-1x-1'
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
  let col = db.collection(cols[idx] + '_' + 'ClsInfo');
  col.ensureIndex({ name: 1 }, function(err, res) {
    col = db.collection(cols[idx] + '_' + 'ImgTestClsStat');
    col.ensureIndex({ iter: 1 }, function(err, res) {
      col.ensureIndex({ cls: 1 }, function(err, res) {
        col = db.collection(cols[idx] + '_' + 'ImgTestStat');
        col.ensureIndex({ iter: 1 }, function(err, res) {
          idx += 1;
          if (idx < cols.length) { act(db, idx, cb); } else { cb('done'); }
        });
      });
    })
  });
}