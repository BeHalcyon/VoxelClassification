#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
//#include "VMUtils/include/VMUtils/json_binding.hpp"
//#include "VMUtils/include/VMUtils/cmdline.hpp"
#include "../VoxelClassification/json_struct.h"
#include "../VoxelClassification/VMUtils/include/VMUtils/cmdline.hpp"
#include <time.h>
#include <stdio.h>
#include "python.h"

struct WordVectorJsonStruct : public vm::json::Serializable<WordVectorJsonStruct>
{
	VM_JSON_FIELD(std::vector<std::string>, word_name_vector);
	VM_JSON_FIELD(std::vector<std::vector<float>>, word_vector);
	VM_JSON_FIELD(std::vector<std::string>, label_name_vector);
	VM_JSON_FIELD(std::vector<std::vector<int>>, label_vector);
}WORD_VECTOR_JSON;

struct InputFileJSONStruct JSON;

std::map<std::string, int> word_hash_map;
std::map<std::string, int> label_hash_map;

void readWordVector(const std::string & input_word_emb_file, std::vector<std::string>& word_vec, std::vector<std::vector<float>>& vec, int & vector_size)
{
	FILE *fi = fopen(input_word_emb_file.c_str(), "rb");
	char ch, word[100000];
	float f_num;
	int size;

	fscanf(fi, "%d %d", &size, &vector_size);

	//vec = (real *)malloc(size * vector_size * sizeof(real));

	vec.resize(size);
	word_vec.resize(size);

	for (auto i = 0; i < vec.size(); i++)
	{
		vec[i].resize(vector_size, 0);
	}
	//printf("test0\n");

	for (long long k = 0; k != size; k++)
	{
		fscanf(fi, "%s", word);
		word_vec[k] = (word);
		word_hash_map[word] = k;

		auto intword = atoi(word);
		//printf("%s %d\n", word, intword);
		//ch = fgetc(fi);
		//ch++;
		//AddVertex(word);
		for (int c = 0; c != vector_size; c++)
		{
			fscanf(fi, "%f", &f_num);
			//fread(&f_num, sizeof(real), 1, fi);
			vec[k][c] = float(f_num);
			//vec[c + k * vector_size] = (real)f_num;
		}
	}
	fclose(fi);
	printf("Number of words: %d\n", size);
	printf("Vector dimension: %d\n", vector_size);
}

void readLabelName(const std::string& label_node_file, std::vector<std::string> & label_name_vector)
{
	FILE *fi = fopen(label_node_file.c_str(), "rb");
	char word[100];
	int i = 0;
	while (1)
	{
		if (fscanf(fi, "%s", word) != 1) break;
		label_name_vector.push_back(word);
		label_hash_map[word] = i;
		i++;
	}
	fclose(fi);
}

bool isNum(char* str)
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
void calcBitLabelVector(const std::string& label_word_net_file, 
	const std::vector<std::string>& word_vec, const std::vector<std::vector<float>>& vec,
	const std::vector<std::string> & label_name_vector, std::vector<std::vector<int>> & label_bit_vector)
{
	FILE *fi = fopen(label_word_net_file.c_str(), "rb");
	char label[100], word[100];
	double weight;
	char c;
	label_bit_vector.resize(word_vec.size());
	for (auto i = 0; i < word_vec.size(); i++) label_bit_vector[i].resize(label_name_vector.size(),0);
	while (1)
	{
		if (fscanf(fi, "%s %s %lf %c", label, word, &weight, &c) != 4) break;
		std::string label_string = label;
		std::string word_string = word;
		auto label_index = label_hash_map[label_string];
		auto word_index = word_hash_map[word_string];
		label_bit_vector[word_index][label_index] = 1;
	}
	fclose(fi);
}


void writeVectorToJson(const std::string& cs, const std::vector<std::basic_string<char>>& cses, const std::vector<std::vector<float>>& vec, 
	const std::vector<std::basic_string<char>>& vector, const std::vector<std::vector<int>>& ises)
{

	std::vector<std::string> word_vector;
	std::vector<int> label_vector;

	std::ofstream output_json(cs);
	WORD_VECTOR_JSON.set_word_name_vector(cses);
	WORD_VECTOR_JSON.set_word_vector(vec);
	WORD_VECTOR_JSON.set_label_name_vector(vector);
	WORD_VECTOR_JSON.set_label_vector(ises);
	vm::json::Writer writer;
	writer.write(output_json, WORD_VECTOR_JSON);

	std::cout << "Embedding file has been transferred into json." << std::endl;
}

void loadPythonModule(const std::string& json_file_name)
{
	// 初始化，载入python的扩展模块
	Py_Initialize();
	if (!Py_IsInitialized()) {
		std::cout << "Python init failed!" << std::endl;
		return;
	}

	//将Python工作路径切换到待调用模块所在目录，一定要保证路径名的正确性
	std::string path = R"(E:/project/science_project/VoxelClassification/CNN/)";
	std::string chdir_cmd = "sys.path.append('" + path+"')";
	
	//const char* cstr_cmd = chdir_cmd.c_str();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString(chdir_cmd.c_str());




	// 加载模块
	//PyObject* moduleName = PyString_FromString("valuepredict"); //模块名，不是文件名
	PyObject* pModule = PyImport_ImportModule("valuepredict");
	if (!pModule) // 加载模块失败
	{
		std::cout << "[ERROR] Python get module failed." << std::endl;
		return;
	}
	std::cout << "[INFO] Python get module succeed." << std::endl;
	
	// 加载函数
	PyObject* pv = PyObject_GetAttrString(pModule, "doValuePrediction");
	if (!pv || !PyCallable_Check(pv))  // 验证是否加载成功
	{
		std::cout << "[ERROR] Can't find funftion (doValuePrediction)" << std::endl;
		return;
	}
	PyObject* args = Py_BuildValue("s", json_file_name);
	PyObject* pRet = PyObject_CallObject(pv, args);

	std::cout << "[INFO] Get function (doValuePrediction) succeed." << std::endl;


	// 获取参数
	if (pRet)  // 验证是否调用成功
	{
		
		std::cout << "result:";
	}

	Py_Finalize();
}

int main(int argc, char* argv[])
{

	std::string json_file_path;

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
		a.parse_check(argc, argv);
		json_file_path = a.get<std::string>("configure_json");
	}

	try
	{
		auto time_before = clock();

		std::ifstream json_file(json_file_path);
		json_file >> JSON;
		vm::json::Writer writer;
		writer.write(std::cout, JSON);

		std::string infoFile = JSON.data_path.vifo_file;
		std::string file_prefix = JSON.data_path.file_prefix;
		std::string word_file_name = file_prefix + JSON.file_name.word_embedding_file;
		std::string label_file_name = file_prefix + JSON.file_name.all_embedding_file;
		int window_size = JSON.segmenation_process.segmentation_window_size;
		int volume_index = JSON.data_path.volume_index;
		std::string segmentation_file_name = file_prefix + JSON.segmenation_process.segmentation_file_name;
		int is_gpu_used = JSON.segmenation_process.segmentation_gpu;

		std::string output_word_json_file = file_prefix + JSON.value_prediction.word_embedding_json;
		std::string input_word_emb_file = file_prefix + JSON.file_name.word_embedding_file;

		std::vector<std::vector<float>> vec;
		std::vector<std::string> word_vec;
		int vector_size;
		readWordVector(input_word_emb_file, word_vec, vec, vector_size);

		std::string label_node_file = file_prefix + JSON.file_name.label_node_file;
		std::vector<std::string> label_name_vector;
		readLabelName(label_node_file, label_name_vector);

		std::string label_word_net_file = file_prefix + JSON.file_name.lw_net_origin_file;
		std::vector<std::vector<int>> label_bit_vector;
		calcBitLabelVector(label_word_net_file, word_vec, vec, label_name_vector, label_bit_vector);


		writeVectorToJson(output_word_json_file, word_vec, vec, label_name_vector, label_bit_vector);


		std::cout << "Transfer word embedding to json file process is over." << std::endl;


		//loadPythonModule(output_word_json_file);


		auto time_end = clock();
		std::cout << "Calculating time for similarity map: \t" << (time_end - time_before) / 1000.0 << std::endl;

	}
	catch (std::exception & e)
	{
		vm::println("{}", e.what());
	}
	//getchar();
}
