'use strict';

let mongodb = require('mongodb');
let _ = require('lodash');
let client = mongodb.MongoClient;
let url = 'mongodb://msraiv:5000/final';
let cols = [
  { name: 'imagenet-8x-1' },
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
  let col = db.collection(cols[idx].name + '_' + 'KernelIvCosine');
  let wCol = db.collection(cols[idx].name + '_' + 'KernelIvCosineRange');
  let count = 0;
  let ps = new Promise((resolve, reject) => {
    col.distinct('lid', function(err, lids) {
      for (let i = 0; i < lids.length; i += 1) {
        let lid = lids[i];
        col.find({ lid: lid }).sort({'iter': 1}).toArray(function(err, data) {
          // compute and insert
          let min, max;
          let tmp = [];
          _.each(data, d => {
            min = _.min(d.value);
            max = _.max(d.value);
            tmp.push({
              iter: d.iter,
              lid: d.lid,
              name: d.name,
              value: [min, max]
            });
          });
          wCol.insertMany(tmp).then(function() {
            count += 1;
            console.log(tmp.length, ': ', count, '/', lids.length);
            if (count === lids.length) { resolve('done'); }
          });
        });
      }
    })
  });

  ps.then((msg) => {
    cb('done');
  });

  // col.remove({ iter: { $in: cols[idx].iters } }, { w: 1 }, function(err, res) {
  //   idx += 1;
  //   if (idx < cols.length) { act(db, idx, cb); } else { cb('done'); }
  // });
}