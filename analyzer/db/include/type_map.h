#pragma once

#include <libanalyzer/include/info/info.h>
#define MAP_TYPE_STATSEQ std::map<analyzer::Infos::TYPE_SEQ, std::string>
#define STATSEQ_INDEX(x, y) (int)y * (int)analyzer::Infos::TYPE_SEQ::END + (int)x

#define MAP_TYPE_STAT std::map<analyzer::Infos::TYPE_STAT, std::string>
#define STAT_INDEX(x, y) (int)y * (int)analyzer::Infos::TYPE_STAT::END + (int)x

#define MAP_TYPE_STAT_KERNEL std::map<analyzer::Infos::TYPE_STAT_KERNEL, std::string>
#define STAT_KERNEL_INDEX(x, y) (int)y * (int)analyzer::Infos::TYPE_STAT_KERNEL::END + (int)x

#define MAP_TYPE_DIST std::map<analyzer::Infos::TYPE_DISTANCE, std::string>
#define DIST_INDEX(x, y) (int)y * (int)analyzer::Infos::TYPE_DISTANCE::END + (int)x

#define MAP_TYPE_CLUSTER std::map<analyzer::Infos::TYPE_CLUSTER, std::string>
#define CLUSTER_INDEX(x, y) (int)y * (int)analyzer::Infos::TYPE_CLUSTER::END + (int)x

#define MAP_TYPE_CONTENT std::map<analyzer::Infos::TYPE_CONTENT, std::string>

#define MAP_LAYER std::map<std::string, std::vector<int>>

#include <libanalyzer/include/recorder/recorder.h>
#define MAP_TYPE_RECORD std::map<std::string, std::string>

namespace db {

	using TYPE_SEQ = analyzer::Infos::TYPE_SEQ;
	MAP_TYPE_STATSEQ mapTypeSeq = {
		// { TYPE_SEQ::HISTOGRAM, "SeqHistogram" },
		{ TYPE_SEQ::CHANGERATIO, "SeqChangeRatio" }
	};

	using TYPE_STAT = analyzer::Infos::TYPE_STAT;
	MAP_TYPE_STAT mapTypeStat = {
		{ TYPE_STAT::MAX, "StatMax" },
		{ TYPE_STAT::MIN, "StatMin" },
		{ TYPE_STAT::MEAN, "StatMean" },
		{ TYPE_STAT::SUM, "StatSum" },
		{ TYPE_STAT::VAR, "StatVar" },
		{ TYPE_STAT::STD, "StatStd" },
		// { TYPE_STAT::NORM_0, "StatNorm0" },
		{ TYPE_STAT::NORM_1, "StatNorm1" },
		{ TYPE_STAT::NORM_2, "StatNorm2" },
		{ TYPE_STAT::QUANTILE_1_2, "StatMid" },
		{ TYPE_STAT::QUANTILE_1_4, "StatQuarter1" },
		{ TYPE_STAT::QUANTILE_3_4, "StatQuarter3" }
	};

	using TYPE_STAT_KERNEL = analyzer::Infos::TYPE_STAT_KERNEL;
	MAP_TYPE_STAT_KERNEL mapTypeStatKernel = {
		{ TYPE_STAT_KERNEL::CR_NORM_1, "KernelCRNorm1" },
		{ TYPE_STAT_KERNEL::CR_NORM_2, "KernelCRNorm2" },
		{ TYPE_STAT_KERNEL::W_STD, "KernelWeightStd" },
		{ TYPE_STAT_KERNEL::G_NORM_1, "KernelGradNorm1" },
		{ TYPE_STAT_KERNEL::G_NORM_2, "KernelGradNorm2" }
	};

	MAP_TYPE_STAT_KERNEL mapTypeIvKernel = {
		{ TYPE_STAT_KERNEL::I_EUCLIDEAN, "KernelIvEuclidean" },
		{ TYPE_STAT_KERNEL::I_MANHATTAN, "KernelIvManhattan" },
		{ TYPE_STAT_KERNEL::I_COSINE, "KernelIvCosine" },
		{ TYPE_STAT_KERNEL::I_CR_NORM_1, "KernelIvCRNorm1" },
		{ TYPE_STAT_KERNEL::I_CR_NORM_2, "KernelIvCRNorm2" }
	};

	using TYPE_DISTANCE = analyzer::Infos::TYPE_DISTANCE;
	MAP_TYPE_DIST mapTypeDist = {
		{ TYPE_DISTANCE::CORRELATION, "DistCorrelation" },
		{ TYPE_DISTANCE::COSINE, "DistCosine" },
		{ TYPE_DISTANCE::EUCLIDEAN, "DistEuclidean" },
		{ TYPE_DISTANCE::MANHATTAN, "DistManhattan" },
		{ TYPE_DISTANCE::COSINE_NORM, "DistCosineNorm" },
		{ TYPE_DISTANCE::EUCLIDEAN_NORM, "DistEuclideanNorm" }
	};

	using TYPE_CLUSTER = analyzer::Infos::TYPE_CLUSTER;
	MAP_TYPE_CLUSTER mapTypeCluster = {
		{ TYPE_CLUSTER::KMEANS, "ClusterKmeans" }
	};

	using TYPE_CONTENT = analyzer::Infos::TYPE_CONTENT;
	MAP_TYPE_CONTENT mapTypeContent = {
		{ TYPE_CONTENT::GRAD, "Grad" },
		{ TYPE_CONTENT::WEIGHT, "Weight" }
	};

	MAP_TYPE_RECORD mapTypeRecord = {
		{ "train_error", "RecTrainError" },
		{ "train_loss", "RecTrainLoss" },
		{ "test_error", "RecTestError" },
		{ "test_loss", "RecTestLoss" },
		{ "forward_time", "RecForwardTime" },
		{ "backward_time", "RecBackwardTime" },
		{ "update_time", "RecUpdateTime" },
		{ "sum_time", "RecSumTime" },
		{ "learning_rate", "RecLearningRate" }
	};

	MAP_LAYER mapLayer = {
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
}
