#include <google/gflags/gflags.h>
#include <assert.h>

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
		db_instance = new db::DB(db_name, model_name, host);

		first_parainfo = true;
		first_imginfo = true;

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
		db_instance->createIndexes();
	}

	Analyzer::~Analyzer() {
		delete db_instance;
	}

	bool Analyzer::deal_rec_info(int iteration, RECORD_TYPE type, float value) {
		// write to db
		std::cout << "deal_rec_info" << std::endl;
		
		db_instance->importRecorderInfo(iteration, recType[type].c_str(), value);
		return true;
	}
	

	bool Analyzer::deal_para_info(Info &para_info_) {
		// write to db
		std::cout << "deal_para_info" << std::endl;

		std::shared_ptr<Infos> cur_info;
		cur_info.reset(new Infos(para_info_));
		std::cout << "deal_para_stat_grad" << std::endl;
		cur_info->compute_stat_all(Infos::TYPE_CONTENT::GRAD);
		std::cout << "deal_para_seq_grad" << std::endl;
		cur_info->compute_seq_all(Infos::TYPE_CONTENT::GRAD);
		std::cout << "deal_para_stat_weight" << std::endl;
		cur_info->compute_stat_all(Infos::TYPE_CONTENT::WEIGHT);
		std::cout << "deal_para_seq_weight" << std::endl;
		cur_info->compute_seq_all(Infos::TYPE_CONTENT::WEIGHT);
		std::cout << "deal_para_kernel" << std::endl;
		cur_info->compute_stat_kernel_all(Infos::TYPE_CONTENT::WEIGHT);
		
		std::cout << "deal_para_import_db" << std::endl;
		db_instance->bindInfo(&cur_info->get());
		db_instance->importAll();

		if (first_parainfo) {
			first_parainfo = false;
			std::cout << "deal_para_layer_info" << std::endl;
			db_instance->importLayerAttrs();
		}
		else {
			std::cout << "deal_para_kernel_iv" << std::endl;
			cur_info->compute_stat_kernel_all(Infos::TYPE_CONTENT::WEIGHT, pre_info->get());
			db_instance->importAllStatsKernelIV();
		}

		pre_info = cur_info;
		return true;
	}

	bool Analyzer::deal_img_info(Images &img_info, int batchsize) {
		// write to db
		std::cout << "deal_img_info " << img_info.images_size() << std::endl;

		if (first_imginfo) {
			for (int j = 0; j < img_info.images_size(); j++) { map_img_label[img_info.images(j).file_name()] = -1; }
			first_imginfo = false;
		}
		db_instance->bindImgInfo(&img_info);
		db_instance->importTestImgInfo(map_img_label, batchsize);
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
		return true;
	}


}
