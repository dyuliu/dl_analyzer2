
#include <info/info.h>
#include <fstream>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <exception>

namespace analyzer_tools {

	namespace analyzer {

		Infos::Infos() {
			init_type_name();
			init_stat();
		}

		Infos::Infos(std::string path) {
			if (!filesystem::exist(path.c_str()))
				THROW("Error: Missing input file!");

			load_from_file(path);
			init_type_name();
			init_stat();
		}

		Infos::Infos(Info &info_) {
			info = info_;
			init_type_name();
			init_stat();
		}

		Infos::Infos(std::string path, int rank_size) {
			if (!filesystem::exist(path.c_str()))
				THROW("Error: Missing input file!");

			load_from_file(path);
			init_type_name();
			init_stat();
		}

		void Infos::init_type_name() {
			name_content_type = std::map < TYPE_CONTENT, std::string > {
				{ TYPE_CONTENT::GRAD, "grad" },
				{ TYPE_CONTENT::WEIGHT, "weight" }
			};

			name_distance_type = std::map < TYPE_DISTANCE, std::string > {
				{ TYPE_DISTANCE::EUCLIDEAN, "Euclidean" },
				{ TYPE_DISTANCE::COSINE, "Cosine" },
				{ TYPE_DISTANCE::MANHATTAN, "Manhattan" },
				{ TYPE_DISTANCE::CORRELATION, "Correlation" },
				{ TYPE_DISTANCE::EUCLIDEAN_NORM, "Euclidean_Norm" },
				{ TYPE_DISTANCE::COSINE_NORM, "Cosine_Norm" }
			};

			name_stat_type = std::map < TYPE_STAT, std::string > {
				{ TYPE_STAT::MAX, "max" },
				{ TYPE_STAT::MIN, "min" },
				{ TYPE_STAT::MEAN, "mean" },
				{ TYPE_STAT::SUM, "sum" },
				{ TYPE_STAT::VAR, "var" },
				{ TYPE_STAT::STD, "std" },
				// { TYPE_STAT::NORM_0,  "norm0" },
				{ TYPE_STAT::NORM_1, "norm1" },
				{ TYPE_STAT::NORM_2, "norm2" },
				{ TYPE_STAT::QUANTILE_1_2, "mid" },
				{ TYPE_STAT::QUANTILE_1_4, "quarter1" },
				{ TYPE_STAT::QUANTILE_3_4, "quarter3" }
			};

			name_seq_type = std::map < TYPE_SEQ, std::string > {
				// { TYPE_SEQ::HISTOGRAM,  "histogram"},
				{ TYPE_SEQ::CHANGERATIO, "changeratio" }
			};

			name_cluster_type = std::map < TYPE_CLUSTER, std::string > {
				{ TYPE_CLUSTER::KMEANS, "kmeans"}
			};

			name_stat_kernel_type = std::map < TYPE_STAT_KERNEL, std::string > {
				{ TYPE_STAT_KERNEL::CR_NORM_1, "cr_norm1"},
				{ TYPE_STAT_KERNEL::CR_NORM_2, "cr_norm2" },
				{ TYPE_STAT_KERNEL::W_STD, "w_std" },
				{ TYPE_STAT_KERNEL::G_NORM_1, "g_norm1" },
				{ TYPE_STAT_KERNEL::G_NORM_2, "g_norm2" },
				{ TYPE_STAT_KERNEL::I_EUCLIDEAN, "i_euclidean" },
				{ TYPE_STAT_KERNEL::I_MANHATTAN, "i_manhattan" },
				{ TYPE_STAT_KERNEL::I_COSINE, "i_cosine" },
				{ TYPE_STAT_KERNEL::I_CR_NORM_1, "i_cr_norm1" },
				{ TYPE_STAT_KERNEL::I_CR_NORM_2, "i_cr_norm2" }
			};

			// for imagenet
			map_layers = std::map < std::string, std::vector<int> > {
				{ "interstellar2a", { 6, 11, 16, 21 } },
				{ "interstellar2b", { 26, 31, 36 } },
				{ "interstellar2c", { 41, 46, 51 } },
				{ "interstellar3a", { 56, 61, 66, 71 } },
				{ "interstellar3b", { 76, 81, 86 } },
				{ "interstellar3c", { 91, 96, 101 } },
				{ "interstellar3d", { 106, 111, 116 } },
				{ "interstellar4a", { 121, 126, 131, 136 } },
				{ "interstellar4b", { 141, 146, 151 } },
				{ "interstellar4c", { 156, 161, 166 } },
				{ "interstellar4d", { 171, 176, 181 } },
				{ "interstellar4e", { 186, 191, 196 } },
				{ "interstellar4f", { 201, 206, 211 } },
				{ "interstellar5a", { 216, 221, 226, 231 } },
				{ "interstellar5b", { 236, 241, 246 } },
				{ "interstellar5c", { 251, 256, 261 } },
				{ "conv2", { 6, 11, 16, 21, 26, 31, 36, 41, 46, 51 } },
				{ "conv3", { 56, 61, 66, 71, 76, 81, 86, 91, 96, 101, 106, 111, 116 } },
				{ "conv4", { 121, 126, 131, 136, 141, 146, 151, 156, 161, 166, 171, 176, 181, 186, 191, 196, 201, 206, 211 } },
				{ "conv5", { 216, 221, 226, 231, 236, 241, 246, 251, 256, 261 } }
			};


			// for cifar
			//map_layers = std::map<std::string, std::vector<int>> {
			//	{ "interstellar2a", { 5, 10, 15, 20 } },
			//	{ "interstellar3a", { 280, 285, 290, 295 } },
			//	{ "interstellar4a", { 555, 560, 565, 570 } }
			//};
			//for (int i = 2; i <= 4; i++) {
			//	int idx;
			//	std::vector<int> convVec;

			//	if (i == 2) {
			//		idx = 25;
			//		auto vec2 = map_layers.find("interstellar2a")->second;
			//		convVec.insert(convVec.end(), vec2.begin(), vec2.end());
			//	}
			//	if (i == 3) {
			//		idx = 300;
			//		auto vec2 = map_layers.find("interstellar3a")->second;
			//		convVec.insert(convVec.end(), vec2.begin(), vec2.end());
			//	}
			//	if (i == 4) {
			//		idx = 575;
			//		auto vec2 = map_layers.find("interstellar4a")->second;
			//		convVec.insert(convVec.end(), vec2.begin(), vec2.end());
			//	}
			//	for (int j = 1; j <= 17; j++) {
			//		std::string name = "interstellar" + std::to_string(i) + "b" + std::to_string(j);
			//		int a = idx, b = idx + 5, c = idx + 10;
			//		idx = c + 5;
			//		map_layers.insert(std::map<std::string, std::vector<int>>::value_type(name, {a, b, c}));
			//		convVec.push_back(a);
			//		convVec.push_back(b);
			//		convVec.push_back(c);
			//	}
			//	std::string convName = "conv" + std::to_string(i);
			//	map_layers.insert(std::map<std::string, std::vector<int>>::value_type(convName, convVec));
			//}
		}

		void Infos::init_stat() {
			// init computational related parameters
			for (int i = 0; i < info.layers_size(); i++) {
				for (int idx = (int)TYPE_CONTENT::GRAD; idx < (int)TYPE_CONTENT::END; idx++) {
					for (int j = (int)TYPE_STAT::MAX; j < (int)TYPE_STAT::END; j++) {
						auto ptr = info.mutable_layers(i)->add_stat();
						ptr->set_value(0.0);
						ptr->set_type(name_stat_type[(TYPE_STAT)j].c_str());
						ptr->set_content(name_content_type[(TYPE_CONTENT)idx].c_str());
					}
					for (int j = (int)TYPE_DISTANCE::EUCLIDEAN; j < (int)TYPE_DISTANCE::END; j++) {
						auto ptr = info.mutable_layers(i)->add_distance();
						ptr->set_value(0.0);
						ptr->set_type(name_distance_type[(TYPE_DISTANCE)j].c_str());
						ptr->set_content(name_content_type[(TYPE_CONTENT)idx].c_str());
					}
					for (int j = (int)TYPE_SEQ::CHANGERATIO; j < (int)TYPE_SEQ::END; j++) {
						auto ptr = info.mutable_layers(i)->add_seq();
						ptr->set_value(0.0);
						ptr->set_type(name_seq_type[(TYPE_SEQ)j].c_str());
						ptr->set_content(name_content_type[(TYPE_CONTENT)idx].c_str());
					}
					for (int j = (int)TYPE_CLUSTER::KMEANS; j < (int)TYPE_CLUSTER::END; j++) {
						auto ptr = info.mutable_layers(i)->add_cluster();
						ptr->set_type(name_cluster_type[(TYPE_CLUSTER)j].c_str());
						ptr->set_num(0);
						ptr->set_content(name_content_type[(TYPE_CONTENT)idx].c_str());
					}
					for (int j = (int)TYPE_STAT_KERNEL::CR_NORM_1; j < (int)TYPE_STAT_KERNEL::END; j++) {
						auto ptr = info.mutable_layers(i)->add_stat_kernel();
						ptr->set_value(0.0);
						ptr->set_type(name_stat_kernel_type[(TYPE_STAT_KERNEL)j].c_str());
						ptr->set_content(name_content_type[(TYPE_CONTENT)idx].c_str());
					}
				}
			}

			int count = 0;
			for (auto it = map_layers.begin(); it != map_layers.end(); ++it) {
				auto l_ptr = info.add_h_layers();
				l_ptr->set_name(it->first);
				for (int idx = (int)TYPE_CONTENT::GRAD; idx < (int)TYPE_CONTENT::END; idx++) {
					for (int j = (int)TYPE_STAT::MAX; j < (int)TYPE_STAT::END; j++) {
						auto ptr = l_ptr->add_stat();
						ptr->set_value(0.0);
						ptr->set_type(name_stat_type[(TYPE_STAT)j].c_str());
						ptr->set_content(name_content_type[(TYPE_CONTENT)idx].c_str());
					}
					for (int j = (int)TYPE_SEQ::CHANGERATIO; j < (int)TYPE_SEQ::END; j++) {
						auto ptr = l_ptr->add_seq();
						ptr->set_value(0.0);
						ptr->set_type(name_seq_type[(TYPE_SEQ)j].c_str());
						ptr->set_content(name_content_type[(TYPE_CONTENT)idx].c_str());
					}
				}
			}
		}

		void Infos::load_from_file(std::string filename) {

			std::ifstream fp(filename.c_str(), std::ios::binary);

			google::protobuf::io::IstreamInputStream fstr(&fp);
			google::protobuf::io::CodedInputStream code_input(&fstr);
			code_input.SetTotalBytesLimit((int)MAX_PROTOFILE_SIZE, (int)MAX_PROTOFILE_SIZE);

			info.ParseFromCodedStream(&code_input);
			fp.close();
		}

		// dump to file
		void Infos::save_to_file(std::string foldname) {

			if (!filesystem::exist(foldname.c_str()))
				filesystem::create_directory(foldname.c_str());

			std::string filename = foldname + "/" + info.filename() + ".info";
			std::ofstream fp(filename.c_str(), std::ios::binary);

			info.SerializeToOstream(&fp);
			fp.close();
		}

		void Infos::copy_hyperparam(Infos &other, TYPE_CONTENT content_type, HyperParam hp) {

			CHECK_EQ(info.layers_size(), other.get().layers_size());

			for (int i = 0; i < info.layers_size(); i++) {
				CHECK_EQ(info.layers(i).stat_size(), other.get().layers(i).stat_size());
				if (hp == HyperParam::STAT) {
					for (int j = (int)TYPE_STAT::MAX; j < (int)TYPE_STAT::END; j++) {
						auto idx = index((TYPE_STAT)j, content_type);
						info.mutable_layers(i)->mutable_stat(idx)->CopyFrom(other.get().layers(i).stat(idx));
					}
				}
				else if (hp == HyperParam::DISTANCE) {
					for (int j = (int)TYPE_DISTANCE::EUCLIDEAN; j < (int)TYPE_DISTANCE::END; j++) {
						auto idx = index((TYPE_DISTANCE)j, content_type);
						info.mutable_layers(i)->mutable_distance(idx)->CopyFrom(other.get().layers(i).distance(idx));
					}
				}
				else if (hp == HyperParam::SEQ) {
					for (int j = (int)TYPE_SEQ::CHANGERATIO; j < (int)TYPE_SEQ::END; j++) {
						auto idx = index((TYPE_SEQ)j, content_type);
						info.mutable_layers(i)->mutable_seq(idx)->CopyFrom(other.get().layers(i).seq(idx));
					}
				}
			}

		}

		template<typename T>
		inline static T type_search(std::string e, std::map<T, std::string> s) {
			for (auto name : s)
				if (e == name.second)
					return name.first;
			THROW("Could not find specify type!");
		}

		template<>
		Infos::TYPE_STAT Infos::to_type<Infos::TYPE_STAT>(std::string in) {
			return type_search<TYPE_STAT>(in, name_stat_type);
		}

		template<>
		Infos::TYPE_CONTENT Infos::to_type<Infos::TYPE_CONTENT>(std::string in) {
			return type_search<TYPE_CONTENT>(in, name_content_type);
		}

		template<>
		Infos::TYPE_DISTANCE Infos::to_type<Infos::TYPE_DISTANCE>(std::string in) {
			return type_search<TYPE_DISTANCE>(in, name_distance_type);
		}

		template<>
		Infos::TYPE_SEQ Infos::to_type<Infos::TYPE_SEQ>(std::string in) {
			return type_search<TYPE_SEQ>(in, name_seq_type);
		}

		template<>
		Infos::TYPE_CLUSTER Infos::to_type<Infos::TYPE_CLUSTER>(std::string in) {
			return type_search<TYPE_CLUSTER>(in, name_cluster_type);
		}

		template<>
		Infos::TYPE_STAT_KERNEL Infos::to_type<Infos::TYPE_STAT_KERNEL>(std::string in) {
			return type_search<TYPE_STAT_KERNEL>(in, name_stat_kernel_type);
		}
	}

}