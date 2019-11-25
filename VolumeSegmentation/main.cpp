#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include "SourceVolume.h"
#include "VolumeSegmentation.h"
//#include "VMUtils/include/VMUtils/json_binding.hpp"
//#include "VMUtils/include/VMUtils/cmdline.hpp"
#include "../VoxelClassification/json_struct.h"
#include "../VoxelClassification/VMUtils/include/VMUtils/cmdline.hpp"
#include <time.h>
struct InputFileJSONStruct JSON;

void readInfoFile(const std::string& infoFileName, int& data_number, std::string& datatype, hxy::my_int3& dimension, hxy::my_double3& space,
	std::vector<std::string>& file_list)
{
	file_list.clear();

	std::ifstream inforFile(infoFileName);

	inforFile >> data_number;
	inforFile >> datatype;
	inforFile >> dimension.x >> dimension.y >> dimension.z;
	//Debug 20190520 增加sapce接口
	inforFile >> space.x >> space.y >> space.z;
	const std::string filePath = infoFileName.substr(0, infoFileName.find_last_of('/') == -1 ?
		infoFileName.find_last_of('\\') + 1 : infoFileName.find_last_of('/') + 1);
	std::cout << (filePath.c_str()) << std::endl;
	for (auto i = 0; i < data_number; i++)
	{
		std::string rawFileName;
		inforFile >> rawFileName;
		std::string volumePath = filePath + rawFileName;
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

int getWordMapFromFile(const std::string & file_name, std::map<std::string, std::vector<float>>& word_map)
{
	//string file_name = "./workspace/word.emb";
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

	return vector_dimension;
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


void getLabelMapFromFile(const std::string & file_name, std::map<std::string, std::vector<float>>& label_map)
{
	//string file_name = "./workspace/all_node.emb";
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
		if (!word.empty() && !isNum(word))
		{
			label_map[word] = word_vector;
		}
		//std::cout << word << std::endl;
	}
	std::cout << "all_node.emb has been loaded." << std::endl;

}




// struct InputFileJSONStruct : public vm::json::Serializable<InputFileJSONStruct>
// {
// 	VM_JSON_FIELD(int, window_size);
// 	VM_JSON_FIELD(int, volume_index);
// 	VM_JSON_FIELD(double, threshold);
//
//
// 	VM_JSON_FIELD(std::string, vifo_file_name);
// 	VM_JSON_FIELD(std::string, file_prefix);
// 	VM_JSON_FIELD(std::string, word_file_name);
// 	VM_JSON_FIELD(std::string, label_file_name);
// 	VM_JSON_FIELD(std::string, segmentation_file_name);
// 	
// }JSON;


int main(int argc, char* argv[])
{
	
	std::string json_file_path;
	double threshold = -1.0;

	if (argc <= 1)
	{
		std::cout << "Using default json configure file." << std::endl;
		json_file_path = R"(E:\project\science_project\VoxelClassification\x64\Release\configuration_json\jet_mixfrac_0051_data.json)";
	}
	else
	{
		// create a parser
		cmdline::parser a;
		a.add<std::string>("configure_json", 'c', "configure json file for segmentation", true, "");
		a.add<double>("threshold", 't', "threshold for filter large different voxels", 
			false, -0xfffff, cmdline::range<double>(-0xfffff,100));
		
		a.parse_check(argc, argv);

		json_file_path = a.get<std::string>("configure_json");
		threshold = a.get<double>("threshold");
	}

	try
	{
		auto time_before = clock();

		std::ifstream json_file(json_file_path);
		json_file >> JSON;
		vm::json::Writer writer;
		writer.write(std::cout, JSON);


		std::map<std::string, std::vector<float>> word_map;
		std::map<std::string, std::vector<float>> label_map;

		int data_number;
		std::string datatype;
		hxy::my_int3 dimension;
		hxy::my_double3 space;
		std::vector<std::string> file_list;


		std::string infoFile = JSON.data_path.vifo_file;
		std::string file_prefix = JSON.data_path.file_prefix;
		std::string word_file_name = file_prefix + JSON.file_name.word_embedding_file;
		std::string label_file_name = file_prefix + JSON.file_name.all_embedding_file;
		int window_size = JSON.segmenation_process.segmentation_window_size;
		int volume_index = JSON.data_path.volume_index;
		std::string segmentation_file_name = file_prefix + JSON.segmenation_process.segmentation_file_name;
		int is_gpu_used = JSON.segmenation_process.segmentation_gpu;

		if (threshold == -0xfffff) threshold = JSON.segmenation_process.segmentation_threshold;


		readInfoFile(infoFile, data_number, datatype, dimension, space, file_list);
		SourceVolume source_volume(file_list, dimension.x, dimension.y, dimension.z, datatype);
		source_volume.loadVolume();
		source_volume.loadRegularVolume();
		auto volume_data = *source_volume.getRegularVolume(volume_index);


		int vector_size = getWordMapFromFile(word_file_name, word_map);
		getLabelMapFromFile(label_file_name, label_map);

		std::vector<int> segmentation_vector(dimension.x*dimension.y*dimension.z, 0);

		VolumeSegmentation volume_segmentation;
		
		if (is_gpu_used)
			volume_segmentation.segemationGPU(volume_data, dimension.x, dimension.y, dimension.z,
				word_map, label_map, window_size, vector_size, segmentation_vector, threshold);
		else
			volume_segmentation.segemation(volume_data, dimension.x, dimension.y, dimension.z,
				word_map, label_map, window_size, vector_size, segmentation_vector, threshold);

		volume_segmentation.saveSegmentation(segmentation_vector, segmentation_file_name);

		std::cout << "Segmentation process is over." << std::endl;

		auto time_end = clock();
		std::cout << "Calculating time for similarity map: \t" << (time_end - time_before) / 1000.0 << std::endl;

	}
	catch (std::exception & e)
	{
		vm::println("{}", e.what());
	}
	//getchar();
}
