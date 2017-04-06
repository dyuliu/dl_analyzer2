'use strict';

let mongodb = require('mongodb');
let _ = require('lodash');
let client = mongodb.MongoClient;
let url = 'mongodb://msraiv:5000/final';
let cols = [
  { name: 'imagenet-8x-1'},
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
  console.log(cols[idx].name);
  let col = db.collection(cols[idx].name + '_' + 'ImgTestData');
  col.distinct('file', function(err, docs) {
    for (let i = 0; i < docs.length; i += 1) {
      let file = docs[i];
      col.deleteOne({file: file});
    }
    cb('done');
  })
  // col.remove({ iter: { $in: cols[idx].iters } }, { w: 1 }, function(err, res) {
  //   idx += 1;
  //   if (idx < cols.length) { act(db, idx, cb); } else { cb('done'); }
  // });
}