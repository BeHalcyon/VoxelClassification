#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include "SourceVolume.h"
#include "ParameterControlWidget.h"
#include "SliceView.h"
#include "data2word.h"
#include "TrainNetwork.h"
#include <VMUtils/json_binding.hpp>


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);

	void setConnectionState();
	void getWordMapFromFile(std::map<std::string, std::vector<float>>& word_map);
	void getLabelMapFromFile(std::map<std::string, std::vector<float>>& label_map);
	bool getDrawedState() const { return is_drawed; }

public slots:
	void slot_ImportVifoFile();
	void slot_ImportJsonFile();
	void slot_SaveWWNet();
	void slot_SaveWordNode();
	void calcLabelArray();
	void slot_SaveLWNet();
	void slot_SaveLabelNode();
	void slot_SaveWordLabelNet(const int edge_weight_type = 0);
	void slot_SaveWordLabelNode();
	void slot_ExportNetAndNodeFile();

private:
	Ui::MainWindowClass	ui;
	std::string				infoFileName = "F:\\atmosphere\\timestep21_float\\_SPEEDf21.vifo";
	int						data_number;
	std::string				datatype;
	hxy::my_int3			dimension;
	hxy::my_double3			space;
	vector<string>			file_list;
	vector<unsigned char>	volume_data;
	std::string				file_path;


	ParameterControlWidget*	parameter_control_widget;
	QDockWidget *			parameter_dock;
	SliceView *				slice_view;
	int						plane_mode = 0;	//0 for xy-plane, 1 for yz-plane, 2 for xz-plane;
	int						slice_id = 0;
	bool					is_drawed = false;


	SourceVolume			source_volume;
	Volume2Word				volume2word;
	bool					is_volume2word_calculated = false;
	bool					is_vifo_file_loaded = false;
	QVector<QVector<int>>	context_label;

	std::unique_ptr<TrainNetwork>			train_network;
	bool					is_json_file_loaded = false;
};
