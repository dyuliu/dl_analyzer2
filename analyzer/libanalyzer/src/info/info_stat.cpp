
#include <info/info.h>
#include <emath/include/statistic.h>

namespace analyzer {

	unsigned int Infos::index(TYPE_STAT stat_type, TYPE_CONTENT data_content) {
		return ((int)data_content * (int)TYPE_STAT::END + (int)stat_type);
	}

	void Infos::compute_stat(TYPE_STAT stat_type, TYPE_CONTENT data_content) {

		if (stat_type == TYPE_STAT::END) return;

		//// compute leaf layer
		//for (int i = 0; i < info.layers_size(); i++) {

		//	if (info.layers(i).type() == "batch_norm") continue;

		//	if (data_content == TYPE_CONTENT::GRAD && !info.layers(i).grad_size()) return;
		//	if (data_content == TYPE_CONTENT::WEIGHT && !info.layers(i).weight_size()) return;

		//	// index of stat
		//	const int idx = index(stat_type, data_content);
		//	auto ptr = info.mutable_layers(i)->mutable_stat(idx);

		//	if (ptr->value() != 0.0) continue;

		//	if (stat_type == TYPE_STAT::MAX) {
		//		if (data_content == TYPE_CONTENT::GRAD)
		//			ptr->set_value(emath::max(ArrayToVector(info.layers(i).grad())));
		//		if (data_content == TYPE_CONTENT::WEIGHT)
		//			ptr->set_value(emath::max(ArrayToVector(info.layers(i).weight())));
		//	}

		//	if (stat_type == TYPE_STAT::MIN) {
		//		if (data_content == TYPE_CONTENT::GRAD)
		//			ptr->set_value(emath::min(ArrayToVector(info.layers(i).grad())));
		//		if (data_content == TYPE_CONTENT::WEIGHT)
		//			ptr->set_value(emath::min(ArrayToVector(info.layers(i).weight())));
		//	}

		//	if (stat_type == TYPE_STAT::MEAN) {
		//		if (data_content == TYPE_CONTENT::GRAD)
		//			ptr->set_value(emath::mean(ArrayToVector(info.layers(i).grad())));
		//		if (data_content == TYPE_CONTENT::WEIGHT)
		//			ptr->set_value(emath::mean(ArrayToVector(info.layers(i).weight())));
		//	}

		//	if (stat_type == TYPE_STAT::STD) {
		//		if (data_content == TYPE_CONTENT::GRAD)
		//			ptr->set_value(emath::std(ArrayToVector(info.layers(i).grad())));
		//		if (data_content == TYPE_CONTENT::WEIGHT)
		//			ptr->set_value(emath::std(ArrayToVector(info.layers(i).weight())));
		//	}

		//	if (stat_type == TYPE_STAT::SUM) {
		//		if (data_content == TYPE_CONTENT::GRAD)
		//			ptr->set_value(emath::sum(ArrayToVector(info.layers(i).grad())));
		//		if (data_content == TYPE_CONTENT::WEIGHT)
		//			ptr->set_value(emath::sum(ArrayToVector(info.layers(i).weight())));
		//	}

		//	if (stat_type == TYPE_STAT::VAR) {
		//		if (data_content == TYPE_CONTENT::GRAD)
		//			ptr->set_value(emath::var(ArrayToVector(info.layers(i).grad())));
		//		if (data_content == TYPE_CONTENT::WEIGHT)
		//			ptr->set_value(emath::var(ArrayToVector(info.layers(i).weight())));
		//	}

		//	/*
		//	if (stat_type == TYPE_STAT::NORM_0) {
		//		if (data_content == TYPE_CONTENT::GRAD)
		//			ptr->set_value(emath::norm(ArrayToVector(info.layers(i).grad()), 0));
		//		if (data_content == TYPE_CONTENT::WEIGHT)
		//			ptr->set_value(emath::norm(ArrayToVector(info.layers(i).weight()), 0));
		//	}
		//	*/

		//	if (stat_type == TYPE_STAT::NORM_1) {
		//		if (data_content == TYPE_CONTENT::GRAD)
		//			ptr->set_value(emath::norm(ArrayToVector(info.layers(i).grad()), 1));
		//		if (data_content == TYPE_CONTENT::WEIGHT)
		//			ptr->set_value(emath::norm(ArrayToVector(info.layers(i).weight()), 1));
		//	}

		//	if (stat_type == TYPE_STAT::NORM_2) {
		//		if (data_content == TYPE_CONTENT::GRAD)
		//			ptr->set_value(emath::norm(ArrayToVector(info.layers(i).grad()), 2));
		//		if (data_content == TYPE_CONTENT::WEIGHT)
		//			ptr->set_value(emath::norm(ArrayToVector(info.layers(i).weight()), 2));
		//	}

		//	if (stat_type == TYPE_STAT::QUANTILE_1_2) {
		//		if (data_content == TYPE_CONTENT::GRAD)
		//			ptr->set_value(emath::quantile(ArrayToVector(info.layers(i).grad()), 0.5));
		//		if (data_content == TYPE_CONTENT::WEIGHT)
		//			ptr->set_value(emath::quantile(ArrayToVector(info.layers(i).weight()), 0.5));
		//	}

		//	if (stat_type == TYPE_STAT::QUANTILE_1_4) {
		//		if (data_content == TYPE_CONTENT::GRAD)
		//			ptr->set_value(emath::quantile(ArrayToVector(info.layers(i).grad()), 0.25));
		//		if (data_content == TYPE_CONTENT::WEIGHT)
		//			ptr->set_value(emath::quantile(ArrayToVector(info.layers(i).weight()), 0.25));
		//	}

		//	if (stat_type == TYPE_STAT::QUANTILE_3_4) {
		//		if (data_content == TYPE_CONTENT::GRAD)
		//			ptr->set_value(emath::quantile(ArrayToVector(info.layers(i).grad()), 0.75));
		//		if (data_content == TYPE_CONTENT::WEIGHT)
		//			ptr->set_value(emath::quantile(ArrayToVector(info.layers(i).weight()), 0.75));
		//	}
		//}

		// compute high level layers
		for (int i = 0; i < info.h_layers_size(); i++) {

			auto it = this->map_layers.find(info.h_layers(i).name());
			if (it == this->map_layers.end()) continue;
			std::vector<DType> grad, weight;
			
			for (auto const& lid : it->second) {
				if (data_content == TYPE_CONTENT::GRAD && !info.layers(lid).grad_size()) return;
				if (data_content == TYPE_CONTENT::WEIGHT && !info.layers(lid).weight_size()) return;
				grad.insert(grad.end(), info.layers(lid).grad().data(), info.layers(lid).grad().data() + info.layers(lid).grad().size());
				weight.insert(weight.end(), info.layers(lid).weight().data(), info.layers(lid).weight().data() + info.layers(lid).weight().size());
			}


			// index of stat
			const int idx = index(stat_type, data_content);
			auto ptr = info.mutable_h_layers(i)->mutable_stat(idx);

			if (ptr->value() != 0.0) continue;

			if (stat_type == TYPE_STAT::MAX) {
				if (data_content == TYPE_CONTENT::GRAD)
					ptr->set_value(emath::max(grad));
				if (data_content == TYPE_CONTENT::WEIGHT)
					ptr->set_value(emath::max(weight));
			}

			if (stat_type == TYPE_STAT::MIN) {
				if (data_content == TYPE_CONTENT::GRAD)
					ptr->set_value(emath::min(grad));
				if (data_content == TYPE_CONTENT::WEIGHT)
					ptr->set_value(emath::min(weight));
			}

			if (stat_type == TYPE_STAT::MEAN) {
				if (data_content == TYPE_CONTENT::GRAD)
					ptr->set_value(emath::mean(grad));
				if (data_content == TYPE_CONTENT::WEIGHT)
					ptr->set_value(emath::mean(weight));
			}

			if (stat_type == TYPE_STAT::STD) {
				if (data_content == TYPE_CONTENT::GRAD)
					ptr->set_value(emath::std(grad));
				if (data_content == TYPE_CONTENT::WEIGHT)
					ptr->set_value(emath::std(weight));
			}

			if (stat_type == TYPE_STAT::SUM) {
				if (data_content == TYPE_CONTENT::GRAD)
					ptr->set_value(emath::sum(grad));
				if (data_content == TYPE_CONTENT::WEIGHT)
					ptr->set_value(emath::sum(weight));
			}

			if (stat_type == TYPE_STAT::VAR) {
				if (data_content == TYPE_CONTENT::GRAD)
					ptr->set_value(emath::var(grad));
				if (data_content == TYPE_CONTENT::WEIGHT)
					ptr->set_value(emath::var(weight));
			}

			if (stat_type == TYPE_STAT::NORM_1) {
				if (data_content == TYPE_CONTENT::GRAD)
					ptr->set_value(emath::norm(grad, 1));
				if (data_content == TYPE_CONTENT::WEIGHT)
					ptr->set_value(emath::norm(weight, 1));
			}

			if (stat_type == TYPE_STAT::NORM_2) {
				if (data_content == TYPE_CONTENT::GRAD)
					ptr->set_value(emath::norm(grad, 2));
				if (data_content == TYPE_CONTENT::WEIGHT)
					ptr->set_value(emath::norm(weight, 2));
			}

			if (stat_type == TYPE_STAT::QUANTILE_1_2) {
				if (data_content == TYPE_CONTENT::GRAD)
					ptr->set_value(emath::quantile(grad, 0.5));
				if (data_content == TYPE_CONTENT::WEIGHT)
					ptr->set_value(emath::quantile(weight, 0.5));
			}

			if (stat_type == TYPE_STAT::QUANTILE_1_4) {
				if (data_content == TYPE_CONTENT::GRAD)
					ptr->set_value(emath::quantile(grad, 0.25));
				if (data_content == TYPE_CONTENT::WEIGHT)
					ptr->set_value(emath::quantile(weight, 0.25));
			}

			if (stat_type == TYPE_STAT::QUANTILE_3_4) {
				if (data_content == TYPE_CONTENT::GRAD)
					ptr->set_value(emath::quantile(grad, 0.75));
				if (data_content == TYPE_CONTENT::WEIGHT)
					ptr->set_value(emath::quantile(weight, 0.75));
			}
		}
	}

	void Infos::compute_stat_all(TYPE_CONTENT data_content) {
		for (unsigned int j = (int)TYPE_STAT::MAX; j < (int)TYPE_STAT::END; j++) {
			compute_stat((TYPE_STAT)j, data_content);
		}
	}

	void Infos::compute_stat_list(std::vector<TYPE_STAT> stat_list, TYPE_CONTENT data_content) {
		for (unsigned int j = 0; j < stat_list.size(); j++) {
			compute_stat((TYPE_STAT)stat_list[j], data_content);
		}
	}
}
