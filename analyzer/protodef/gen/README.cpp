// please refer to google protocol buffers - https://developers.google.com/protocol-buffers/docs/tutorials

/*----------------------------------------------------------------------------------*/
USE EXAMPLE - Define an Images Class
// protobuf format
message Image {                             // one image info
  optional string class_name = 1;           // *which class
  optional string file_name = 2;            // *image file name on disk
  optional int32 label_id = 3;              // *image true label
  optional int32 answer = 4;                // *image classfication result at current iteration
  repeated float prob = 5 [packed=true];    // class score (probability) vector
}

message Images {                            // an array of images info
  optional int32 iteration = 1;             // *
  repeated Image images = 2;                // *
}

// c++ code
analyzer_proto::Images test_img_info;
test_img_info.set_iteration(20000);         // set iteration attr for images
analyzer_proto::Image* img = test_img_info.add_images();    // add one image info to images
img->set_class_name('cls1');
img->set_file_name('fish3001.png');
img->set_label_id(1);                       // each class correspond to one interger
img->set_answer(10);
for (int i = 0; i < 1000; i++) {            // say there are 1000 classes in total
  img->add_prob(prob[i]);                   // prob is the computed score vector
}
// modify one image in class Images, say the first added image
test_img_info.mutable_images(0)->set_answer(95);


/*----------------------------------------------------------------------------------*/
USE EXAMPLE - Define an Info Class

// protobuf format
message HyperParameter {
  optional string type = 1;                      // stat type, e.g., mean, sum
  optional string content = 2;                   // weight or gradient
  optional float value = 3;                      // single value
  repeated float data = 4;                       // array - for sequence stat
}

message Layer {
  optional int32 count = 1;                       // number of parameters
  optional string type = 2;                       // *type of layer, including batch_norm, conv, and inner_product
  optional int32 num = 3;                         // *number of filters
  optional int32 channels = 4;                    // *number of filters of previous layer
  optional int32 height = 5;                      // *filter height
  optional int32 width = 6;                       // *filter width
  optional string name = 7;                       // *layer name
  repeated float weight = 8;                      // *weigth vector
  repeated float grad = 9;                        // *grad vector
  repeated HyperParameter stat = 10;              // computed different types of stat (i.e., single value)
  repeated HyperParameter seq = 11;               // computed different types of sequence stat (i.e., array)
  repeated HyperParameter stat_kernel = 12;       // computed different types of filter-level stat
}

message HLayer {
	optional string name = 1;                       // high level layer name
	repeated HyperParameter stat = 2;               // computed different types of stat (i.e., single value)
	repeated HyperParameter seq = 3;                // computed different types of sequence stat (i.e., array)
}

// following defines a synthesized .info
message Info {
  optional string filename = 1;     // file name used to store on disk; iteration_rank.info, e.g., 00026600_000
  optional int32 iteration = 2;     // *dumped iteration
  optional int32 worker_id = 3;     // ssgd - node id
  optional Images images = 4;       // training image infro
  repeated Layer layers = 5;        // *elementary CONV layer
  repeated HLayer h_layers = 6;     // computed high level CONV layer
}

// c++ code, all * attrs are required to fill in
analyzer_proto::Info info;
info.set_iteration(20000);                            // set iteration
analyzer_proto::Layer* nl = info.add_layers();        // add one layer
nl->set_count(9408);                                  // number of parameters, 64*3*7*7
nl->set_name('conv1');
nl->set_num(64);                                      // 64 filters in CONV1 layer
nl->set_channels(3);                                  // e.g., three color channels, R,G,B
nl->set_height(7);                                    // filter height
nl->set_width(7);                                     // filter width
nl->set_type('conv');                                 // batch_norm, conv, inner_product
// then add weight and gradient vector
for (int i = 0; i < nl->count(); i++) {               // 9408 parameters
  nl->add_grad(gradient[i]);
  nl->add_weight(weight[i]);
}

