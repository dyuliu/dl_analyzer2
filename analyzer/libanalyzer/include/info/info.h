
#pragma once


#include <config.h>
#include <memory>
#include <map>

namespace analyzer_tools {


namespace analyzer {

	class Infos {

	public:

		enum class TYPE_STAT : unsigned int {
			MAX		= 0U,
			MIN		= 1U,
			MEAN	= 2U,
			SUM		= 3U,
			VAR		= 4U,
			STD		= 5U,
			// NORM_0	= 6U,
			NORM_1	= 7U,
			NORM_2	= 8U,
			QUANTILE_1_2 = 9U,
			QUANTILE_1_4 = 10U,
			QUANTILE_3_4 = 11U,
			END
		};

		enum class TYPE_STAT_KERNEL : unsigned int {
			CR_NORM_1 = 0U,  // cr for change ratio
			CR_NORM_2 = 1U,
			W_STD = 2U,		 // w for weight
			G_NORM_1 = 3U,	 // g for gradient
			G_NORM_2 = 4U,
			I_EUCLIDEAN = 5U,// i for interval
			I_MANHATTAN = 6U,
			I_COSINE = 7U,
			I_CR_NORM_1 = 8U,
			I_CR_NORM_2 = 9U,
			END
		};

		enum class TYPE_SEQ : unsigned int {
			CHANGERATIO = 0U,
			// HISTOGRAM = 1U,
			END
		};

		enum class TYPE_CONTENT : unsigned int {
			GRAD	= 0U,
			WEIGHT	= 1U,
			END
		};

		enum class HyperParam : unsigned int {
			STAT = 0U,
			DISTANCE = 1U,
			SEQ = 2U
		};

	public:

		// dump to file
		void save_to_file(std::string foldname);

		// load from file
		void load_from_file(std::string filename);

	public:

		// get
		unsigned int index(TYPE_STAT stat_type, TYPE_CONTENT data_content);
		unsigned int index(TYPE_SEQ seq_type, TYPE_CONTENT data_content);
		unsigned int index(TYPE_STAT_KERNEL stat_kernel_type, TYPE_CONTENT data_content);

		// string to type
		template <typename Tout>
		Tout to_type(std::string);

		// stat
		void compute_stat(TYPE_STAT stat_type, TYPE_CONTENT data_content);
		void compute_stat_list(std::vector<TYPE_STAT> stat_list, TYPE_CONTENT data_content);
		void compute_stat_all(TYPE_CONTENT data_content);

		// seq
		void compute_seq(TYPE_SEQ seq_type, TYPE_CONTENT data_content);
	    void compute_seq_list(std::vector<TYPE_SEQ> stat_list, TYPE_CONTENT data_content);
		void compute_seq_all(TYPE_CONTENT data_content);

		// stat_kernel
		void compute_stat_kernel(TYPE_STAT_KERNEL seq_type, TYPE_CONTENT data_content);
		void compute_stat_kernel_list(std::vector<TYPE_STAT_KERNEL> stat_list, TYPE_CONTENT data_content);
		void compute_stat_kernel_all(TYPE_CONTENT data_content);

		// stat kernel - for interval
		void compute_stat_kernel(TYPE_STAT_KERNEL seq_type, TYPE_CONTENT data_content, Info& pre_info);
		void compute_stat_kernel_all(TYPE_CONTENT data_content, Info& pre_info);

		// data transfer

		// get data
		std::vector<DType> get_content_data(TYPE_CONTENT content_type, std::string layer_name);
		std::vector<DType> get_content_data(std::string content_type, std::string layer_name);

	// Print
	public:
		void print_total_info();
		void print_file_info();
		void print_conv_layer_info();
		void print_stat_info(TYPE_CONTENT data_content);
		void print_seq_info(TYPE_CONTENT data_content);

	// Interface
	public:

		Info& get() { return info; }

		// constructor
		Infos(std::string filename, std::map <std::string, std::vector<int>>& layer_tree_);
		Infos(Info &info_, std::map <std::string, std::vector<int>>& layer_tree_);


		// copy
		void copy_hyperparam(Infos &other, TYPE_CONTENT content_type, HyperParam hp);

		// init
		void init_stat();
		void init_type_name();
	
	private:
		
		// dump info
		Info info;

		std::map<TYPE_STAT, std::string> name_stat_type;
		std::map<TYPE_CONTENT, std::string> name_content_type;
		std::map<TYPE_SEQ, std::string> name_seq_type;
		std::map<TYPE_STAT_KERNEL, std::string> name_stat_kernel_type;
		std::map<std::string, std::vector<int>>& map_layers;

	};
}

}