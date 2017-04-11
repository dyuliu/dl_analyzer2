
#include <info/info.h>
#include <fstream>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <exception>

namespace analyzer_tools {

	namespace analyzer {

		Infos::Infos(std::string path, std::map <std::string, std::vector<int>>& layer_tree_) : map_layers(layer_tree_) {
			if (!filesystem::exist(path.c_str()))
				THROW("Error: Missing input file!");

			load_from_file(path);
			init_type_name();
			init_stat();
		}

		Infos::Infos(Info &info_, std::map <std::string, std::vector<int>>& layer_tree_) : map_layers(layer_tree_) {
			info = info_;
			init_type_name();
			init_stat();
		}


		void Infos::init_type_name() {
			name_content_type = std::map < TYPE_CONTENT, std::string > {
				{ TYPE_CONTENT::GRAD, "grad" },
				{ TYPE_CONTENT::WEIGHT, "weight" }
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
					for (int j = (int)TYPE_SEQ::CHANGERATIO; j < (int)TYPE_SEQ::END; j++) {
						auto ptr = info.mutable_layers(i)->add_seq();
						ptr->set_value(0.0);
						ptr->set_type(name_seq_type[(TYPE_SEQ)j].c_str());
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
		Infos::TYPE_SEQ Infos::to_type<Infos::TYPE_SEQ>(std::string in) {
			return type_search<TYPE_SEQ>(in, name_seq_type);
		}

		template<>
		Infos::TYPE_STAT_KERNEL Infos::to_type<Infos::TYPE_STAT_KERNEL>(std::string in) {
			return type_search<TYPE_STAT_KERNEL>(in, name_stat_kernel_type);
		}
	}

}