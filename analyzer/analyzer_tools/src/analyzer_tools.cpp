#include <assert.h>

#include <proto/analyzer_proto.pb.h>

#include <info/info.h>
#include <db/include/entry.h>
#include "analyzer_tools.h"
#include <json11.hpp>
#include <sstream>
#include <fstream>
#include <exception>

using namespace json11;

namespace analyzer_tools {

	Analyzer::Analyzer(string db_name, string model_name, string host, string layer_hierarchfile_file) {
		db_instance = new db::DB(db_name, model_name, host);

		first_parainfo = true;
		first_imginfo = true;

		rec_type = std::map < RECORD_TYPE, std::string > {
			{ RECORD_TYPE::TRAIN_ERROR, "train_error" },
			{ RECORD_TYPE::TRAIN_LOSS, "train_loss" },
			{ RECORD_TYPE::TEST_ERROR, "test_error" },
			{ RECORD_TYPE::TEST_LOSS, "test_loss" },
			{ RECORD_TYPE::FORWARD_TIME, "forward_time" },
			{ RECORD_TYPE::BACKWARD_TIME, "backward_time" },
			{ RECORD_TYPE::UPDATE_TIME, "update_time" },
			{ RECORD_TYPE::LEARNING_RATE, "learning_rate" }
		};

		std::ifstream fs(layer_hierarchfile_file);
		string content;
		string line;
		while (getline(fs, line)) {
			content += line;
		}
		fs.close();

		string err;
		Json jn = Json::parse(content.data(), err);
		if (!err.empty()) {
			throw std::exception("Failed to parse json file");
		}
		else {
			std::cout << "layer json file loaded" << std::endl;
		}

		int len = jn["length"].int_value();
		for (int i = 0; i < len; i++) {
			auto j = jn[std::to_string(i)];
			auto name = j["name"].string_value();
			if (!j["nodes"].is_null()) {				
				layer_tree_name.insert(std::pair<string, std::vector<string>>(name, {}));
				layer_tree.insert(std::pair<string, std::vector<int>>(name, {}));
				auto arr = j["nodes"].array_items();
				for (auto & k : arr) {
					auto sname = k["name"].string_value();
					layer_tree_name[name].push_back(sname);
					if (!k["nodes"].is_null()) {
						layer_tree_name.insert(std::pair<string, std::vector<string>>(sname, {}));
						layer_tree.insert(std::pair<string, std::vector<int>>(sname, {}));
						auto karr = k["nodes"].array_items();
						for (auto & l : karr) {
							auto ssname = l["name"].string_value();
							layer_tree_name[sname].push_back(ssname);
						}
					}
				}
			}
		}

		// init index
		db_instance->createIndexes();
	}

	Analyzer::~Analyzer() {
		delete db_instance;
	}

	bool Analyzer::deal_rec_info(int iteration, RECORD_TYPE type, float value) {
		// write to db
		// std::cout << "deal_rec_info" << std::endl;
		
		db_instance->importRecorderInfo(iteration, rec_type[type].c_str(), value);
		return true;
	}
	

	bool Analyzer::deal_para_info(Info &para_info_) {
		// write to db
		//std::cout << "deal_para_info " << para_info_.iteration() << std::endl;

		if (first_parainfo) {
			// turn layer_tree_name -> layer_tree, i.e., layer name -> layer id
			std::map <std::string, int> nametolid;
			for (int i = 0; i < para_info_.layers_size(); i++) {
				auto name = para_info_.layers(i).name();
				auto lid = i;
				nametolid.insert(std::pair<string, int>(name, lid));
			}

			for (auto it = layer_tree_name.begin(); it != layer_tree_name.end(); ++it) {
				for (auto &child_name : it->second) {
					if (nametolid.find(child_name) != nametolid.end()) {
						layer_tree[it->first].push_back(nametolid[child_name]);
					}
					else {
						for (auto &sub_child_name : layer_tree_name[child_name]) {
							layer_tree[it->first].push_back(nametolid[sub_child_name]);
						}
					}
				}
			}
		}

		std::shared_ptr<Infos> cur_info;
		cur_info.reset(new Infos(para_info_, layer_tree));
		//std::cout << "deal_para_stat_grad" << std::endl;
		cur_info->compute_stat_all(Infos::TYPE_CONTENT::GRAD);
		//std::cout << "deal_para_seq_grad" << std::endl;
		cur_info->compute_seq_all(Infos::TYPE_CONTENT::GRAD);
		//std::cout << "deal_para_stat_weight" << std::endl;
		cur_info->compute_stat_all(Infos::TYPE_CONTENT::WEIGHT);
		//std::cout << "deal_para_seq_weight" << std::endl;
		cur_info->compute_seq_all(Infos::TYPE_CONTENT::WEIGHT);
		//std::cout << "deal_para_kernel" << std::endl;
		cur_info->compute_stat_kernel_all(Infos::TYPE_CONTENT::WEIGHT);
		
		//std::cout << "deal_para_import_db" << std::endl;
		db_instance->bindInfo(&cur_info->get());
		db_instance->importAll();

		if (first_parainfo) {
			first_parainfo = false;
			//std::cout << "deal_para_layer_info" << std::endl;
			db_instance->importLayerAttrs();
		}
		else {
			//std::cout << "deal_para_kernel_iv" << std::endl;
			cur_info->compute_stat_kernel_all(Infos::TYPE_CONTENT::WEIGHT, pre_info->get());
			db_instance->importAllStatsKernelIV();
		}

		pre_info = cur_info;
		return true;
	}

	bool Analyzer::deal_img_info(Images &img_info, int batchsize) {
		// write to db
		//std::cout << "img_info " << img_info.iteration() << std::endl;

		if (first_imginfo) {
			for (int j = 0; j < img_info.images_size(); j++) { map_img_label[img_info.images(j).file_name()] = -1; }
		} else {
			// do check
			for (int j = 0; j < img_info.images_size(); j++) {
				std::map<std::string, int>::iterator it = map_img_label.find(img_info.images(j).file_name());
				if (it == map_img_label.end()) {
					assert("file names are not consistent");
				}
			}
		}
		db_instance->bindImgInfo(&img_info);
		// db_instance->importTestImgInfo(map_img_label, batchsize);
		db_instance->importTestImgData(first_imginfo);
		// db_instance->importTestImgStat(first_imginfo);
		first_imginfo = false;
		return true;
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
