#include "type_map.h"
#include "entry.h"
#include <utils/color_print.h>

#define CHECK_HAVE_WEIGHT {if (this->iData->worker_id() != 0) continue;}



namespace analyzer_tools {

	namespace db {

		DB::DB(std::string database, std::string dbName, std::string serverAddress) {
			mongo::client::initialize();
			try {
				connection.connect(serverAddress);
				std::cout << "mongoDB connect successfully" << std::endl;
				this->database = database;
				this->dbName = dbName;
			}
			catch (const mongo::DBException &e) {
				std::cout << "caught " << e.what() << std::endl;
			}
		}

		DB::~DB() {
			mongo::client::shutdown();
		}

		void DB::bindInfo(Info *data) {
			this->iData = data;
		}

		void DB::bindImgInfo(Images *data) {
			this->imgData = data;
		};

		void DB::importGradient(std::string colName) {
			// std::cout << "Importing gradient data to collection \"" << colName << "\"." << std::endl;
			std::string col = this->database + "." + this->dbName + "_" + colName;
			Info *data = this->iData;
			const float *dt = NULL;
			int kernelSize = 0, kernelNum = 0, layerLength = 0, channelNum = 0, offset = 0;


			for (int i = 0; i < data->layers_size(); i++) {
				if (data->layers(i).type() == "batch_norm") continue;
				dt = data->layers(i).grad().data();
				kernelSize = data->layers(i).width() * data->layers(i).height();
				layerLength = data->layers(i).grad().size();
				channelNum = data->layers(i).channels();
				kernelNum = data->layers(i).num();
				offset = channelNum * kernelSize;

				BulkOperationBuilder bulk = this->connection.initializeUnorderedBulkOp(col);

				// save mongo document by cid
				for (int cid = 0; cid < channelNum; cid++) {
					// fetch kernels with cid j
					BSONObjBuilder bObj;
					bObj
						.append("iter", data->iteration())
						//.append("wid", data->worker_id())
						.append("lid", i)
						.append("cid", cid);
					BSONObjBuilder valueObj;
					int kcount = 0;
					for (int k = cid * kernelSize; k < layerLength; k += offset) {
						std::vector<DType> kn(dt + k, dt + k + kernelSize);
						BSONArrayBuilder floatArrValue;
						for (auto &d : kn) { floatArrValue.append(d); }
						valueObj.append(std::to_string(kcount * channelNum + cid), floatArrValue.arr());
						kcount++;
					}
					bObj.append("value", valueObj.obj());
					bulk.insert(bObj.obj());
				}

				mongo::WriteResult rs;
				bulk.execute(0, &rs);
			}
		}

		void DB::importWeight(std::string colName) {
			// std::cout << "Importing weight data to collection \"" << colName << "\"." << std::endl;
			std::string col = this->database + "." + this->dbName + "_" + colName;
			Info *data = this->iData;
			const float *dt = NULL;
			int kernelSize = 0, kernelNum = 0, layerLength = 0, channelNum = 0, offset = 0;


			for (int i = 0; i < data->layers_size(); i++) {
				if (data->layers(i).type() == "batch_norm") continue;
				dt = data->layers(i).weight().data();
				kernelSize = data->layers(i).width() * data->layers(i).height();
				layerLength = data->layers(i).weight().size();
				channelNum = data->layers(i).channels();
				kernelNum = data->layers(i).num();
				offset = channelNum * kernelSize;

				BulkOperationBuilder bulk = this->connection.initializeUnorderedBulkOp(col);

				// save mongo document by cid
				for (int cid = 0; cid < channelNum; cid++) {
					// fetch kernels with cid j
					BSONObjBuilder bObj;
					bObj
						.append("iter", data->iteration())
						//.append("wid", data->worker_id())
						.append("lid", i)
						.append("cid", cid);
					BSONObjBuilder valueObj;
					int kcount = 0;
					for (int k = cid * kernelSize; k < layerLength; k += offset) {
						std::vector<DType> kn(dt + k, dt + k + kernelSize);
						BSONArrayBuilder floatArrValue;
						for (auto &d : kn) { floatArrValue.append(d); }
						valueObj.append(std::to_string(kcount * channelNum + cid), floatArrValue.arr());
						kcount++;
					}
					bObj.append("value", valueObj.obj());
					bulk.insert(bObj.obj());
				}

				mongo::WriteResult rs;
				bulk.execute(0, &rs);
			}
		}

		void DB::importRaw() {
			this->importGradient();
			if (this->iData->worker_id() == 0)
				this->importWeight();
		}

		void DB::importStat(TYPE_STAT statName, TYPE_CONTENT contentName, std::string colName) {
			MAP_TYPE_STAT::iterator iterStat;
			iterStat = mapTypeStat.find(statName);
			if (iterStat == mapTypeStat.end()) {
				std::cout << "Wrong TYPE_STAT" << std::endl;
				return;
			}
			MAP_TYPE_CONTENT::iterator iterContent;
			iterContent = mapTypeContent.find(contentName);
			if (iterContent == mapTypeContent.end()) {
				std::cout << "Wrong TYPE_CONTENT" << std::endl;
				return;
			}
			if (colName == "") {
				colName = iterContent->second + iterStat->second;
			}
			//std::cout << "Importing data to \""<< colName << "\"." << std::endl;
			std::string col = this->database + "." + this->dbName + "_" + colName;
			Info *data = this->iData;
			BSONObjBuilder bObj;
			bObj.append("iter", data->iteration());
			BSONObjBuilder valueObj;
			for (int i = 0; i < data->layers_size(); i++) {
				if (data->layers(i).type() == "batch_norm") continue;
				valueObj.append(std::to_string(i), data->layers(i).stat(STAT_INDEX(statName, contentName)).value());
			}
			bObj.append("value", valueObj.obj());
			this->connection.insert(col, bObj.obj());

			col = this->database + "." + this->dbName + "_" + "HL" + colName;
			BSONObjBuilder bObj2;
			bObj2.append("iter", data->iteration());
			BSONObjBuilder valueObj2;
			for (int i = 0; i < data->h_layers_size(); i++) {
				valueObj2.append(data->h_layers(i).name(), data->h_layers(i).stat(STAT_INDEX(statName, contentName)).value());
			}
			bObj2.append("value", valueObj2.obj());
			this->connection.insert(col, bObj2.obj());

		}

		void DB::importSeq(TYPE_SEQ seqName, TYPE_CONTENT contentName, std::string colName) {
			MAP_TYPE_STATSEQ::iterator iterStatSeq;
			iterStatSeq = mapTypeSeq.find(seqName);
			if (iterStatSeq == mapTypeSeq.end()) {
				std::cout << "Wrong TYPE_STAT" << std::endl;
				return;
			}
			MAP_TYPE_CONTENT::iterator iterContent;
			iterContent = mapTypeContent.find(contentName);
			if (iterContent == mapTypeContent.end()) {
				std::cout << "Wrong TYPE_CONTENT" << std::endl;
				return;
			}
			if (colName == "") {
				colName = iterContent->second + iterStatSeq->second;
			}

			//std::cout << "Importing data to \""<< colName << "\"." << std::endl;
			std::string col = this->database + "." + this->dbName + "_" + colName;
			Info *data = this->iData;
			BSONObjBuilder bObj;
			bObj.append("iter", data->iteration());
			BSONObjBuilder valueObj;
			for (int i = 0; i < data->layers_size(); i++) {

				if (data->layers(i).type() == "batch_norm") continue;
				BSONArrayBuilder floatArrValue;
				for (auto v : data->layers(i).seq(STATSEQ_INDEX(seqName, contentName)).data()) {
					floatArrValue.append(v);
				}
				valueObj.append(std::to_string(i), floatArrValue.arr());
			}
			bObj.append("value", valueObj.obj());
			this->connection.insert(col, bObj.obj());

			col = this->database + "." + this->dbName + "_" + "HL" + colName;
			BSONObjBuilder bObj2;
			bObj2.append("iter", data->iteration());
			BSONObjBuilder valueObj2;
			for (int i = 0; i < data->h_layers_size(); i++) {
				BSONArrayBuilder floatArrValue;
				for (auto v : data->h_layers(i).seq(STATSEQ_INDEX(seqName, contentName)).data()) {
					floatArrValue.append(v);
				}
				valueObj2.append(data->h_layers(i).name(), floatArrValue.arr());
			}
			bObj2.append("value", valueObj2.obj());
			this->connection.insert(col, bObj2.obj());
		}

		void DB::importAllStats() {
			for (auto it = mapTypeStat.begin(); it != mapTypeStat.end(); ++it) {
				this->importStat(it->first, TYPE_CONTENT::GRAD);
				CHECK_HAVE_WEIGHT
					this->importStat(it->first, TYPE_CONTENT::WEIGHT);
			}
		}

		void DB::importAllSeqs() {
			for (auto it = mapTypeSeq.begin(); it != mapTypeSeq.end(); ++it) {
				if (it->first != TYPE_SEQ::CHANGERATIO)
					this->importSeq(it->first, TYPE_CONTENT::GRAD);
				CHECK_HAVE_WEIGHT
					this->importSeq(it->first, TYPE_CONTENT::WEIGHT);
			}
		}

		void DB::importImgInfo(std::string colName) {

			std::string col = this->database + "." + this->dbName + "_" + colName;
			Info *data = this->iData;
			Images imgs = data->images();

			BSONObjBuilder bObj;
			bObj.append("iter", data->iteration());
			//.append("wid", data->worker_id());

			BSONArrayBuilder cls;
			BSONArrayBuilder file;
			BSONArrayBuilder label;

			for (int i = 0; i < imgs.images_size(); i++) {
				cls.append(imgs.images(i).class_name());
				file.append(imgs.images(i).file_name());
				label.append(imgs.images(i).label_id());
			}
			bObj.append("cls", cls.arr());
			bObj.append("file", file.arr());
			bObj.append("label", label.arr());

			BSONObj o = bObj.obj();
			this->connection.insert(col, o);
		}

		void DB::importTestImgStat(bool first, std::string colName) {
			// to do
		};


		void DB::importTestImgData(bool first, std::string colName) {
			std::string col = this->database + "." + this->dbName + "_" + colName;
			Images *data = this->imgData;
			if (first) {

				//COUT_G("initializing") << std::endl;
				for (int i = 0; i < data->images_size(); i++) {
					auto file = data->images(i).file_name();
					auto cls = data->images(i).class_name();
					auto label = data->images(i).label_id();
					vector<int> iter;
					vector<int> answer;
					vector<int> correct;

					struct Img tmp = {
						file,
						cls,
						label,
						iter,
						answer,
						correct
					};

					this->imgs[file] = tmp;
					
					if (this->clsInfo.find(cls) == clsInfo.end()) {
						this->clsInfo.insert(std::pair<string, Cls>(cls, { cls, 0, {} }));
					}
					this->clsInfo[cls].size++;
					this->clsInfo[cls].file.push_back(file);
				}

				// write clsInfo to db
				std::string clsInfoCol = this->database + "." + this->dbName + "_ClsInfo";
				for (auto it = this->clsInfo.begin(); it != this->clsInfo.end(); ++it) {
					BSONObjBuilder bObj;
					auto info = it->second;
					bObj.append("name", info.name)
						.append("size", info.size);
					BSONArrayBuilder arr;
					for (auto const& d : info.file) { arr.append(d); }
					bObj.append("file", arr.arr());
					this->connection.insert(clsInfoCol, bObj.obj());
				}

			}

			std::map<string, ClsStat> clsStat;
			std::string clsStatCol = this->database + "." + this->dbName + "_ImgTestClsStat";
			for (int i = 0; i < data->images_size(); i++) {
				auto file = data->images(i).file_name();
				auto cls = data->images(i).class_name();
				auto label = data->images(i).label_id();
				auto answer = data->images(i).answer();
				int err = 0;
				if (label != answer) err = 1;
				this->imgs[file].iter.push_back(data->iteration());
				this->imgs[file].answer.push_back(answer);
				this->imgs[file].correct.push_back(1 - err);


				// write ImgTestClsStat
				if (clsStat.find(cls) == clsStat.end()) {
					clsStat.insert(std::pair<string, ClsStat>(cls, { cls, data->iteration(), 0, {}, {} }));
				}
				clsStat[cls].testError += err;
			}
			// calc outvalue
			int step = 100;
			for (auto it = this->clsInfo.begin(); it != this->clsInfo.end(); ++it) {
				auto info = it->second;
				clsStat[info.name].testError /= info.size;
				// calc abLeft
				vector<int> abLeft(step, 0);
				for (auto &filename : info.file) {
					auto img = this->imgs[filename];
					int ci = img.correct.size() - 1;
					if (ci - 1 >= 0 && img.correct[ci - 1] != img.correct[ci]) {
						int k = ci - 1;
						int left = std::max(0, ci - step);
						while (k >= left) {
							if (img.correct[k] == img.correct[ci]) break;
							abLeft[ci - k - 1] += 1;
							k--;
						}
					}
				}

				BSONObjBuilder bObj;
				bObj.append("cls", info.name)
					.append("iter", clsStat[info.name].iter)
					.append("testError", clsStat[info.name].testError);
				BSONArrayBuilder arr;
				for (auto const& d : abLeft) { arr.append(d); }
				bObj.append("abLeft", arr.arr());

				// add abLeft
				this->connection.insert(clsStatCol, bObj.obj());
			}




			// write ImgTestData
			BulkOperationBuilder bulk = this->connection.initializeUnorderedBulkOp(col);

			for (auto iter = imgs.begin(); iter != imgs.end(); iter++) {
				BSONObjBuilder bObj;
				auto o = iter->second;
				bObj.append("file", o.file)
					.append("cls", o.cls)
					.append("label", o.label);

				BSONArrayBuilder arrIter;
				for (auto const& d : o.iter) { arrIter.append(d); }
				BSONArrayBuilder arrAnswer;
				for (auto const& d : o.answer) { arrAnswer.append(d); }

				bObj.append("iter", arrIter.arr())
					.append("answer", arrAnswer.arr());
				if (first) {
					bulk.insert(bObj.obj());
				} else {
					bulk.find((BSONObjBuilder() << "file" << o.file).obj()).replaceOne(bObj.obj());
				}
			}
			mongo::WriteResult rs;
			bulk.execute(0, &rs);

		};


		void DB::importTestImgInfo(std::map<std::string, int> &map_label, int batchsize, std::string colName) {
			std::string col = this->database + "." + this->dbName + "_" + colName;
			Images *data = this->imgData;
			for (int bs = 0, i = 0; i < data->images_size(); i += batchsize) {
				BSONObjBuilder bObj;
				bObj.append("iter", data->iteration())
					.append("batch", bs);
				BSONArrayBuilder cls;
				BSONArrayBuilder file;
				BSONArrayBuilder label;
				BSONArrayBuilder answer;
				BSONArrayBuilder event;  // 4 dimension
				int eventArr[4] = { 0, 0, 0, 0 };  // n to y, y to n, y to y, n to n  

				// BSONArrayBuilder prob;

				for (int j = i; j < i + batchsize; j++) {
					if (j >= data->images_size()) { break; }
					int correct_label = data->images(j).label_id();
					int cur_label = data->images(j).answer();
					int pre_label = map_label[data->images(j).file_name()];
					cls.append(data->images(j).class_name());
					label.append(correct_label);
					file.append(data->images(j).file_name());
					answer.append(cur_label);
					if (cur_label == correct_label && pre_label != correct_label) { eventArr[0]++; }
					if (cur_label != correct_label && pre_label == correct_label) { eventArr[1]++; }
					if (cur_label == correct_label && pre_label == correct_label) { eventArr[2]++; }
					if (cur_label != correct_label && pre_label != correct_label) { eventArr[3]++; }
					map_label[data->images(j).file_name()] = cur_label;
					// BSONArrayBuilder probArr;
					// for (int k = 0; k < data->images(j).prob_size(); k++) { probArr.append(data->images(j).prob(k)); }
					// prob.append(probArr.arr());
				}
				bObj.append("cls", cls.arr());
				bObj.append("file", file.arr());
				bObj.append("label", label.arr());
				bObj.append("answer", answer.arr());
				for (int ei = 0; ei < 4; ei++) { event.append(eventArr[ei]); }
				bObj.append("event", event.arr());
				// bObj.append("prob", prob.arr());
				bs++;

				BSONObj o = bObj.obj();
				this->connection.insert(col, o);
			}
		}

		void DB::importStatKernel(analyzer::Infos::TYPE_STAT_KERNEL statName, analyzer::Infos::TYPE_CONTENT contentName, std::string type) {
			MAP_TYPE_STAT_KERNEL::iterator iterStat;
			if (type == "stat") {
				iterStat = mapTypeStatKernel.find(statName);
				if (iterStat == mapTypeStatKernel.end()) {
					std::cout << "Wrong TYPE_STAT" << std::endl;
					return;
				}
			}
			else if (type == "iv") {
				iterStat = mapTypeIvKernel.find(statName);
				if (iterStat == mapTypeIvKernel.end()) {
					std::cout << "Wrong TYPE_STAT" << std::endl;
					return;
				}
			}

			MAP_TYPE_CONTENT::iterator iterContent;
			iterContent = mapTypeContent.find(contentName);
			if (iterContent == mapTypeContent.end()) {
				std::cout << "Wrong TYPE_CONTENT" << std::endl;
				return;
			}
			std::string colName = iterStat->second;;

			std::string col = this->database + "." + this->dbName + "_" + colName;
			// std::cout << "Importing data to \""<< col << "\"." << std::endl;

			Info *data = this->iData;
			BulkOperationBuilder bulk = this->connection.initializeUnorderedBulkOp(col);

			for (int i = 0; i < data->layers_size(); i++) {
				if (data->layers(i).type() == "batch_norm") continue;
				BSONObjBuilder bObj;
				bObj
					.append("iter", data->iteration())
					.append("lid", i)
					.append("name", data->layers(i).name());
				BSONArrayBuilder floatArrValue;
				auto st = data->layers(i).stat_kernel(STAT_KERNEL_INDEX(statName, contentName));
				for (int j = 0; j < st.data_size(); j++) {
					floatArrValue.append(st.data(j));
				}
				bObj.append("value", floatArrValue.arr());
				bulk.insert(bObj.obj());
			}
			mongo::WriteResult rs;
			bulk.execute(0, &rs);

		}

		void DB::importAllStatsKernel() {
			for (auto it = mapTypeStatKernel.begin(); it != mapTypeStatKernel.end(); ++it) {
				this->importStatKernel(it->first, TYPE_CONTENT::WEIGHT, "stat");
			}
		}

		void DB::importAllStatsKernelIV() {
			for (auto it = mapTypeIvKernel.begin(); it != mapTypeIvKernel.end(); ++it) {
				this->importStatKernel(it->first, TYPE_CONTENT::WEIGHT, "iv");
			}
		}

		void DB::importAll() {
			this->importAllStats();
			this->importAllSeqs();
			//this->importImgInfo();
			this->importAllStatsKernel();
		}

		void DB::importLayerAttrs(std::string colName) {
			std::cout << this->database << std::endl;
			std::string col = this->database + "." + this->dbName + "_" + colName;
			std::cout << "Importing layer attrs to collection \"" << col << "\"." << std::endl;
			Info *data = this->iData;
			for (int i = 0; i < data->layers_size(); i++) {
				BSONObjBuilder bObj;
				bObj.append("lid", i)
					.append("name", data->layers(i).name())
					.append("type", data->layers(i).type())
					.append("channels", data->layers(i).channels())
					.append("kernelNum", data->layers(i).num())
					.append("kernelWidth", data->layers(i).width())
					.append("kernelHeight", data->layers(i).height());
				this->connection.insert(col, bObj.obj());
			}
			this->connection.createIndex(col, fromjson("{lid:1}"));
			this->connection.createIndex(col, fromjson("{type:1}"));
		}

		void DB::importRecorderInfo(int iteration, string type, float value) {
			std::string col;
			MAP_TYPE_RECORD::iterator iterRecord = mapTypeRecord.find(type);
			col = this->database + "." + this->dbName + "_" + iterRecord->second;
			BSONObjBuilder bObj;
			bObj.append("iter", iteration)
				.append("value", value);
			BSONObj o = bObj.obj();
			this->connection.insert(col, o);
		}

		void DB::createIndexes() {

			std::cout << "Creating Indexes " << this->dbName << std::endl;

			// create index for record info
			std::string col;
			for (auto it = mapTypeRecord.begin(); it != mapTypeRecord.end(); ++it) {
				col = this->database + "." + this->dbName + "_" + it->second;
				std::cout << "Creating Index on " << col << std::endl;
				this->connection.createIndex(col, fromjson("{iter:1}"));
			}

			// create index for layer stat info
			for (auto it = mapTypeStat.begin(); it != mapTypeStat.end(); ++it) {
				col = this->database + "." + this->dbName + "_Grad" + it->second;
				std::cout << "Creating Index on " << col << std::endl;
				this->connection.createIndex(col, fromjson("{iter:1}"));

				col = this->database + "." + this->dbName + "_Weight" + it->second;
				std::cout << "Creating Index on " << col << std::endl;
				this->connection.createIndex(col, fromjson("{iter:1}"));

				col = this->database + "." + this->dbName + "_HLGrad" + it->second;
				std::cout << "Creating Index on " << col << std::endl;
				this->connection.createIndex(col, fromjson("{iter:1}"));

				col = this->database + "." + this->dbName + "_HLWeight" + it->second;
				std::cout << "Creating Index on " << col << std::endl;
				this->connection.createIndex(col, fromjson("{iter:1}"));
			}

			// create index for layer seq info
			for (auto it = mapTypeSeq.begin(); it != mapTypeSeq.end(); ++it) {
				col = this->database + "." + this->dbName + "_Weight" + it->second;
				std::cout << "Creating Index on " << col << std::endl;
				this->connection.createIndex(col, fromjson("{iter:1}"));
			}

			// create index for kernel stat info
			for (auto it = mapTypeStatKernel.begin(); it != mapTypeStatKernel.end(); ++it) {
				col = this->database + "." + this->dbName + "_" + it->second;
				std::cout << "Creating Index on " << col << std::endl;
				this->connection.createIndex(col, fromjson("{iter:1}"));
				this->connection.createIndex(col, fromjson("{lid:1}"));
			}

			// create index for kernel interval (distance) info
			for (auto it = mapTypeIvKernel.begin(); it != mapTypeIvKernel.end(); ++it) {
				col = this->database + "." + this->dbName + "_" + it->second;
				std::cout << "Creating Index on " << col << std::endl;
				this->connection.createIndex(col, fromjson("{iter:1}"));
				this->connection.createIndex(col, fromjson("{lid:1}"));
			}

			//col = this->database + "." + this->dbName + "_ImgTrainInfo";
			//std::cout << "Creating Index on " << col << std::endl;
			//this->connection.createIndex(col, fromjson("{iter:1}"));

			// create index for test image info
			col = this->database + "." + this->dbName + "_ImgTestInfo";
			std::cout << "Creating Index on " << col << std::endl;
			this->connection.createIndex(col, fromjson("{iter:1}"));
			this->connection.createIndex(col, fromjson("{iter:1, batch:1}"));

			col = this->database + "." + this->dbName + "_ImgTestData";
			std::cout << "Creating Index on " << col << std::endl;
			this->connection.createIndex(col, fromjson("{file:1}"));
			this->connection.createIndex(col, fromjson("{cls:1}"));

			col = this->database + "." + this->dbName + "_ClsInfo";
			std::cout << "Creating Index on " << col << std::endl;
			this->connection.createIndex(col, fromjson("{name:1}"));

			col = this->database + "." + this->dbName + "_ImgTestClsStat";
			std::cout << "Creating Index on " << col << std::endl;
			this->connection.createIndex(col, fromjson("{iter:1}"));
			this->connection.createIndex(col, fromjson("{cls:1}"));

		}

		void DB::deleteDB() {
			std::cout << "Deleting database with name " << this->dbName << std::endl;
			std::string col;
			for (auto it = mapTypeRecord.begin(); it != mapTypeRecord.end(); ++it) {
				col = this->database + "." + this->dbName + "_" + it->second;
				std::cout << "Deleting collection " << col << std::endl;
				this->connection.dropCollection(col);
			}

			for (auto it = mapTypeStat.begin(); it != mapTypeStat.end(); ++it) {
				col = this->database + "." + this->dbName + "_Grad" + it->second;
				std::cout << "Deleting collection " << col << std::endl;
				this->connection.dropCollection(col);

				col = this->database + "." + this->dbName + "_Weight" + it->second;
				std::cout << "Deleting collection " << col << std::endl;
				this->connection.dropCollection(col);
			}

			for (auto it = mapTypeSeq.begin(); it != mapTypeSeq.end(); ++it) {
				col = this->database + "." + this->dbName + "_Grad" + it->second;
				std::cout << "Deleting collection " << col << std::endl;
				this->connection.dropCollection(col);

				col = this->database + "." + this->dbName + "_Weight" + it->second;
				std::cout << "Deleting collection " << col << std::endl;
				this->connection.dropCollection(col);
			}

			for (auto it = mapTypeStatKernel.begin(); it != mapTypeStatKernel.end(); ++it) {
				col = this->database + "." + this->dbName + "_" + it->second;
				std::cout << "Deleting collection " << col << std::endl;
				this->connection.dropCollection(col);
			}

			for (auto it = mapTypeIvKernel.begin(); it != mapTypeIvKernel.end(); ++it) {
				col = this->database + "." + this->dbName + "_" + it->second;
				std::cout << "Deleting collection " << col << std::endl;
				this->connection.dropCollection(col);
			}

			col = this->database + "." + this->dbName + "_" + "ImgTrainInfo";
			this->connection.dropCollection(col);
			col = this->database + "." + this->dbName + "_" + "ImgTestInfo";
			this->connection.dropCollection(col);

		}

		void DB::fetchTestIterSet(std::vector<int>& v) {
			std::string col = this->database + "." + this->dbName + "_" + "ImgTestData";
			cout << col << endl;
			BSONObj b = this->connection.findOne(col, BSONObj());
			vector<BSONElement> iters = b.getField("iter").Array();
			v.reserve(iters.size());
			for (int i = 0; i < iters.size(); i++) {
				v.push_back(iters[i].Int());
			}
		}

		void DB::processImgData() {
			std::string col = this->database + "." + this->dbName + "_" + "ImgTestInfo";
			std::string colWrite = this->database + "." + this->dbName + "_" + "ImgTestData";
			// initialization
			COUT_G("initializing") << std::endl;
			std::map<string, Img> imgs;
			BSONObj b = this->connection.findOne(col, BSONObj());
			vector<BSONElement> v_files = b.getField("file").Array();
			vector<BSONElement> v_cls = b.getField("cls").Array();
			vector<BSONElement> v_label = b.getField("label").Array();
			for (int i = 0; i < v_files.size(); i++) {
				auto file = v_files[i].String();
				auto cls = v_cls[i].String();
				auto label = v_label[i].Int();
				vector<int> iter;
				vector<int> answer;

				struct Img tmp = {
					file,
					cls,
					label,
					iter,
					answer
				};

				imgs[file] = tmp;
			}
			BSONObj proj = (BSONObjBuilder() << "_id" << 0 << "iter" << 1 << "file" << 1 << "label" << 1 << "answer" << 1).obj();
			std::auto_ptr<DBClientCursor> cursor = this->connection.query(col, BSONObj(), 0, 0, &proj);

			COUT_G("fetching and processing") << std::endl;
			int count = 0;
			while (cursor->more()) {
				BSONObj b = cursor->next();
				int iter = b.getIntField("iter");
				vector<BSONElement> v_files = b.getField("file").Array();
				vector<BSONElement> v_labels = b.getField("label").Array();
				vector<BSONElement> v_answers = b.getField("answer").Array();
				for (int i = 0; i < v_files.size(); i++) {
					auto file = v_files[i].String();
					auto answer = v_answers[i].Int();
					imgs[file].iter.push_back(iter);
					imgs[file].answer.push_back(answer);
				}
				if (++count % 50 == 0) { cout << count << endl; }
			}

			COUT_G("inserting") << std::endl;
			// insert data to mongoDB
			count = 0;
			for (auto iter = imgs.begin(); iter != imgs.end(); iter++) {
				BSONObjBuilder bObj;
				auto o = iter->second;
				bObj.append("file", o.file)
					.append("cls", o.cls)
					.append("label", o.label);

				BSONArrayBuilder arrIter;
				for (auto const& d : o.iter) { arrIter.append(d); }
				BSONArrayBuilder arrAnswer;
				for (auto const& d : o.answer) { arrAnswer.append(d); }

				bObj.append("iter", arrIter.arr())
					.append("answer", arrAnswer.arr());

				this->connection.insert(colWrite, bObj.obj());
				if (++count % 1000 == 0) { cout << count << endl; }
			}

			// create index
			this->connection.createIndex(colWrite, fromjson("{file:1}"));
			this->connection.createIndex(colWrite, fromjson("{cls:1}"));
		}

	}

}
