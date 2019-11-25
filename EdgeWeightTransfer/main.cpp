#include <iostream>
#include <string>
#include "../VoxelClassification/json_struct.h"
#include <fstream>
#include <map>
#include <set>
#include "../VoxelClassification/cmdline.h"
#include "time.h"

using namespace std;

struct InputFileJSONStruct JSON;

struct ww_net
{
	int a, b;
	bool operator < (const ww_net & other) const
	{
		return this->a == other.a ? this->b < other.b : this->a < other.a;
	}
};

struct lw_net
{
	std::string a, b;
	double w;
};

void transWWNet(const std::string & ww_net_origin_file, const std::string & ww_net_file, const int & edge_type, std::vector<std::vector<double>>& ww_vec)
{
	std::map<ww_net, double> ww_map;
	FILE *fi = fopen(ww_net_origin_file.c_str(), "rb");
	char tp;
	int u, v;
	double w;

	int min_value = 0xffffff, max_value = -0xffffff;
	while (fscanf(fi, "%d %d %lf %c", &u, &v, &w, &tp) == 4)
	{
		ww_map[{u, v}] = w;
		min_value = std::min(min_value, u);
		max_value = std::max(max_value, u);
	}
	fclose(fi);

	ww_vec.resize(max_value - min_value + 1);
	for (auto i = 0; i < ww_vec.size(); i++) ww_vec[i].resize(ww_vec.size());
	for (auto & iter : ww_map)
	{
		ww_vec[iter.first.a][iter.first.b] = iter.second;
	}

	if (edge_type == 0)
	{
		return;
	}
	//基于权重的归一化
	else if (edge_type == 1)
	{
		for (auto& i : ww_vec)
		{
			double sum = 0.0;
			for (auto j : i)
			{
				sum += j;
			}
			if (sum == 0.0) continue;
			for (auto& j : i)
			{
				j /= sum;
			}
		}
	}
	//权重均为1
	else if (edge_type == 2)
	{
		for (auto& i : ww_vec)
		{
			double sum = 0.0;
			for (auto& j : i)
			{
				if (j > 0) j = 1;
			}
		}
	}
	//log归一化
	else if (edge_type == 3)
	{
		for (auto& i : ww_vec)
		{
			double sum = 0.0;
			for (auto j : i)
			{
				if (j > 1)
					sum += log(j);
			}
			if (sum == 0.0) continue;
			for (auto& j : i)
			{
				if (j > 1)
					j = log(j) / sum;
				else j = 0;
			}
		}
	}
	else
	{
		printf("Edge type for ww net is invalid.");
		exit(-1);
	}


	FILE *fo = fopen(ww_net_file.c_str(), "wb");
	auto cnt = 0;
	for (auto i = 0; i < ww_vec.size(); i++)
	{
		for (auto j = 0; j < ww_vec[i].size(); j++)
		{
			if (ww_vec[i][j] > 0)
			{
				fprintf(fo, "%s\t%s\t%lf\t\tw\n",
					std::to_string(i).c_str(), std::to_string(j).c_str(), ww_vec[i][j] * 1.0f);
			}
			cnt++;
		}
	}
	//printf("\n");
	fclose(fo);

	printf("The ww.net file has been transferred.\n");

}

void transLWNet(const std::string & lw_net_origin_file, const std::string & lw_net_file, const int & edge_type, std::vector<lw_net>& lw_vec)
{
	FILE *fi = fopen(lw_net_origin_file.c_str(), "rb");
	char word1[100], word2[100], tp;
	int u, v;
	double w;


	while (fscanf(fi, "%s %s %lf %c", word1, word2, &w, &tp) == 4)
	{
		lw_vec.push_back({ word1, word2,w });
		//label_set.insert(word1);
	}
	fclose(fi);

	if (edge_type == 0)
	{
		return;
	}
	//所有权重均为1
	else if (edge_type == 1)
	{

		FILE *fo = fopen(lw_net_file.c_str(), "wb");
		auto cnt = 0;

		for (auto& i : lw_vec)
		{
			fprintf(fo, "%s\t%s\t%d\t\tl\n", i.a.c_str(), i.b.c_str(), 1);
		}

		printf("\n");
		fclose(fo);
		printf("The ww.net file has been saved.\n");
	}
	else
	{
		printf("Edge type for lw net is invalid.\n");
		exit(-1);
	}

	printf("The text.hid file has been transferred.\n");
}


void transTextHin(const std::string & text_hin_origin_file, const std::string & text_hin_file, const int & ww_edge_type, const int & lw_edge_type,
	std::vector<std::vector<double>>& ww_vec, std::vector<lw_net>& lw_map)
{

	FILE *fo = fopen(text_hin_file.c_str(), "wb");
	//auto cnt = 0;
	for (auto i = 0; i < ww_vec.size(); i++)
	{
		for (auto j = 0; j < ww_vec[i].size(); j++)
		{
			if (ww_vec[i][j] > 0)
			{
				fprintf(fo, "%s\t%s\t%lf\t\tw\n",
					std::to_string(i).c_str(), std::to_string(j).c_str(), ww_vec[i][j] * 1.0f);
			}
			//cnt++;
		}
	}
	//printf("\n");

	if(lw_edge_type==0)
	{
		for (auto& i : lw_map)
		{
			fprintf(fo, "%s\t%s\t%lf\t\tl\n", i.a.c_str(), i.b.c_str(), i.w);
		}
	}
	else if(lw_edge_type==1)
	{
		for (auto& i : lw_map)
		{
			fprintf(fo, "%s\t%s\t%d\t\tl\n", i.a.c_str(), i.b.c_str(), 1);
		}
	}
	else
	{
		
	}
	
	//printf("\n");
	fclose(fo);
	printf("The text.hin file has been transferred.\n");
}

int main(int argc, char* argv[])
{


	std::string configure_json_file;
	std::string output_path_json_file;
	if (argc <= 1)
	{
		std::cout << "Using default json configure file." << std::endl;
		configure_json_file = R"(E:\project\science_project\VoxelClassification\x64\Release\configuration_json\TOOTH_8bit_128_128_160_data.json)";
	}
	else
	{
		cmdline::parser a;
		a.add<std::string>("configure_json", 'c', "configure file for transferring edge weight", true, "");
		a.parse_check(argc, argv);

		configure_json_file = a.get<std::string>("configure_json");
	}
	try
	{
		auto time_before = clock();
		std::ifstream json_file(configure_json_file);
		json_file >> JSON;
		vm::json::Writer writer;
		writer.write(std::cout, JSON);
		
		std::string file_prefix = JSON.data_path.file_prefix;
		std::string ww_net_file = file_prefix + JSON.file_name.ww_net_file;
		std::string lw_net_file = file_prefix + JSON.file_name.lw_net_file;
		std::string text_hin_file = file_prefix + JSON.file_name.text_hin_file;

		std::string ww_net_origin_file = file_prefix + JSON.file_name.ww_net_origin_file;
		std::string lw_net_origin_file = file_prefix + JSON.file_name.lw_net_origin_file;
		std::string text_hin_origin_file = file_prefix + JSON.file_name.text_hin_origin_file;
		
		int ww_edge_type = JSON.data_prepare.ww_edge_weight_type;
		int lw_edge_type = JSON.data_prepare.lw_edge_weight_type;
		
		
		std::vector<std::vector<double>> ww_vec;
		std::vector<lw_net> lw_vec;
		transWWNet(ww_net_origin_file, ww_net_file, ww_edge_type, ww_vec);
		transLWNet(lw_net_origin_file, lw_net_file, lw_edge_type, lw_vec);
		transTextHin(text_hin_origin_file, text_hin_file, ww_edge_type, lw_edge_type, ww_vec, lw_vec);

		auto time_end = clock();

		std::cout << "Calculating time for edge weight transfer: \t" << (time_end - time_before) / 1000.0 << std::endl;
		//getchar();

	}
	catch (std::exception & e)
	{
		vm::println("{}", e.what());
	}

}