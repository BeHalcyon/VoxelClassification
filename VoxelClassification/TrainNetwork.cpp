#include "TrainNetwork.h"




TrainNetwork::TrainNetwork(const std::string& node_file, const std::string& words_file, const std::string& hin_file,
	const int& vector_size, const int& negative_number, const long long sample_number, const float& alpha,
	const int& num_threads): edge_count_actual(0), starting_alpha(0)
{
	this->nodes_file = node_file;
	this->words_file = words_file;
	this->hin_file = hin_file;
	this->output_file = "./workspace/word.emb";
	this->vector_size = vector_size;
	this->negative = negative_number;
	this->samples = sample_number;
	this->alpha = alpha;
	this->num_threads = num_threads;

	is_trained = false;

	gsl_rng_env_setup();
	gsl_T = gsl_rng_rand48;
	gsl_r = gsl_rng_alloc(gsl_T);
	assert(gsl_r);
	gsl_rng_set(gsl_r, 314159265);


}

double TrainNetwork::func_rand_num()
{
	return gsl_rng_uniform(gsl_r);
}


void *TrainNetwork::TrainModelThread(void *id)
{
	long long edge_count = 0, last_edge_count = 0;
	unsigned long long next_random = (long long)id;
	hxy::real *error_vec = (hxy::real *)calloc(vector_size, sizeof(hxy::real));

	while (1)
	{
		if (edge_count > samples / num_threads + 2) break;

		if (edge_count - last_edge_count > 10000)
		{
			edge_count_actual += edge_count - last_edge_count;
			last_edge_count = edge_count;
			printf("%cAlpha: %f Progress: %.3lf%%", 13, alpha, (hxy::real)edge_count_actual / (hxy::real)(samples + 1) * 100);
			fflush(stdout);
			alpha = starting_alpha * (1 - edge_count_actual / (hxy::real)(samples + 1));
			if (alpha < starting_alpha * 0.0001) alpha = starting_alpha * 0.0001;
		}

		trainer_ww.train_sample(alpha, error_vec, [this]() {return this->func_rand_num(); } , next_random);
		//trainer_dw.train_sample(alpha, error_vec, func_rand_num, next_random);
		trainer_lw.train_sample(alpha, error_vec, [this]() {return this->func_rand_num(); }, next_random);

		//edge_count += 3;
		edge_count += 2;
	}
	free(error_vec);
	pthread_exit(NULL);

	return 0;
}

TrainNetwork * ths = nullptr;
void* f(void * a)
{
	ths->TrainModelThread(a);
	return 0;
}

void TrainNetwork::TrainModel()
{
	long a;
	pthread_t *pt = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
	starting_alpha = alpha;
	//nodes_file格式，将所有的node文件合并：name
	nodes.init(nodes_file.c_str(), vector_size);
	//words_file格式（words.node），针对于单词库：name
	words.init(words_file.c_str(), vector_size);
	//hin_file格式，将所有的ww.net,lw.net,dl.net合并：name_a name_b weight type
	//保证u为全部节点： w1 w2 w3 w4 ... wn | l1 l2 l3 l4 ... lm | d1 d2 d3 ... dp
	//保证v为单词节点： w1 w2 w3 w4 ... wn

	//text_hin可看作是网络与words节点连接的网络
	text_hin.init(hin_file.c_str(), &nodes, &words);

	trainer_ww.init('w', &text_hin, negative);

	//trainer_dw.init('d', &text_hin, negative);
	trainer_lw.init('l', &text_hin, negative);

	clock_t start = clock();
	printf("Training process:\n");
	ths = this;
	for (a = 0; a < num_threads; a++) pthread_create(&pt[a], NULL, f, (void *)a);
	for (a = 0; a < num_threads; a++) pthread_join(pt[a], NULL);
	printf("\n");
	clock_t finish = clock();
	printf("Total time: %lf\n", (double)(finish - start) / CLOCKS_PER_SEC);

	is_trained = true;
	
	//words.outputSimilarity(output_file, binary);
}
void TrainNetwork::saveWordEmbedding()
{
	if(!is_trained)
	{
		TrainModel();
	}
	words.output(output_file.c_str(), binary);
}

TrainNetwork::~TrainNetwork()
{
	if(gsl_r)
	{
		std::cout << "-------you" << std::endl;
		gsl_rng_free(gsl_r);
		std::cout << "------------------------I-----------------------love -----------------you" << std::endl;
	}
}
