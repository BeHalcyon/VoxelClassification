#pragma once
#include <string>
#include <vector>

class Vector2Image
{
public:
	Vector2Image(const std::string & input_word_emb_file);
	~Vector2Image();

	void process();
	void savePNG(const std::string & output_similarity_map);
private:
	void readWordVector(std::vector<std::vector<float>>& vec);
	void calcSimilarity(const std::vector<std::vector<float>>& vec, std::vector<unsigned char>& image_data);

	std::string							input_word_emb_file;
	int									vector_size;
	const int							MAX_STRING = 10000;
	std::vector<std::vector<float>>		vec;
	const int							DIMENSION = 256;
	std::vector<unsigned char>			image_data;
};

