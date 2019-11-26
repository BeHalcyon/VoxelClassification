#include "VolumeSegmentation.h"
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include "omp.h"
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file = nullptr, int line = 0, bool abort = true)
{
	if (code != cudaSuccess)
	{
		fprintf(stderr, "GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
		//if (abort) exit(code);
	}
}


extern "C" void kernelSegmentation(int block_number, int thread_number, unsigned char* d_volume_data, const int sz, const int xDim, const int yDim, const int zDim,
	const float threshold, const int window_size,
	float* word_vector, size_t word_vecotr_size,
	int word_number, float* label_vector, size_t label_vector_size, size_t label_number, const int vector_size, int* segmentation_data);

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


#pragma omp parallel for
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

#pragma omp parallel for
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

void VolumeSegmentation::segemationGPU(const std::vector<unsigned char>& volume_data, const int& width, const int& height,
	const int& depth, std::map<std::string, std::vector<float>>&word_map,
	std::map<std::string, std::vector<float>>&label_map, const int & window_size, const int & vector_size, std::vector<int>& segementaion_data,
	const double & threshold)
{
	if (window_size == 3 || window_size == 1||window_size==5)
	{
		const auto sz = width * height * depth;
		const auto& xDim = width;
		const auto& yDim = height;
		const auto& zDim = depth;

		auto label_number = label_map.size();
		std::vector<std::string> label_id_vector;
		for (auto& based : label_map)
		{
			label_id_vector.push_back(based.first);
		}
		//设定cuda设备
		cudaDeviceProp deviceProp;
		cudaGetDeviceProperties(&deviceProp, 0);
		printf("\nDevice%d:\"%s\"\n", 0, deviceProp.name);
		cudaSetDevice(0);

		//申请并拷贝体数据
		unsigned char * d_volume_data;
		gpuAssert(cudaMalloc((void**)&d_volume_data, sz * sizeof(unsigned char)));
		gpuAssert(cudaMemcpy(d_volume_data, volume_data.data(), sz * sizeof(unsigned char), cudaMemcpyHostToDevice));
		//创建设备的word和label向量
		std::vector<float> word_vector;
		int min_value = 0xffffff, max_value = -0xffffff;
		for (auto& i : word_map)
		{
			min_value = std::min(min_value, atoi(i.first.c_str()));
			max_value = std::max(max_value, atoi(i.first.c_str()));
		}
		word_vector.resize(vector_size*(max_value - min_value + 1));
		for (auto& i : word_map)
		{
			auto index = atoi(i.first.c_str());
			for (auto j = 0; j < vector_size; j++)
				word_vector[index*vector_size + j] = i.second[j];
		}
		float*	d_word_vector = nullptr;
		gpuAssert(cudaMalloc((void**)&d_word_vector, word_vector.size() * sizeof(float)));
		gpuAssert(cudaMemcpy(d_word_vector, word_vector.data(), word_vector.size() * sizeof(float), cudaMemcpyHostToDevice));

		float* d_label_vector = nullptr;
		std::vector<float> label_vector(label_map.size()*vector_size);
		int idx = 0;
		for (auto& based : label_map)
		{
			for (auto j = 0; j < vector_size; j++)
				label_vector[idx*vector_size + j] = based.second[j];
			idx++;
		}
		gpuAssert(cudaMalloc((void**)&d_label_vector, label_vector.size() * sizeof(float)));
		gpuAssert(cudaMemcpy(d_label_vector, label_vector.data(), label_vector.size() * sizeof(float), cudaMemcpyHostToDevice));

		int thread_number = 512;
		int block_number = sz / thread_number + 1;

		int* d_segmentation_data;
		gpuAssert(cudaMalloc((void**)&d_segmentation_data, sz * sizeof(int)));


		kernelSegmentation(block_number, thread_number, d_volume_data, sz, xDim, yDim, zDim, threshold, window_size,
			d_word_vector, word_vector.size(), max_value - min_value + 1,
			d_label_vector, label_vector.size(), label_map.size(), vector_size, d_segmentation_data);

		cudaMemcpy(segementaion_data.data(), d_segmentation_data, sz * sizeof(int), cudaMemcpyDeviceToHost);

		cudaFree(d_volume_data);
		cudaFree(d_label_vector);
		cudaFree(d_word_vector);
		cudaFree(d_segmentation_data);
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