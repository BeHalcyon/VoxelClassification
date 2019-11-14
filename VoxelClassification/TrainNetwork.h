#pragma once
#include<string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <gsl/gsl_rng.h>
#include "linelib.h"
#include <map>

class TrainNetwork
{
public:
	TrainNetwork() =default;
	TrainNetwork(const std::string & node_file, const std::string & words_file, const std::string & hin_file,
		const int & vector_size, const int & negative_number, const long long sample_number, const float & real,
		const int & num_threads);
	TrainNetwork & operator=(const TrainNetwork & other) = delete;
	TrainNetwork(const TrainNetwork &) = delete;
	TrainNetwork & operator=(TrainNetwork && other) = delete;
	TrainNetwork(TrainNetwork &&) = delete;

	double func_rand_num();
	void* TrainModelThread(void* id);
	void TrainModel();
	void saveLabelEmbedding();
	void saveWordEmbedding();
	std::map<std::string, std::vector<hxy::real>> exportLabelVector();
	std::map<std::string, std::vector<hxy::real>> exportWordVector();

	bool trainState() const { return is_trained; }
	~TrainNetwork();

private:
	std::string nodes_file, words_file;
	std::string hin_file;
	std::string output_file;
	int binary = 0, num_threads = 1, vector_size = 100, negative = 5;
	long long samples = 1, edge_count_actual;
	float alpha = 0.025, starting_alpha;

	const gsl_rng_type * gsl_T; 
	gsl_rng * gsl_r = nullptr;

	hxy::line_node nodes, words;
	hxy::line_hin text_hin;
	hxy::line_trainer trainer_lw, trainer_dw, trainer_ww;

	bool		is_trained = false;

	std::map<std::string, std::vector<hxy::real>>	label_map;
	std::map<std::string, std::vector<hxy::real>>	word_map;
};

