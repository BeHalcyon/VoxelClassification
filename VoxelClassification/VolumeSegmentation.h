#pragma once
#include <map>
#include <vector>

class VolumeSegmentation
{
public:
	VolumeSegmentation();

	void segemation(
		const std::vector<unsigned char>&				volume_data,
		const int &										width,
		const int &										height,
		const int &										depth,
		std::map<std::string, std::vector<float>>&		word_map,
		std::map<std::string, std::vector<float>>&		label_map,
		const int &										window_size,
		const int &										vector_size,
		std::vector<int>&								segementaion_data
	);
	~VolumeSegmentation();
	void saveSegmentation(const std::vector<int>& segmentation_volume);
};

