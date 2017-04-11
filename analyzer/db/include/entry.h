#pragma once
#undef UNICODE
#define UNICODE
#undef _WINSOCKAPI_
#define _WINSOCKAPI_

#include <winsock2.h>

#include <vector>
#include <mongo/client/dbclient.h>
#include <libanalyzer/include/proto/analyzer_proto.pb.h>
#include <libanalyzer/include/info/info.h>

using mongo::BSONArrayBuilder;
using mongo::BSONArray;
using mongo::BSONElement;
using mongo::BSONObj;
using mongo::BSONObjBuilder;
using mongo::BulkOperationBuilder;
using mongo::DBClientCursor;
using mongo::fromjson;

namespace analyzer_tools {
	namespace db {

		using analyzer_proto::Info;
		using analyzer_proto::Images;
		using std::string;
		using std::vector;
		using std::cout;
		using std::endl;

		struct Img {
			string file;
			string cls;
			int label;
			vector<int> iter;
			vector<int> answer;
			vector<int> correct;
		};

		struct Cls {
			string name;
			int size;
			vector<string> file;
		};

		struct ClsStat {
			string cls;
			int iter;
			double testError;
			vector<int> abLeft;
			vector<int> abRight;
		};

		class DB {

			// public API
		public:

			/**
			Construction Function
			@param serverAddress e.g., "192.168.1.10",
			the default value is "localhost".
			*/
			DB(std::string database = "cnnvis", std::string dbName = "DeepLearning", std::string serverAddress = "localhost:27017");

			/**
			Deconstruction Function
			*/
			~DB();

			/**
			Bind data to private members
			*/
			void bindInfo(Info *d);

			void bindImgInfo(Images *d);

			// void bindRecorder(Recorder *d);

			/**
			Import a selected stat into DB
			*/
			void importStat(analyzer::Infos::TYPE_STAT statName, analyzer::Infos::TYPE_CONTENT contentName, std::string colName = "");

			/**
			Import all stat information to DB
			*/
			void importAllStats();

			/**
			Import a selected seq into DB
			*/
			void importSeq(analyzer::Infos::TYPE_SEQ seqName, analyzer::Infos::TYPE_CONTENT contentName, std::string colName = "");

			/**
			Import all seq information to DB
			*/
			void importAllSeqs();

			/**
			Import all kernel ralted stats
			*/
			void importStatKernel(analyzer::Infos::TYPE_STAT_KERNEL statName, analyzer::Infos::TYPE_CONTENT contentName, std::string type = "stat");

			void importAllStatsKernel();

			void importAllStatsKernelIV();

			/**
			Import layer attrs
			*/
			void importLayerAttrs(std::string colName = "LayerInfo");

			/*8
			Import img info including class_name & data (file) name
			*/
			void importImgInfo(std::string colName = "ImgTrainInfo");

			void importTestImgInfo(std::map<std::string, int> &map_label, int batchsize = 1000, std::string colName = "ImgTestInfo");
			void importTestImgData(bool first, std::string colName = "ImgTestData");
			void importTestImgStat(bool first, std::string colName = "ImgTestStat");

			/**
			Import all stats and layer attrs
			*/
			void importAll();

			/**
			Import fine-grained gradient data, optional
			*/
			void importGradient(std::string colName = "GradRaw");

			/**
			Import fine-grained weight data, optional
			*/
			void importWeight(std::string colName = "WeightRaw");

			/**
			Import raw gradient and weight data
			*/
			void importRaw();

			/**
			Import recorder info, e.g., error rate.
			*/
			void importRecorderInfo(int iteration, string type, float value);


			/**
			Create Indexes for stat, statseq, and recorder collections
			*/
			void createIndexes();

			void processImgData();

			void fetchTestIterSet(std::vector<int>& v);

			void deleteDB();

			// private data
		private:
			mongo::DBClientConnection connection;
			std::string dbName;
			std::string database;
			Info *iData;
			Images *imgData;
			std::map<string, Img> imgs;
			std::map<string, Cls> clsInfo;
			std::map<string, BSONObjBuilder> imgsBson;
		};

	}

}

