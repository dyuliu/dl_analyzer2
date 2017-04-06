'use strict';

let mongodb = require('mongodb');
let _ = require('lodash');
let client = mongodb.MongoClient;
let url = 'mongodb://msraiv:5000/final';
let cols = [
  'imagenet-8x-1'
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
  let col = db.collection(cols[idx] + '_' + 'KernelIvCosineRange');
  col.ensureIndex({ iter: 1 }, function(err, res) {
    col.ensureIndex({ lid: 1 }, function(err, res) {
      idx += 1;
      if (idx < cols.length) { act(db, idx, cb); } else { cb('done'); }
    })
  });
}