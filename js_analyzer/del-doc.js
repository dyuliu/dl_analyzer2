'use strict';

let mongodb = require('mongodb');
let _ = require('lodash');
let client = mongodb.MongoClient;
let url = 'mongodb://msraiv:5000/final';
let cols = [
  { name: 'imagenet-8x-1', iters: [340800, 342400, 344000, 345600, 347200, 671200, 672800, 674400] },
  { name: 'imagenet-2x-lr2', iters: [310400, 312000, 313600, 315200, 316800, 318400, 310400] },
  { name: 'imagenet-2x-1', iters: [420800, 422400, 424000, 425600, 427200, 428800, 711200, 712800, 714400, 716000, 717600] },
  { name: 'imagenet-1x-m0', iters: [291200, 581200, 582800, 584400, 831200, 832800, 834400] },
  { name: 'imagenet-1x-lr2', iters: [291200, 292800, 581200, 582800, 584400] },
  { name: 'imagenet-1x-lr0.5', iters: [291200, 560400, 562000, 563600, 565200] },
  // { name: 'imagenet-1x-1', iters: [291200, 560400, 562000, 563600, 565200] },   // err!!!
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
  console.log(cols[idx].name);
  let col = db.collection(cols[idx].name + '_' + 'ImgTestInfo');
  col.remove({ iter: { $in: cols[idx].iters } }, { w: 1 }, function(err, res) {
    idx += 1;
    if (idx < cols.length) { act(db, idx, cb); } else { cb('done'); }
  });
}