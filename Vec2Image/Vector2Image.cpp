
#include "Vector2Image.h"
#include <vector>

#include <QtGui/qimage.h>
#include <QtCore/QString>
Vector2Image::Vector2Image(const std::string& input_word_emb_file)
		:input_word_emb_file(input_word_emb_file)
{
	vec.resize(DIMENSION);
	image_data.resize(DIMENSION*DIMENSION, 0);
}

Vector2Image::~Vector2Image()
{
}

void Vector2Image::readWordVector(std::vector<std::vector<float>>& vec)
{
	FILE *fi = fopen(input_word_emb_file.c_str(), "rb");
	char ch, word[100000];
	float f_num;
	int size;

	fscanf(fi, "%d %d", &size, &vector_size);

	//vec = (real *)malloc(size * vector_size * sizeof(real));

	
	for (auto i = 0; i < vec.size(); i++)
	{
		vec[i].resize(vector_size, 0);
	}
	//printf("test0\n");

	for (long long k = 0; k != size; k++)
	{
		fscanf(fi, "%s", word);
		auto intword = atoi(word);
		//printf("%s %d\n", word, intword);
		//ch = fgetc(fi);
		//ch++;
		//AddVertex(word);
		for (int c = 0; c != vector_size; c++)
		{
			fscanf(fi, "%f", &f_num);
			//fread(&f_num, sizeof(real), 1, fi);
			vec[intword][c] = float(f_num);
			//vec[c + k * vector_size] = (real)f_num;
		}
	}
	fclose(fi);
	printf("Number of words: %d\n", size);
	printf("Vector dimension: %d\n", vector_size);
}

void Vector2Image::calcSimilarity(const std::vector<std::vector<float>>& vec, std::vector<unsigned char>& image_data)
{
	std::vector<std::vector<float>> similarity_array(256);

	for (auto i = 0; i < similarity_array.size(); i++) similarity_array[i].resize(256, 0);

	for (auto i = 0; i < similarity_array.size(); i++)
	{
		float a_mode = 0;
		for (auto k = 0; k < vector_size; k++)
		{
			a_mode += vec[i][k] * vec[i][k];
		}

		for (auto j = i; j < similarity_array[i].size(); j++)
		{
			float b_mode = 0;
			float ab_mult = 0;
			for (auto k = 0; k < vector_size; k++)
			{
				b_mode += vec[j][k] * vec[j][k];
				ab_mult += vec[i][k] * vec[j][k];
			}
			if (a_mode == 0 || b_mode == 0)
			{
				similarity_array[i][j] = 0;
			}
			else
			{
				float intv = 1.0 / (std::sqrt(a_mode)*std::sqrt(b_mode));
				similarity_array[i][j] = ab_mult * intv;
			}
			if (similarity_array[i][j] < 0)
				similarity_array[i][j] = 0;

			similarity_array[j][i] = similarity_array[i][j];
		}
	}
	//printf("test1\n");
	
	for (auto i = 0; i < similarity_array.size(); i++)
	{
		for (auto j = 0; j < similarity_array[i].size(); j++)
		{
			image_data[i*similarity_array[i].size() + j] = unsigned char(similarity_array[i][j] * 255);
		}
	}
}

void Vector2Image::process()
{
	readWordVector(vec);
	calcSimilarity(vec, image_data);
}

void Vector2Image::savePNG(const std::string& output_similarity_map)
{
	QImage image(image_data.data(), DIMENSION, DIMENSION, QImage::Format_Grayscale8);
	for (auto i = 0; i < DIMENSION; i++)
	{
		for (auto j = 0; j < DIMENSION; j++)
		{
			const auto buf = image_data[i*DIMENSION + j];
			image.setPixel(j, i, qRgb(255 - buf, 255 - buf, 255 - buf));
		}
	}
	image.mirrored(false, true).save(output_similarity_map.c_str());
	printf("The similarity map has been saved.\n");
}
