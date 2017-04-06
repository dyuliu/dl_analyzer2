#include <google/gflags/gflags.h>
#include <assert.h>

DEFINE_string(action, "recorder", "recorder, stat, distance, batch");
DEFINE_string(src, "running_info_0.log", "the specify file/folder path");

DEFINE_string(type, "", "specify output type");
DEFINE_string(hp, "", "stat, dist, seq");
DEFINE_string(content, "weight", "grad or weight");
DEFINE_string(framework, "caffepro", "caffepro, caffe, torch, cntk");

DEFINE_uint64(batchsize, 1, "batch size of records");
DEFINE_uint64(imgbatch, 1000, "batch size of test images");
DEFINE_uint64(interval, 1, "specify output interval");
DEFINE_uint64(maxlayer, 2, "specify max layers to calculate");

DEFINE_bool(all, false, "if output all type info");
DEFINE_bool(db, false, "if upload to db");
DEFINE_bool(parse, false, "parse log file to recorders");

DEFINE_string(dbname, "", "sub-database name");
DEFINE_string(database, "", "database name");

#define CHECK_FLAGS_SRC {if (!FLAGS_src.size()) assert(!"Missing src path!");}
#define CHECK_FLAGS_IMGBATCH {if (!FLAGS_imgbatch) assert(!"Error input of test image batchsize!");}
#define CHECK_FLAGS_TYPE {if (!FLAGS_type.size()) assert(!"Missing specify output type!");}
#define CHECK_FLAGS_HP {if (!FLAGS_hp.size()) assert(!"Missing specify hyperparameter type!");}
#define CHECK_FLAGS_BATCHSIZE {if (!FLAGS_batchsize) assert(!"Error input of batchsize!");}
#define CHECK_FLAGS_INTERVAL {if (FLAGS_interval<=0) assert(!"The interval should be larger than 0!");}
#define CHECK_FLAGS_CONTENT {if (FLAGS_content!="grad"&&FLAGS_content!="weight") assert(!"content value is grad or weight");}
#define CHECK_FLAGS_FRAMEWORK {if (!FLAGS_framework.size()) assert(!"Please enter caffepro, caffe, torch, cntk");}


#include <config.h>

#include <fstream>
#include <proto/analyzer_proto.pb.h>

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <info/info.h>
#include <recorder/recorder.h>
#include <db/include/entry.h>

#include <omp.h>

#include <emath\include\config.h>
#include <emath\include\distance.h>

#include "analyzer_tools.h"


namespace analyzer_tools {

	Analyzer::Analyzer(string db_name, string model_name, string host) {
		dbInstance = new db::DB(FLAGS_database, FLAGS_dbname, "msraiv:5000");

		firstParaInfo = true;
		firstImgInfo = true;

		recType = std::map < RECORD_TYPE, std::string > {
			{ RECORD_TYPE::TRAIN_ERROR, "train_error" },
			{ RECORD_TYPE::TRAIN_LOSS, "train_loss" },
			{ RECORD_TYPE::TEST_ERROR, "test_error" },
			{ RECORD_TYPE::TEST_LOSS, "test_loss" },
			{ RECORD_TYPE::FORWARD_TIME, "forward_time" },
			{ RECORD_TYPE::BACKWARD_TIME, "backward_time" },
			{ RECORD_TYPE::UPDATE_TIME, "update_time" },
			{ RECORD_TYPE::LEARNING_RATE, "learning_rate" }
		};

		// init index
		dbInstance->createIndexes();
	}

	Analyzer::~Analyzer() {
		delete dbInstance;
	}

	bool Analyzer::deal_rec_info(int iteration, RECORD_TYPE type, float value) {
		// write to db
		dbInstance->importRecorderInfo(iteration, recType[type].c_str(), value);
		return true;
	}
	

	bool Analyzer::deal_para_info(Info &para_info_) {
		// write to db
		std::shared_ptr<Infos> cur_info;
		cur_info.reset(new Infos(para_info_));
		cur_info->compute_stat_all(Infos::TYPE_CONTENT::GRAD);
		cur_info->compute_seq_all(Infos::TYPE_CONTENT::GRAD);
		cur_info->compute_stat_all(Infos::TYPE_CONTENT::WEIGHT);
		cur_info->compute_seq_all(Infos::TYPE_CONTENT::WEIGHT);
		cur_info->compute_stat_kernel_all(Infos::TYPE_CONTENT::WEIGHT);
		
		dbInstance->bindInfo(&cur_info->get());
		dbInstance->importAll();

		if (firstParaInfo) {
			firstParaInfo = false;
			dbInstance->importLayerAttrs();
		}
		else {
			cur_info->compute_stat_kernel_all(Infos::TYPE_CONTENT::WEIGHT, pre_info->get());
			dbInstance->importAllStatsKernelIV();
		}

		pre_info = cur_info;
		return true;
	}

	bool Analyzer::deal_img_info(Images &img_info_, int batchsize) {
		// write to db
		Images img_info = img_info_;
		if (firstImgInfo) {
			for (int j = 0; j < img_info.images_size(); j++) { map_img_label[img_info.images(j).file_name()] = -1; }
			firstImgInfo = false;
		}
		dbInstance->bindImgInfo(&img_info);
		dbInstance->importTestImgInfo(map_img_label, batchsize);
		return true;
	}

	void print() {
		std::cout << "lib print test" << std::endl;
	}

	bool save_image(char *data, int length, string cls_name, string img_name) {
		string file = cls_name + "\\" + img_name;
		std::ofstream out(file, std::ios::binary);
		out.write((const char*)data, sizeof(unsigned char) * length);
		out.close();
		// free(data);
	}


}
