#include "VolumeSegmentation.h"
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>

VolumeSegmentation::VolumeSegmentation()
{
}

void VolumeSegmentation::segemation(const std::vector<unsigned char>& volume_data, const int& width, const int& height,
	const int& depth, std::map<std::string, std::vector<float>>&word_map,
	std::map<std::string, std::vector<float>>&label_map, const int & window_size, const int & vector_size, std::vector<int>& segementaion_data,
	const double & threshold)
{
	const auto sz = width * height * depth;
	

	const auto& xDim = width;
	const auto& yDim = height;
	const auto& zDim = depth;

	auto label_number = label_map.size();
	std::vector<std::string> label_id_vector;
	for(auto& based: label_map)
	{
		label_id_vector.push_back(based.first);
	}

	if (window_size == 3)
	{
		const int dx26[26] = { -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1 };
		const int dy26[26] = { -1, -1, -1,  0,  0,  0,  1,  1,  1, -1, -1, -1,  0,  0,  1,  1,  1, -1, -1, -1,  0,  0,  0,  1,  1,  1 };
		const int dz26[26] = { -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1 };

		for (auto index = 0; index < sz; index++)
		{
			std::vector<float> average_vector(vector_size, 0);

			const int oz = index / (xDim*yDim);
			const int ox = index % xDim;
			const int oy = (index % (xDim*yDim)) / xDim;
			int a = 10;
			//std::stringstream ss;
			//ss << a;
			//std::string str = ss.str();
			auto center_word = volume_data[index];
			const auto center_word_string = std::to_string(center_word);
			auto& word_vector = word_map[center_word_string];

			for (auto j = 0; j < vector_size; j++) average_vector[j] += word_vector[j];

			auto cnt = 1;
			for (auto i = 0; i < 26; i++)
			{
				int nx = ox + dx26[i];//new x
				int ny = oy + dy26[i];//new y
				int nz = oz + dz26[i];//new z

				if (nx >= 0 && nx < xDim && ny >= 0 && ny < yDim && nz >= 0 && nz < zDim)
				{
					int nind = nz * xDim*yDim + ny * xDim + nx;

					cnt++;
					//const auto center_word_string = std::to_string(volume_data[center_word]);
					auto& context_vector = word_map[std::to_string(volume_data[nind])];

					for (auto j = 0; j < vector_size; j++) average_vector[j] += context_vector[j];
				}
			}

			for(auto j=0;j<vector_size;j++) average_vector[j] /= cnt;

			//判断该数组与label数组的关系
			double max_value = -0xffffff;
			
			cnt = 0;
			
			for (auto n = 0; n < label_id_vector.size(); n++)
			{
				const auto & label_name = label_id_vector[n];
				auto& label_vector = label_map[label_name];
				auto buf = 0.0f;
				for(auto m=0;m<vector_size;m++)
				{
					buf += average_vector[m] * label_vector[m];
				}
				if(buf > max_value)
				{
					max_value = buf;
					cnt = n+1;
				}
			}
			if(max_value < threshold)
			{
				cnt = 0;
			}
			segementaion_data[index] = 10*cnt;
		}
	}
	else if (window_size == 1)
	{
		const int dx6[6] = { -1,  1,  0,  0,  0,  0 };
		const int dy6[6] = { 0,  0, -1,  1,  0,  0 };
		const int dz6[6] = { 0,  0,  0,  0, -1,  1 };


		for (auto index = 0; index < sz; index++)
		{
			std::vector<float> average_vector(vector_size, 0);

			const int oz = index / (xDim*yDim);
			const int ox = index % xDim;
			const int oy = (index % (xDim*yDim)) / xDim;
			int a = 10;
			//std::stringstream ss;
			//ss << a;
			//std::string str = ss.str();
			auto center_word = volume_data[index];
			const auto center_word_string = std::to_string(center_word);
			auto& word_vector = word_map[center_word_string];

			for (auto j = 0; j < vector_size; j++) average_vector[j] += word_vector[j];

			auto cnt = 1;
			for (auto i = 0; i < 6; i++)
			{
				int nx = ox + dx6[i];//new x
				int ny = oy + dy6[i];//new y
				int nz = oz + dz6[i];//new z

				if (nx >= 0 && nx < xDim && ny >= 0 && ny < yDim && nz >= 0 && nz < zDim)
				{
					int nind = nz * xDim*yDim + ny * xDim + nx;

					cnt++;
					//const auto center_word_string = std::to_string(volume_data[center_word]);
					auto& context_vector = word_map[std::to_string(volume_data[nind])];

					for (auto j = 0; j < vector_size; j++) average_vector[j] += context_vector[j];
				}
			}

			for (auto j = 0; j < vector_size; j++) average_vector[j] /= cnt;

			//判断该数组与label数组的关系
			double max_value = -0xffffff;

			//cnt为0是背景
			cnt = 0;

			for (auto n = 0; n < label_id_vector.size(); n++)
			{
				const auto & label_name = label_id_vector[n];
				auto& label_vector = label_map[label_name];
				auto buf = 0.0f;
				for (auto m = 0; m < vector_size; m++)
				{
					buf += average_vector[m] * label_vector[m];
				}
				if (buf > max_value)
				{
					max_value = buf;
					cnt = n+1;
				}
			}
			if (max_value < threshold)
			{
				cnt = 0;
			}

			segementaion_data[index] = 10 * cnt;

		}
	}
	else
	{
		printf("Error : Window size is not available.\n");
		exit(-1);
	}


}


VolumeSegmentation::~VolumeSegmentation()
{
}

void VolumeSegmentation::saveSegmentation(
	const std::vector<int>& segmentation_volume, const std::string & file_name)
{

	int sz = segmentation_volume.size();

	std::ofstream outfile(file_name, std::ios::binary);
	for (int i = 0; i < sz; i++)
	{
		outfile.write((char*)(&segmentation_volume[i]), sizeof(int));
	}
	outfile.close();
	std::cout << "Segmentation based label vector and word vector has been saved." << std::endl;

}