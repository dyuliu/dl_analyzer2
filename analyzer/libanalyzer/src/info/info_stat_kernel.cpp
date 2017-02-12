
#include <info/info.h>
#include <emath/include/statistic.h>

namespace analyzer {

	unsigned int Infos::index(TYPE_STAT_KERNEL stat_type, TYPE_CONTENT data_content) {
		return ((int)data_content * (int)TYPE_STAT_KERNEL::END + (int)stat_type);
	}

	void Infos::compute_stat_kernel(TYPE_STAT_KERNEL stat_type, TYPE_CONTENT data_content) {

		if (stat_type == TYPE_STAT_KERNEL::END) return;

		const float *wt = NULL;
		const float *gt = NULL;

		int kernelSize = 0, kernelNum = 0, layerLength = 0, channelNum = 0, offset = 0;
		for (int i = 0; i < info.layers_size(); i++) {

			
			if (info.layers(i).type() == "batch_norm") continue;

			if (!info.layers(i).grad_size()) return;
			if (!info.layers(i).weight_size()) return;
			
			const int idx = index(stat_type, data_content);
			auto ptr = info.mutable_layers(i)->mutable_stat_kernel(idx);
			if (ptr->data().size() != 0) continue;

			wt = info.layers(i).weight().data();
			gt = info.layers(i).grad().data();

			kernelSize = info.layers(i).width() * info.layers(i).height();
			layerLength = info.layers(i).weight().size();
			channelNum = info.layers(i).channels();
			kernelNum = info.layers(i).num();
			offset = channelNum * kernelSize;

			//for (int cid = 0; cid < channelNum; cid++) {
			//	for (int k = cid * kernelSize; k < layerLength; k += offset) {
			//		std::vector<DType> wtt(wt + k, wt + k + kernelSize);
			//		std::vector<DType> gtt(gt + k, gt + k + kernelSize);
			//		DType v;

			//		if (stat_type == TYPE_STAT_KERNEL::CR_NORM_1) {
			//			auto wv = emath::norm(wtt, 1);
			//			auto gv = emath::norm(gtt, 1);
			//			v = gv / wv;
			//		}
			//		else if (stat_type == TYPE_STAT_KERNEL::CR_NORM_2) {
			//			auto wv = emath::norm(wtt, 2);
			//			auto gv = emath::norm(gtt, 2);
			//			v = gv / wv;
			//		}
			//		ptr->mutable_data()->Add(v);
			//	}
			//}

			for (int nid = 0; nid < kernelNum; nid++) {
				DType v;
				std::vector<DType> wtt(wt + nid * offset, wt + (nid + 1) * offset);
				std::vector<DType> gtt(gt + nid * offset, gt + (nid + 1) * offset);
				if (stat_type == TYPE_STAT_KERNEL::CR_NORM_1) {
					auto wv = emath::norm(wtt, 1);
					auto gv = emath::norm(gtt, 1);
					v = gv / wv;
				}
				else if (stat_type == TYPE_STAT_KERNEL::CR_NORM_2) {
					auto wv = emath::norm(wtt, 2);
					auto gv = emath::norm(gtt, 2);
					v = gv / wv;
				}
				else if (stat_type == TYPE_STAT_KERNEL::W_STD) {
					auto wv = emath::std(wtt);
					v = wv;
				}
				else if (stat_type == TYPE_STAT_KERNEL::G_NORM_1) {
					auto gv = emath::norm(gtt, 1);
					v = gv;
				}
				else if (stat_type == TYPE_STAT_KERNEL::G_NORM_2) {
					auto gv = emath::norm(gtt, 2);
					v = gv;
				}
				
				ptr->mutable_data()->Add(v);
			}

		}
	}

	void Infos::compute_stat_kernel_all(TYPE_CONTENT data_content) {
		for (unsigned int j = (int)TYPE_STAT_KERNEL::CR_NORM_1; j < (int)TYPE_STAT_KERNEL::END; j++) {
			__FUNC_TIME_CALL(compute_stat_kernel((TYPE_STAT_KERNEL)j, data_content), name_stat_kernel_type[(TYPE_STAT_KERNEL)j]);
			compute_stat_kernel((TYPE_STAT_KERNEL)j, data_content);
		}
	}

	void Infos::compute_stat_kernel_list(std::vector<TYPE_STAT_KERNEL> stat_list, TYPE_CONTENT data_content) {

#ifdef __DEBUG_INFO_OUTPUT
		COUT_METD << "func: compute_list_stat_kernel" << std::endl;
#endif

		for (unsigned int j = 0; j < stat_list.size(); j++) {
#ifdef __DEBUG_INFO_OUTPUT
			__FUNC_TIME_CALL(compute_stat_kernel(stat_list[j], data_content), name_stat_kernel_type[stat_list[j]]);
#else
			compute_stat_kernel((TYPE_STAT_KERNEL)stat_list[j], data_content);
#endif
		}
	}
}
