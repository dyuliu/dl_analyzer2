
#include <info/info.h>
#include <iomanip>

namespace analyzer_tools {

	namespace analyzer{

		void Infos::print_total_info() {

			print_file_info();
			print_conv_layer_info();

			print_stat_info(TYPE_CONTENT::GRAD);
			print_stat_info(TYPE_CONTENT::WEIGHT);

			print_seq_info(TYPE_CONTENT::GRAD);
			print_seq_info(TYPE_CONTENT::WEIGHT);

		}

		void Infos::print_file_info() {

			std::cout << std::endl;
			COUT_METD << "----- PRINT FILE INFO -----" << std::endl;

			// print basic information
			if (info.has_filename())
				COUT_CHEK << "Running file: " << info.filename();
			if (info.has_iteration())
				std::cout << ", iteration: " << info.iteration();
			if (info.has_worker_id())
				std::cout << ", worker id: " << info.worker_id();
			if (info.has_images()) {
				std::cout << ", image_number: " << info.images().images_size() << std::endl;
			}

			std::cout << std::endl;
		}

		void Infos::print_conv_layer_info() {

			std::cout << std::endl;
			COUT_METD << "----- PRINT LAYER INFO (NO BN) -----" << std::endl;

			// print layer infomation
			size_t weight_size = 0, grad_size = 0;
			for (int i = 0; i < info.layers_size(); i++) {

				weight_size += info.layers(i).weight_size();
				grad_size += info.layers(i).grad_size();

				if (info.layers(i).type() == "batch_norm") continue;

				COUT_READ << std::setw(3) << i << ", " << info.layers(i).name() << ", " << info.layers(i).type() << ", ("
					<< info.layers(i).num() << " " << info.layers(i).channels() << " "
					<< info.layers(i).height() << " " << info.layers(i).width() << "), "
					<< "weight size: " << info.layers(i).weight_size() << ", "
					<< "grad size: " << info.layers(i).grad_size() << std::endl;
			}
			COUT_CHEK << "Total weight size: " << weight_size << ", Total grad size: " << grad_size << std::endl;
		}

		void Infos::print_stat_info(TYPE_CONTENT data_content) {

			std::cout << std::endl;
			if (data_content == TYPE_CONTENT::GRAD)
				COUT_METD << "----- PRINT GRAD STAT INFO -----" << std::endl;

			if (data_content == TYPE_CONTENT::WEIGHT)
				COUT_METD << "----- PRINT WEIGHT STAT INFO -----" << std::endl;

			// print stat infomation
			size_t weight_size = 0, grad_size = 0;
			for (int i = 0; i < info.layers_size(); i++) {

				weight_size += info.layers(i).weight_size();
				grad_size += info.layers(i).grad_size();

				if (info.layers(i).type() == "batch_norm") continue;

				COUT_CHEK << std::setw(3) << i << ", " << std::setw(30) << info.layers(i).name();
				for (int j = (int)data_content*(int)TYPE_STAT::END; j < ((int)data_content + 1)*(int)TYPE_STAT::END; j++) {
					if (info.layers(i).stat(j).content() == name_content_type[data_content]) {
						std::cout << ", " << info.layers(i).stat(j).type() << ": " << info.layers(i).stat(j).value();
					}
				}
				std::cout << std::endl;
			}
		}

		void Infos::print_seq_info(TYPE_CONTENT data_content) {

			std::cout << std::endl;
			if (data_content == TYPE_CONTENT::GRAD)
				COUT_METD << "----- PRINT GRAD SEQUENCE INFO -----" << std::endl;

			if (data_content == TYPE_CONTENT::WEIGHT)
				COUT_METD << "----- PRINT WEIGHT SEQUENCE INFO -----" << std::endl;

			// print sequence infomation
			size_t weight_size = 0, grad_size = 0;
			for (int i = 0; i < info.layers_size(); i++) {

				weight_size += info.layers(i).weight_size();
				grad_size += info.layers(i).grad_size();

				if (info.layers(i).type() == "batch_norm") continue;

				COUT_CHEK << std::setw(3) << i << ", " << std::setw(30) << info.layers(i).name() << std::endl;
				for (int j = (int)data_content*(int)TYPE_SEQ::END; j < ((int)data_content + 1)*(int)TYPE_SEQ::END; j++) {
					if (info.layers(i).seq(j).content() == name_content_type[data_content]) {
						if (info.layers(i).seq(j).type() == "changeratio") {
							std::cout << info.layers(i).seq(j).type() << ": ";
							std::cout << info.layers(i).seq(j).data().data()[0] << "  ";
							std::cout << info.layers(i).seq(j).data().data()[1] << "  ";
							std::cout << info.layers(i).seq(j).data().data()[2] << std::endl;
						}
					}
				}
			}
		}

		

	}

}