#include <iostream>
//#include "VMUtils/include/VMUtils/json_binding.hpp"
//#include "VMUtils/include/VMUtils/cmdline.hpp"
#include <string>
#include <fstream>
#include "TrainNetwork.h"
#include "../VoxelClassification/json_struct.h"
#include "../VoxelClassification/VMUtils/include/VMUtils/cmdline.hpp"
struct InputFileJSONStruct JSON;

// struct InputFileJSONStruct : public vm::json::Serializable<InputFileJSONStruct>
// {
// 	VM_JSON_FIELD(int, vector_size);
// 	VM_JSON_FIELD(int, sample_number);
// 	VM_JSON_FIELD(double, alpha);
// 	VM_JSON_FIELD(int, thread_number);
// 	//VM_JSON_FIELD(int, window_size);
// 	VM_JSON_FIELD(int, negative_number);
//
// 	VM_JSON_FIELD(std::string, file_prefix);
//
// 	VM_JSON_FIELD(std::string, input_text_node);
// 	VM_JSON_FIELD(std::string, input_word_node);
// 	VM_JSON_FIELD(std::string, input_text_hin);
// 	VM_JSON_FIELD(std::string, output_word_embedding);
// 	VM_JSON_FIELD(std::string, output_node_embedding);
//
// }JSON;



int main(int argc, char* argv[])
{
	std::string json_file_path;
	if(argc <=1)
	{
		std::cout << "Using default json configure file." << std::endl;

		json_file_path = R"(E:\project\science_project\VoxelClassification\VoxelClassification\workspace\TOOTH_8bit_128_128_160_train.json)";
	}
	else
	{
		// create a parser
		cmdline::parser a;
		a.add<std::string>("configure_json", 'c', "configure json file for training", true, "");
		a.parse_check(argc, argv);

		json_file_path = a.get<std::string>("configure_json");
	}

	std::ifstream json_file(json_file_path);
	try
	{
		json_file >> JSON;
		vm::json::Writer writer;
		writer.write(std::cout, JSON);


		const auto vector_size = JSON.train_process.train_vector_size;
		const auto sample_number = JSON.train_process.train_sample_number*1000000;
		const auto alpha = JSON.train_process.train_alpha;
		int negative_number = JSON.train_process.train_negative_number;
		int num_threads = JSON.train_process.train_thread_number;


		std::string file_prefix = JSON.data_path.file_prefix;
		std::string input_text_node = file_prefix + JSON.file_name.text_node_file;
		std::string input_word_node = file_prefix + JSON.file_name.word_node_file;
		std::string input_text_hin = file_prefix + JSON.file_name.text_hin_file;
		std::string output_word_embedding = file_prefix + JSON.file_name.word_embedding_file;
		std::string output_node_embedding = file_prefix + JSON.file_name.all_embedding_file;
		

		auto train_network = std::make_unique<TrainNetwork>(input_text_node, input_word_node, input_text_hin, vector_size,
			negative_number, sample_number, alpha, num_threads);

		train_network->TrainModel();
		train_network->saveWordEmbedding(output_word_embedding);
		train_network->saveLabelEmbedding(output_node_embedding);

		std::cout << "Train process is over." << std::endl;

	}
	catch (std::exception & e)
	{
		vm::println("{}", e.what());
	}

	//getchar();
	return 0;

}