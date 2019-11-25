#include <cuda_runtime_api.h>
#include <helper_cuda.h>
#include <device_launch_parameters.h>


const int x26[26] = { -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1 };
const int y26[26] = { -1, -1, -1,  0,  0,  0,  1,  1,  1, -1, -1, -1,  0,  0,  1,  1,  1, -1, -1, -1,  0,  0,  0,  1,  1,  1 };
const int z26[26] = { -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1 };

const int x6[6] = { -1,  1,  0,  0,  0,  0 };
const int y6[6] = { 0,  0, -1,  1,  0,  0 };
const int z6[6] = { 0,  0,  0,  0, -1,  1 };


__constant__ int dx26[26];
__constant__ int dy26[26];
__constant__ int dz26[26];

__constant__ int dx6[6];
__constant__ int dy6[6];
__constant__ int dz6[6];


__global__
void kernel_segmentation(unsigned char* d_volume_data, const int sz, const int xDim, const int yDim, const int zDim, const float threshold, const int window_size,
	float* word_vec, size_t word_vecotr_size, int word_number, 
	float* label_vec,
                         size_t label_vector_size, size_t label_number, const int vector_size, int* segmentation_data)
{

	const unsigned int index = blockIdx.x*blockDim.x + threadIdx.x;
	//printf("%d\n", index);
	if (index >= sz) return;

	
	//std::vector<float> average_vector(vector_size, 0);

	float average_vector[100];
	for(auto i=0;i<100;i++)
	{
		average_vector[i] = 0;
	}


	const int oz = index / (xDim*yDim);
	const int ox = index % xDim;
	const int oy = (index % (xDim*yDim)) / xDim;
	int idx = 0;
	//std::stringstream ss;
	//ss << a;
	//std::string str = ss.str();
	auto center_word = d_volume_data[index];
	//const auto center_word_string = std::to_string(center_word);

	//float* word_vector = &word_vec[center_word*vector_size];

	for (auto j = 0; j < vector_size; j++) average_vector[j] += word_vec[center_word*vector_size+j];


	//const int dx26[26] = { -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1 };
	//const int dy26[26] = { -1, -1, -1,  0,  0,  0,  1,  1,  1, -1, -1, -1,  0,  0,  1,  1,  1, -1, -1, -1,  0,  0,  0,  1,  1,  1 };
	//const int dz26[26] = { -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1 };

	auto cnt = 1;
	int len = 26;
	if (window_size == 3) len = 26;
	else if (window_size == 1) len = 1;
	for (auto i = 0; i < len; i++)
	{
		int nx = ox;//new x
		int ny = oy;//new y
		int nz = oz;//new z
		if(window_size==3)
		{
			nx += dx26[i];
			ny += dy26[i];
			nz += dz26[i];
		}
		else if(window_size==1)
		{
			nx += dx6[i];
			ny += dy6[i];
			nz += dz6[i];
		}

		if (nx >= 0 && nx < xDim && ny >= 0 && ny < yDim && nz >= 0 && nz < zDim)
		{
			int nind = nz * xDim*yDim + ny * xDim + nx;

			cnt++;
			
			//const auto center_word_string = std::to_string(volume_data[center_word]);
			//auto& context_vector = word_map[std::to_string(volume_data[nind])];

			idx = d_volume_data[nind] * vector_size;

			for (auto j = 0; j < vector_size; j++) average_vector[j] += word_vec[idx + j];
		}
	}

	for (auto j = 0; j < vector_size; j++) average_vector[j] /= cnt;

	//判断该数组与label数组的关系
	float max_value = -0xffffff;

	cnt = 0;

	for (auto n = 0; n < label_number; n++) {
		//float* label_vector = &label_vec[n*vector_size];
		auto buf = 0.0f;
		for (auto m = 0; m < vector_size; m++)
		{
			buf += average_vector[m] * label_vec[n*vector_size+m];
		}
		if (buf > max_value)
		{
			max_value = buf;
			cnt = n + 1;
		}
	}
	if (max_value < threshold)
	{
		cnt = 0;
	}
	segmentation_data[index] = 10 * cnt;

}


extern "C" void kernelSegmentation(int block_number, int thread_number, unsigned char* d_volume_data, const int sz, const int xDim, const int yDim, const int zDim,
	const float threshold, const int window_size,
	float* word_vector, size_t word_vecotr_size,
	int word_number, float* label_vector, size_t label_vector_size, size_t label_number, const int vector_size, int* segmentation_data)
{
	
	(cudaMemcpyToSymbol(dx26, x26, sizeof(int) * 26, 0, cudaMemcpyHostToDevice));
	(cudaMemcpyToSymbol(dy26, y26, sizeof(int) * 26, 0, cudaMemcpyHostToDevice));
	(cudaMemcpyToSymbol(dz26, z26, sizeof(int) * 26, 0, cudaMemcpyHostToDevice));

	(cudaMemcpyToSymbol(dx6, x6, sizeof(int) * 6, 0, cudaMemcpyHostToDevice));
	(cudaMemcpyToSymbol(dy6, y6, sizeof(int) * 6, 0, cudaMemcpyHostToDevice));
	(cudaMemcpyToSymbol(dz6, z6, sizeof(int) * 6, 0, cudaMemcpyHostToDevice));


	kernel_segmentation << <block_number, thread_number >> > (d_volume_data, sz, xDim, yDim, zDim, threshold,window_size, word_vector, word_vecotr_size,
		word_number, label_vector, label_vector_size, label_number, vector_size, segmentation_data);



	cudaError_t cudaStatus = cudaGetLastError();
	if (cudaStatus != cudaSuccess)
	{
		fprintf(stderr, "addKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
		//goto Error;
	}
	cudaDeviceSynchronize();
	//printf("Kernel end.\n");
}
