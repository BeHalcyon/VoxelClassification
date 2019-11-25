//#include "VMUtils/include/VMUtils/cmdline.hpp"
//#include "VMUtils/include/VMUtils/json_binding.hpp"
#include <iostream>
#include <fstream>
#include "Vector2Image.h"
#include "../VoxelClassification/json_struct.h"
#include "../VoxelClassification/VMUtils/include/VMUtils/cmdline.hpp"
#include <time.h>
// struct InputFileJSONStruct : public vm::json::Serializable<InputFileJSONStruct>
// {
// 	VM_JSON_FIELD(std::string, word_emb_file);
// 	VM_JSON_FIELD(std::string, similarity_file);
// 	VM_JSON_FIELD(std::string, file_prefix);
// 	
// }JSON;

struct InputFileJSONStruct JSON;

int main(int argc, char* argv[])
{
	std::string word_emb_file;
	std::string similarity_file;
	std::string configure_file;
	std::string file_prefix;
	if (argc <= 1)
	{
		std::cout << "Using default json configure file." << std::endl;
		//word_emb_file = R"(J:\PTE buffer\TOOTH_8bit_128_128_160_word.emb)";
		//similarity_file = R"(J:\PTE buffer\TOOTH_8bit_128_128_160_similarity_map.png)";
		configure_file = R"(E:\project\science_project\VoxelClassification\x64\Release\configuration_json\TOOTH_8bit_128_128_160_similarity_map.json)";
	}
	else
	{
		cmdline::parser a;
		a.add<std::string>("configure_json", 'c', "configure file for word vector file and output similarity map", true, "");
		//a.add<std::string>("similarity_map", 's', "output similarity map file path", true, "");
		a.parse_check(argc, argv);

		configure_file = a.get<std::string>("configure_json");
	}
	try
	{
		auto time_before = clock();

		std::ifstream json_file(configure_file);
		json_file >> JSON;
		vm::json::Writer writer;
		writer.write(std::cout, JSON);
		file_prefix = JSON.data_path.file_prefix;
		word_emb_file = file_prefix + JSON.file_name.word_embedding_file;
		similarity_file = file_prefix + JSON.similarity_process.similarity_file;

		Vector2Image vector2image(word_emb_file);
		vector2image.process();
		vector2image.savePNG(similarity_file);
		auto time_end = clock();
		std::cout << "Calculating time for similarity map: \t" << (time_end - time_before) / 1000.0 << std::endl;

	}
	catch (std::exception& e)
	{
		vm::println("{}", e.what());
	}
	getchar();
}