#include <iostream>
#include "VMUtils/include/VMUtils/json_binding.hpp"
#include "VMUtils/include/VMUtils/cmdline.hpp"
#include <string>
#include <fstream>
#include "TrainNetwork.h"


struct InputFileJSONStruct : public vm::json::Serializable<InputFileJSONStruct>
{
	VM_JSON_FIELD(int, vector_size);
	VM_JSON_FIELD(int, sample_number);
	VM_JSON_FIELD(double, alpha);
	VM_JSON_FIELD(int, thread_number);
	//VM_JSON_FIELD(int, window_size);
	VM_JSON_FIELD(int, negative_number);

	VM_JSON_FIELD(std::string, file_prefix);

	VM_JSON_FIELD(std::string, input_text_node);
	VM_JSON_FIELD(std::string, input_word_node);
	VM_JSON_FIELD(std::string, input_text_hin);
	VM_JSON_FIELD(std::string, output_word_embedding);
	VM_JSON_FIELD(std::string, output_node_embedding);

}JSON;



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
		a.add<std::string>("train_file", 't', "configure json file for training", true, "");
		a.parse_check(argc, argv);

		json_file_path = a.get<std::string>("train_file");
	}

	std::ifstream json_file(json_file_path);
	try
	{
		json_file >> JSON;
		vm::json::Writer writer;
		writer.write(std::cout, JSON);


		const auto vector_size = JSON.vector_size;
		const auto sample_number = JSON.sample_number*1000000;
		const auto alpha = JSON.alpha;
		int negative_number = JSON.negative_number;
		int num_threads = JSON.thread_number;


		std::string file_prefix = JSON.file_prefix;
		std::string input_text_node = file_prefix + JSON.input_text_node;
		std::string input_word_node = file_prefix + JSON.input_word_node;
		std::string input_text_hin = file_prefix + JSON.input_text_hin;
		std::string output_word_embedding = file_prefix + JSON.output_word_embedding;
		std::string output_node_embedding = file_prefix + JSON.output_node_embedding;
		

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

	getchar();
	return 0;

}