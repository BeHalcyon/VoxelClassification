#include "MainWindow.h"

#include <QFileDialog>
#include <QDebug>
#include <QGraphicsPathItem>
#include <iostream>
#include <fstream>
#include <QDockWidget>
#include <QDialog>
#include <QMessageBox>
#include "VolumeSegmentation.h"


struct InputFileJSONStruct : public vm::json::Serializable<InputFileJSONStruct>
{
	VM_JSON_FIELD(std::string, vifo_file_name);
	VM_JSON_FIELD(int, volume_index);

	VM_JSON_FIELD(std::string, file_prefix);


	VM_JSON_FIELD(std::string, output_ww_net);
	VM_JSON_FIELD(std::string, output_word_node);
	VM_JSON_FIELD(std::string, output_lw_net);
	VM_JSON_FIELD(std::string, output_label_node);
	VM_JSON_FIELD(std::string, output_text_hin);
	VM_JSON_FIELD(std::string, output_text_node);
	VM_JSON_FIELD(int, window_size);
	VM_JSON_FIELD(int, edge_weight_type);
}JSON;


void readInfoFile(const std::string& infoFileName, int& data_number, std::string& datatype, hxy::my_int3& dimension, hxy::my_double3& space,
	std::vector<std::string>& file_list)
{
	file_list.clear();

	ifstream inforFile(infoFileName);

	inforFile >> data_number;
	inforFile >> datatype;
	inforFile >> dimension.x >> dimension.y >> dimension.z;
	//Debug 20190520 增加sapce接口
	inforFile >> space.x >> space.y >> space.z;
	const string filePath = infoFileName.substr(0, infoFileName.find_last_of('/') == -1 ?
		infoFileName.find_last_of('\\') + 1 : infoFileName.find_last_of('/') + 1);
	std::cout << (filePath.c_str()) << std::endl;
	for (auto i = 0; i < data_number; i++)
	{
		string rawFileName;
		inforFile >> rawFileName;
		string volumePath = filePath + rawFileName;
		file_list.push_back(volumePath);
	}
	std::cout << "Info file name : \t\t" << infoFileName.c_str() << std::endl;
	std::cout << "Volume number : \t\t" << data_number << std::endl;
	std::cout << "data type : \t\t\t" << datatype.c_str() << std::endl;
	std::cout << "Volume dimension : \t\t" << "[" << dimension.x << "," << dimension.y << "," << dimension.z << "]" << std::endl;
	std::cout << "Space dimension : \t\t" << "[" << space.x << "," << space.y << "," << space.z << "]" << std::endl;
	for (auto i = 0; i < data_number; i++)
	{
		std::cout << "Volume " << i << " name : \t\t" << file_list[i].c_str() << std::endl;
	}

	std::cout << "Info file has been loaded successfully." << std::endl;
}

void showMessageBox(const QString& text)
{
	QMessageBox msgBox;
	msgBox.setText(text);
	msgBox.exec();
}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowState(windowState() ^ Qt::WindowMaximized);

	parameter_control_widget = new ParameterControlWidget(this);
	parameter_dock = new QDockWidget(tr("Parameter Setting"), this);

	parameter_dock->setWidget(parameter_control_widget);
	addDockWidget(Qt::LeftDockWidgetArea, parameter_dock);

	slice_view = new SliceView(this);


	train_network = nullptr;
	//QDialog* dialog = new QDialog(slice_view);
	//dialog->setModal(true);
	//dialog->show();
	setCentralWidget(slice_view);

	setConnectionState();
}

void MainWindow::setConnectionState()
{
	connect(ui.action_vifo, &QAction::triggered, this, &MainWindow::slot_ImportVifoFile);
	connect(ui.action_json, &QAction::triggered, this, &MainWindow::slot_ImportJsonFile);
	connect(parameter_control_widget->ui.comboBox_Slice_Direction, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index)
	{
		if (index == 0)
			parameter_control_widget->ui.spinBox_Slice_index->setMaximum(dimension.z);
		else if (index == 1)
			parameter_control_widget->ui.spinBox_Slice_index->setMaximum(dimension.x);
		else
			parameter_control_widget->ui.spinBox_Slice_index->setMaximum(dimension.y);

		slice_id = 0;

		plane_mode = index;
		if (is_drawed)
		{
			slice_view->updateImage(volume_data, dimension, plane_mode, slice_id);
		}
	});
	connect(parameter_control_widget->ui.spinBox_Slice_index, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value)
	{
		slice_id = value;
		if (is_drawed)
		{
			//std::cout << "test" << std::endl;
			slice_view->updateImage(volume_data, dimension, plane_mode, slice_id);
		}
	});

	connect(slice_view, &SliceView::signal_updateSliceId, [this](int offset)
	{
		if (offset == 1)
		{
			if (parameter_control_widget->ui.spinBox_Slice_index->maximum() > slice_id + 1)
				parameter_control_widget->ui.spinBox_Slice_index->setValue(slice_id + 1);
		}
		else if (offset == -1)
		{
			if (0 <= slice_id - 1)
				parameter_control_widget->ui.spinBox_Slice_index->setValue(slice_id - 1);
		}
	});

	connect(parameter_control_widget->ui.pushButton_add_label, &QPushButton::clicked, [this]()
	{
		if (!is_drawed)
		{
			showMessageBox("Please load vifo file first.");
		}
		const auto label_name = parameter_control_widget->ui.line_edit_new_label_name->text();

		auto combo_list = parameter_control_widget->ui.comboBox_label_name_list;
		for (auto i = 0; i < combo_list->count(); i++)
		{
			if (label_name == combo_list->itemText(i)) return;
		}
		combo_list->addItem(label_name);
		slice_view->createNewPathItemArray(label_name);

		//const auto label_name = parameter_control_widget->ui.comboBox_label_name_list->currentText();
		slice_view->setLabel(label_name);
		parameter_control_widget->ui.line_edit_new_label_name->setText("label:Name");
	});
	connect(parameter_control_widget->ui.pushButton_delete_label,&QPushButton::clicked,[this]()
	{
		if(slice_view)
		{
			slice_view->deleteAllItems();
		}
		parameter_control_widget->ui.line_edit_new_label_name->setText("label:Name");
		parameter_control_widget->ui.comboBox_label_name_list->clear();
	});
	connect(parameter_control_widget->ui.pushButton_select_label, &QPushButton::clicked, [this]()
	{
		const auto label_name = parameter_control_widget->ui.comboBox_label_name_list->currentText();
		slice_view->setLabel(label_name);
	});
	connect(slice_view, &SliceView::signal_updateLableNumber, [this](const QVector<QVector<QVector<QVector<QGraphicsPathItem*>>>>& vector)
	{

		auto combo_list = parameter_control_widget->ui.comboBox_label_name_list;
		for (auto i = 0; i < combo_list->count(); i++)
		{
			const auto label_name = combo_list->itemText(i);
			const auto label_id = slice_view->getLabelId(label_name);
			if (label_id == -1)
			{
				qDebug() << "Error: label id is -1";
				return;
			}
			parameter_control_widget->ui.tableWidget->setItem(i, 0, new QTableWidgetItem(label_name));

			int cnt = 0;

			for (const auto& j : vector[label_id])
			{
				for (const auto& k : j)
				{
					for (auto buf : k)
					{
						cnt += buf->path().elementCount();
					}
				}
			}
			parameter_control_widget->ui.tableWidget->setItem(i, 1, new QTableWidgetItem(QString("%1").arg(cnt)));
		}
	});

	connect(ui.action_net, &QAction::triggered, this, &MainWindow::slot_ExportNetAndNodeFile);

	connect(ui.action_node, &QAction::triggered, [this]() {

		QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
			"./workspave/labels.node",
			tr("label node (*.node)"));
		if (fileName.isEmpty())
			return;

		/***
		 * 以二进制形式存储非重复label文件，文件格式："
		 * label:1
		 * label:2
		 * label:3
		 * "
		 */
		auto fo = fopen(fileName.toStdString().c_str(), "wb");
		auto combo = parameter_control_widget->ui.comboBox_label_name_list;
		for (auto i = 0; i < combo->count(); i++)
		{
			const auto label_name = combo->itemText(i);
			fprintf(fo, "%s\n", label_name.toStdString().c_str());
		}
		fclose(fo);
	});



	connect(parameter_control_widget->ui.pushButton_save_ww_net, &QPushButton::clicked, this, &MainWindow::slot_SaveWWNet);
	connect(parameter_control_widget->ui.pushButton_save_lw_net, &QPushButton::clicked, this, &MainWindow::slot_SaveLWNet);
	connect(parameter_control_widget->ui.pushButton_save_words_node, &QPushButton::clicked, this, &MainWindow::slot_SaveWordNode);
	connect(parameter_control_widget->ui.pushButton_save_labels_node, &QPushButton::clicked, this, &MainWindow::slot_SaveLabelNode);
	connect(parameter_control_widget->ui.pushButton_save_word_label_node, &QPushButton::clicked, this, &MainWindow::slot_SaveWordLabelNode);
	connect(parameter_control_widget->ui.pushButton__save_ww_lw_net, &QPushButton::clicked, [this]()
	{
		if (is_json_file_loaded)
			slot_SaveWordLabelNet(JSON.edge_weight_type);
		//默认权重为1
		else
			slot_SaveWordLabelNet();
	});
	connect(parameter_control_widget->ui.pushButton_save_all_middle_file, &QPushButton::clicked, [this]()
	{
		slot_SaveWWNet();
		slot_SaveLWNet();
		slot_SaveWordNode();
		slot_SaveLabelNode();
		slot_SaveWordLabelNode();

		if (is_json_file_loaded)
			slot_SaveWordLabelNet(JSON.edge_weight_type);
		//默认权重为1
		else
			slot_SaveWordLabelNet();
	});
	connect(parameter_control_widget->ui.pushButton_save_net_node, &QPushButton::clicked, [this]()
	{
		slot_SaveWWNet();
		slot_SaveLWNet();
		slot_SaveWordNode();
		slot_SaveLabelNode();
		//slot_SaveWordLabelNode();
		//slot_SaveWordLabelNet();
	});

	connect(parameter_control_widget->ui.pushButton_train_network,&QPushButton::clicked, [this]()
	{
		const auto vector_size = parameter_control_widget->ui.spinBox_vector_size->value();
		const auto sample_number = parameter_control_widget->ui.spinBox_sample_number->value()*(long long)1000000;
		const auto alpha = parameter_control_widget->ui.doubleSpinBox_alpha->value();
		int negative_number = parameter_control_widget->ui.spinBox_negative_number->value();
		int num_threads = parameter_control_widget->ui.spinBox_thread_number->value();

		train_network = std::make_unique<TrainNetwork>("./workspace/text.node", "./workspace/words.node", "./workspace/text.hin", vector_size,
			negative_number, sample_number, alpha, num_threads);

		train_network->TrainModel();
		train_network->saveWordEmbedding();
		train_network->saveLabelEmbedding();

		showMessageBox("The graph net has been trained.");
	});
	connect(parameter_control_widget->ui.pushButton_classification_volume, &QPushButton::clicked, [this]()
	{
		if(train_network&&!train_network->trainState())
		{
			showMessageBox("Please train network first.");
			return;
		}

		std::map<std::string, std::vector<float>> word_map;
		std::map<std::string, std::vector<float>> label_map;
		if(!train_network)
		{
			// QString fileName = QFileDialog::getOpenFileName(this, "Open vifo File", "J:/science data/4 Combustion/jet_0051/",
			// 	tr("VIFO (*.vifo )"));
			// if (fileName.isEmpty())
			// 	return;
			// infoFileName = fileName.toStdString();
			readInfoFile("F:\\TOOTH_8bit_128_128_160\\TOOTH_8bit_128_128_160.vifo", data_number, datatype, dimension, space, file_list);
			SourceVolume source_volume(file_list, dimension.x, dimension.y, dimension.z, datatype);

			source_volume.loadVolume();
			source_volume.loadRegularVolume();
			volume_data = *source_volume.getRegularVolume(0);

			getWordMapFromFile(word_map);
			getLabelMapFromFile(label_map);
		}
		else
		{
			word_map = train_network->exportWordVector();
			label_map = train_network->exportLabelVector();
		}

		std::vector<int> segmentation_vector(dimension.x*dimension.y*dimension.z, 0);

		VolumeSegmentation volume_segmentation;
		volume_segmentation.segemation(volume_data, dimension.x, dimension.y, dimension.z,
		                               word_map, label_map,
		                               parameter_control_widget->ui.spinBox_window_size_train->value(), 
		                               parameter_control_widget->ui.spinBox_vector_size->value(),
		                               segmentation_vector, parameter_control_widget->ui.doubleSpinBox_threshold->value());

		volume_segmentation.saveSegmentation(segmentation_vector);
		showMessageBox("The volume segmentation has been calculated.");

	});
}

void MainWindow::getWordMapFromFile(std::map<std::string, std::vector<float>>& word_map)
{
	string file_name = "./workspace/word.emb";
	word_map.clear();
	ifstream word_file(file_name);
	int words_number, vector_dimension;
	word_file >> words_number >> vector_dimension;
	string word;
	for (auto i = 0; i < words_number; i++)
	{
		word_file >> word;
		vector<float> word_vector(vector_dimension);
		for (auto j = 0; j < vector_dimension; j++)
		{
			word_file >> word_vector[j];
			//std::cout << word_vector[j] << " ";
		}
		//std::cout << std::endl;
		word_map[word] = word_vector;
		
	}
	std::cout << "word.emb has been loaded." << std::endl;
}

bool isNum(string str)
{
	std::stringstream sin(str);
	double d;
	char c;
	if (!(sin >> d))
	{
		return false;
	}
	if (sin >> c)
	{
		return false;
	}
	return true;
}


void MainWindow::getLabelMapFromFile(std::map<std::string, std::vector<float>>& label_map)
{
	string file_name = "./workspace/all_node.emb";
	label_map.clear();
	ifstream word_file(file_name);
	int words_number, vector_dimension;
	word_file >> words_number >> vector_dimension;
	string word;
	for (auto i = 0; i < words_number; i++)
	{
		word_file >> word;
		
		vector<float> word_vector(vector_dimension);
		for (auto j = 0; j < vector_dimension; j++)
		{
			word_file >> word_vector[j];
		}
		if (!word.empty()&&!isNum(word))
		{
			label_map[word] = word_vector;
		}
		std::cout << word << std::endl;
	}
	std::cout << "all_node.emb has been loaded." << std::endl;

}

void MainWindow::slot_ExportNetAndNodeFile()
{

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
		"./workspave/lw.net",
		tr("label-word net (*.net)"));
	if (fileName.isEmpty())
		return;

	//全局到局部转换,过滤外界点,平面到立体
	auto label_array = slice_view->tranPathListToVolumeIndex();


	//获取window size
	int window_size = parameter_control_widget->ui.spinBox_window_size->value();
	const auto sz = dimension.x*dimension.y*dimension.z;
	context_label.resize(label_array.size());
	for (auto i = 0; i < context_label.size(); i++) context_label[i].resize(256);

	if (window_size == 3)
	{
		const int dx26[26] = { -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1 };
		const int dy26[26] = { -1, -1, -1,  0,  0,  0,  1,  1,  1, -1, -1, -1,  0,  0,  1,  1,  1, -1, -1, -1,  0,  0,  0,  1,  1,  1 };
		const int dz26[26] = { -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1 };


		for (auto i = 0; i < label_array.size(); i++)
		{
			for (auto index = 0; index < label_array[i].size(); index++)
			{
				auto center_index = label_array[i][index];

				const int oz = center_index / (dimension.x*dimension.y);
				const int ox = center_index % dimension.x;
				const int oy = (center_index % (dimension.x*dimension.y)) / dimension.x;

				context_label[i][volume_data[center_index]]++;

				for (auto p = 0; p < 26; p++)
				{
					int nx = ox + dx26[p];//new x
					int ny = oy + dy26[p];//new y
					int nz = oz + dz26[p];//new z

					if (nx >= 0 && nx < dimension.x && ny >= 0 && ny < dimension.y && nz >= 0 && nz < dimension.z)
					{
						int nind = nz * dimension.x*dimension.y + ny * dimension.x + nx;
						context_label[i][volume_data[nind]]++;
					}
				}
			}
		}
	}
	else if (window_size == 1)
	{
		const int dx6[6] = { -1,  1,  0,  0,  0,  0 };
		const int dy6[6] = { 0,  0, -1,  1,  0,  0 };
		const int dz6[6] = { 0,  0,  0,  0, -1,  1 };


		for (auto i = 0; i < label_array.size(); i++)
		{
			for (auto index = 0; index < label_array[i].size(); index++)
			{
				auto center_index = label_array[i][index];

				const int oz = center_index / (dimension.x*dimension.y);
				const int ox = center_index % dimension.x;
				const int oy = (center_index % (dimension.x*dimension.y)) / dimension.x;

				context_label[i][volume_data[center_index]]++;

				for (auto p = 0; p < 6; p++)
				{
					int nx = ox + dx6[p];//new x
					int ny = oy + dy6[p];//new y
					int nz = oz + dz6[p];//new z

					if (nx >= 0 && nx < dimension.x && ny >= 0 && ny < dimension.y && nz >= 0 && nz < dimension.z)
					{
						int nind = nz * dimension.x*dimension.y + ny * dimension.x + nx;
						context_label[i][volume_data[nind]]++;
					}
				}
			}
		}
	}

	//保存为文件

	auto fo = fopen(fileName.toStdString().c_str(), "wb");
	//以label为数量循环
	auto combo = parameter_control_widget->ui.comboBox_label_name_list;

	for (int m = 0; m < context_label.size(); m++)
	{
		const auto label_name = combo->itemText(m);
		for (int n = 0; n < context_label[m].size(); n++)
		{
			if (context_label[m][n] > 0)
			{
				fprintf(fo, "%s %d %d l\n", label_name.toStdString().c_str(), n, context_label[m][n]);
			}
		}

	}
	fclose(fo);
}

void MainWindow::slot_ImportVifoFile()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Open vifo File", "J:/science data/4 Combustion/jet_0051/",
		tr("VIFO (*.vifo )"));
	if (fileName.isEmpty())
		return;
	infoFileName = fileName.toStdString();
	readInfoFile(infoFileName, data_number, datatype, dimension, space, file_list);


	file_path = file_list[0].substr(0, file_list[0].find_last_of('.'));

	source_volume = SourceVolume(file_list, dimension.x, dimension.y, dimension.z, datatype);

	//source_volume.loadVolume();	//origin data
	source_volume.loadRegularVolume(); //[0, 255] data

	volume_data = *source_volume.getRegularVolume(0);

	std::cout << "The regular volume data has been loaded." << std::endl;

	parameter_control_widget->ui.spinBox_XDim->setValue(dimension.x);
	parameter_control_widget->ui.spinBox_XDim->setEnabled(false);
	parameter_control_widget->ui.spinBox_YDim->setValue(dimension.y);
	parameter_control_widget->ui.spinBox_YDim->setEnabled(false);
	parameter_control_widget->ui.spinBox_ZDim->setValue(dimension.z);
	parameter_control_widget->ui.spinBox_ZDim->setEnabled(false);

	parameter_control_widget->ui.spinBox_Slice_index->setMaximum(dimension.z - 1);


	slice_view->updateImage(volume_data, dimension, plane_mode, slice_id);

	is_drawed = true;
	is_volume2word_calculated = false;
	is_vifo_file_loaded = true;
}

void MainWindow::slot_ImportJsonFile()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Open json File", "./",
		tr("JSON (*.json )"));

	if (fileName.isEmpty())
		return;

	std::ifstream input_file(fileName.toStdString());
	input_file >> JSON;

	infoFileName = JSON.vifo_file_name;

	auto volume_index = JSON.volume_index;
	//infoFileName = fileName.toStdString();
	readInfoFile(infoFileName, data_number, datatype, dimension, space, file_list);


	file_path = file_list[volume_index].substr(0, file_list[volume_index].find_last_of('.'));

	source_volume = SourceVolume(file_list, dimension.x, dimension.y, dimension.z, datatype);

	//source_volume.loadVolume();	//origin data
	source_volume.loadRegularVolume(); //[0, 255] data

	volume_data = *source_volume.getRegularVolume(0);

	std::cout << "The regular volume data has been loaded." << std::endl;

	parameter_control_widget->ui.spinBox_XDim->setValue(dimension.x);
	parameter_control_widget->ui.spinBox_XDim->setEnabled(false);
	parameter_control_widget->ui.spinBox_YDim->setValue(dimension.y);
	parameter_control_widget->ui.spinBox_YDim->setEnabled(false);
	parameter_control_widget->ui.spinBox_ZDim->setValue(dimension.z);
	parameter_control_widget->ui.spinBox_ZDim->setEnabled(false);

	parameter_control_widget->ui.spinBox_Slice_index->setMaximum(dimension.z - 1);

	slice_view->updateImage(volume_data, dimension, plane_mode, slice_id);

	is_drawed = true;
	is_volume2word_calculated = false;
	is_vifo_file_loaded = true;

	is_json_file_loaded = true;

	parameter_control_widget->ui.spinBox_window_size->setValue(JSON.window_size);
}

void MainWindow::slot_SaveWWNet()
{
	if(!is_vifo_file_loaded)
	{
		showMessageBox("Please load vifo file first!");
		return;
	}
	if(!is_volume2word_calculated)
	{
		volume2word.clear();
		volume2word.process(volume_data.data(), 
			dimension.x, dimension.y, dimension.z, 
			parameter_control_widget->ui.spinBox_window_size->value());
		is_volume2word_calculated = true;
	}
	if(is_json_file_loaded)
	{
		volume2word.saveNet(JSON.file_prefix + JSON.output_ww_net);
	}
	else
	{
		volume2word.saveNet();
	}
	std::cout << "ww.net has been saved in ./workspace/...ww.net" << std::endl;
	//showMessageBox("ww.net has been saved in ./workspace/ww.net");
}

void MainWindow::slot_SaveWordNode()
{
	if (!(is_vifo_file_loaded|| is_json_file_loaded))
	{
		showMessageBox("Please load vifo file first!");
		return;
	}
	if (!is_volume2word_calculated)
	{
		volume2word.clear();
		volume2word.process(volume_data.data(),
			dimension.x, dimension.y, dimension.z,
			parameter_control_widget->ui.spinBox_window_size->value());
		is_volume2word_calculated = true;
	}
	if (is_json_file_loaded)
	{
		volume2word.saveWords(JSON.file_prefix + JSON.output_word_node);
	}
	else
	{
		volume2word.saveWords();
	}
	
	std::cout << "words.node has been saved in ./workspace/...words.node" << std::endl;
	//showMessageBox("words.node has been saved in ./workspace/words.node");
}

void MainWindow::calcLabelArray()
{
	//全局到局部转换,过滤外界点,平面到立体
	auto label_array = slice_view->tranPathListToVolumeIndex();

	//获取window size
	int window_size = parameter_control_widget->ui.spinBox_window_size->value();
	const auto sz = dimension.x*dimension.y*dimension.z;
	context_label.resize(label_array.size());
	for (auto i = 0; i < context_label.size(); i++) context_label[i].resize(256);

	if (window_size == 3)
	{
		const int dx26[26] = { -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1 };
		const int dy26[26] = { -1, -1, -1,  0,  0,  0,  1,  1,  1, -1, -1, -1,  0,  0,  1,  1,  1, -1, -1, -1,  0,  0,  0,  1,  1,  1 };
		const int dz26[26] = { -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1 };


		for (auto i = 0; i < label_array.size(); i++)
		{
			for (auto index = 0; index < label_array[i].size(); index++)
			{
				auto center_index = label_array[i][index];

				const int oz = center_index / (dimension.x*dimension.y);
				const int ox = center_index % dimension.x;
				const int oy = (center_index % (dimension.x*dimension.y)) / dimension.x;

				context_label[i][volume_data[center_index]]++;

				for (auto p = 0; p < 26; p++)
				{
					int nx = ox + dx26[p];//new x
					int ny = oy + dy26[p];//new y
					int nz = oz + dz26[p];//new z

					if (nx >= 0 && nx < dimension.x && ny >= 0 && ny < dimension.y && nz >= 0 && nz < dimension.z)
					{
						int nind = nz * dimension.x*dimension.y + ny * dimension.x + nx;
						context_label[i][volume_data[nind]]++;
					}
				}
			}
		}
	}
	else if (window_size == 1)
	{
		const int dx6[6] = { -1,  1,  0,  0,  0,  0 };
		const int dy6[6] = { 0,  0, -1,  1,  0,  0 };
		const int dz6[6] = { 0,  0,  0,  0, -1,  1 };


		for (auto i = 0; i < label_array.size(); i++)
		{
			for (auto index = 0; index < label_array[i].size(); index++)
			{
				auto center_index = label_array[i][index];

				const int oz = center_index / (dimension.x*dimension.y);
				const int ox = center_index % dimension.x;
				const int oy = (center_index % (dimension.x*dimension.y)) / dimension.x;

				context_label[i][volume_data[center_index]]++;

				for (auto p = 0; p < 6; p++)
				{
					int nx = ox + dx6[p];//new x
					int ny = oy + dy6[p];//new y
					int nz = oz + dz6[p];//new z

					if (nx >= 0 && nx < dimension.x && ny >= 0 && ny < dimension.y && nz >= 0 && nz < dimension.z)
					{
						int nind = nz * dimension.x*dimension.y + ny * dimension.x + nx;
						context_label[i][volume_data[nind]]++;
					}
				}
			}
		}
	}
}

void MainWindow::slot_SaveLWNet()
{
	if (!(is_vifo_file_loaded || is_json_file_loaded))
	{
		showMessageBox("Please load vifo file first!");
		return;
	}

	calcLabelArray();

	FILE* fo = nullptr;
	if(is_json_file_loaded)
	{
		std::string file_name = JSON.file_prefix + JSON.output_word_node;
		fo = fopen(file_name.c_str(), "wb");
	}
	else
	{
		fo = fopen("./workspace/lw.net", "wb");
	}
	
	//以label为数量循环
	auto combo = parameter_control_widget->ui.comboBox_label_name_list;

	for (int m = 0; m < context_label.size(); m++)
	{
		const auto label_name = combo->itemText(m);
		for (int n = 0; n < context_label[m].size(); n++)
		{
			if (context_label[m][n] > 0)
			{
				fprintf(fo, "%s %d %d l\n", label_name.toStdString().c_str(), n, context_label[m][n]);
			}
		}
	}
	fclose(fo);
	std::cout << "label-word net has been saved in ./workspace/...lw.net" << std::endl;
	//showMessageBox("label-word net has been saved in ./workspace/lw.net");
}

void MainWindow::slot_SaveLabelNode()
{
	if (!(is_vifo_file_loaded || is_json_file_loaded))
	{
		showMessageBox("Please load vifo file first!");
		return;
	}

	calcLabelArray();
	FILE* fo = nullptr;
	if(is_json_file_loaded)
	{
		std::string file_name = JSON.file_prefix + JSON.output_label_node;
		fo = fopen(file_name.c_str(), "wb");
	}
	else
	{
		fo = fopen("./workspace/labels.node", "wb");
	}
	auto combo = parameter_control_widget->ui.comboBox_label_name_list;
	for (auto i = 0; i < combo->count(); i++)
	{
		const auto label_name = combo->itemText(i);
		fprintf(fo, "%s\n", label_name.toStdString().c_str());
	}
	fclose(fo);
	//showMessageBox("label-word net has been saved in ./workspace/lw.net");
	std::cout << "label-word net has been saved in ./workspace/...lw.net" << std::endl;
}

void MainWindow::slot_SaveWordLabelNet(const int edge_weight_type)
{
	if (!(is_vifo_file_loaded || is_json_file_loaded))
	{
		showMessageBox("Please load vifo file first!");
		return;
	}
	if (!is_volume2word_calculated)
	{
		volume2word.clear();
		volume2word.process(volume_data.data(),
			dimension.x, dimension.y, dimension.z,
			parameter_control_widget->ui.spinBox_window_size->value());
		is_volume2word_calculated = true;
	}
	auto neighbor_histogram = volume2word.getNeighborHistogram();

	//使用原始的权重列表
	if(edge_weight_type == 1)
	{
		
	}
	//使用规则化权重列表
	else if(edge_weight_type == 2)
	{
		for (auto& i : neighbor_histogram)
		{
			auto max_value = 0.0;
			for (double j : i)
			{
				max_value = std::max(max_value, j);
			}
			if (max_value == 0.0) continue;
			
			for (double& j : i)
			{
				j /=max_value;
			}
		}
	}

	FILE *fo = nullptr;
	if(is_json_file_loaded)
	{
		std::string file_name = JSON.file_prefix + JSON.output_text_hin;
		fo = fopen(file_name.c_str(), "wb");
	}
	else
	{
		fo = fopen("./workspace/text.hin", "wb");
	}
	auto cnt = 0;
	for (auto i = 0; i < HISTOGRAM_SIZE; i++)
	{
		for (auto j = 0; j < HISTOGRAM_SIZE; j++)
		{
			if (cnt % 1000 == 0)
			{
				printf("%cWrite word-word file: %.3lf%%", 13, double(cnt) / (HISTOGRAM_SIZE*HISTOGRAM_SIZE) * 100);
				fflush(stdout);
			}
			if (neighbor_histogram[i][j] > 0)
			{
				//Debug 20191121
				fprintf(fo, "%s %s %lf w\n", std::to_string(i).c_str(), std::to_string(j).c_str(), edge_weight_type ? neighbor_histogram[i][j] : 1);
				//fprintf(fo, "%s %s %d w\n", std::to_string(i).c_str(), std::to_string(j).c_str(), 1);
			}
			cnt++;
		}
	}

	calcLabelArray();
	//以label为数量循环
	auto combo = parameter_control_widget->ui.comboBox_label_name_list;

	for (int m = 0; m < context_label.size(); m++)
	{
		const auto label_name = combo->itemText(m);
		for (int n = 0; n < context_label[m].size(); n++)
		{
			if (context_label[m][n] > 0)
			{
				//Debug 20191121
				//fprintf(fo, "%s %d %d l\n", label_name.toStdString().c_str(), n, context_label[m][n]);
				fprintf(fo, "%s %d %d l\n", label_name.toStdString().c_str(), n, edge_weight_type ? context_label[m][n]:1);
			}
		}
	}
	fclose(fo);

	//showMessageBox("ww net and label net have been merged together in ./workspace/text.hin");

	std::cout << "ww net and label net have been merged together in ./workspace/...text.hin" << std::endl;
}

void MainWindow::slot_SaveWordLabelNode()
{
	if (!(is_vifo_file_loaded || is_json_file_loaded))
	{
		showMessageBox("Please load vifo file first!");
		return;
	}
	if (!is_volume2word_calculated)
	{
		volume2word.clear();
		volume2word.process(volume_data.data(),
			dimension.x, dimension.y, dimension.z,
			parameter_control_widget->ui.spinBox_window_size->value());
		is_volume2word_calculated = true;
	}


	FILE* fo;
	if(is_json_file_loaded)
	{
		std::string file_name = JSON.file_prefix + JSON.output_text_node;
		fo = fopen(file_name.c_str(), "w");
	}
	else
	{
		fo = fopen("./workspace/text.node", "w");
	}

	for (int k = 0; k < volume2word.getVertexNumber(); k++)
		fprintf(fo, "%s\n", std::to_string(volume2word.getIntWordByHashID(k)).c_str());


	auto combo = parameter_control_widget->ui.comboBox_label_name_list;
	for (auto i = 0; i < combo->count(); i++)
	{
		const auto label_name = combo->itemText(i);
		fprintf(fo, "%s\n", label_name.toStdString().c_str());
	}
	fclose(fo);

	//showMessageBox("words node and labels node have been merged together in ./workspace/text.node");
	std::cout << "words node and labels node have been merged together in ./workspace/text.node" << std::endl;
}