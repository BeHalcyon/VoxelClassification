#pragma once
#include "VMUtils/include/VMUtils/json_binding.hpp"
struct DataPathJSONStruct : public vm::json::Serializable<DataPathJSONStruct>
{
	VM_JSON_FIELD(std::string, vifo_file);
	VM_JSON_FIELD(std::string, file_prefix);
	VM_JSON_FIELD(int, volume_index);
};

struct FileNameJSONStruct : public vm::json::Serializable<FileNameJSONStruct>
{
	VM_JSON_FIELD(std::string, ww_net_file);
	VM_JSON_FIELD(std::string, word_node_file);
	VM_JSON_FIELD(std::string, lw_net_file);
	VM_JSON_FIELD(std::string, label_node_file);
	VM_JSON_FIELD(std::string, text_hin_file);
	VM_JSON_FIELD(std::string, text_node_file);
	VM_JSON_FIELD(std::string, word_embedding_file);
	VM_JSON_FIELD(std::string, all_embedding_file);

	VM_JSON_FIELD(std::string, ww_net_origin_file);
	VM_JSON_FIELD(std::string, lw_net_origin_file);
	VM_JSON_FIELD(std::string, text_hin_origin_file);

};

struct DataPrepareJSONStruct : public vm::json::Serializable<DataPrepareJSONStruct>
{
	VM_JSON_FIELD(int, window_size);
	VM_JSON_FIELD(int, ww_edge_weight_type);
	VM_JSON_FIELD(int, lw_edge_weight_type);
	VM_JSON_FIELD(std::string, graph_json_file);
};

struct TrainProcessJSONStruct : public vm::json::Serializable<TrainProcessJSONStruct>
{
	VM_JSON_FIELD(int, train_vector_size);
	VM_JSON_FIELD(int, train_sample_number);
	VM_JSON_FIELD(double, train_alpha);
	VM_JSON_FIELD(int, train_thread_number);
	VM_JSON_FIELD(int, train_negative_number);

};

struct ValuePredictionJSONStruct : public vm::json::Serializable<ValuePredictionJSONStruct>
{
	VM_JSON_FIELD(std::string, word_embedding_json);
};


struct SegmentationProcessJSONStruct : public vm::json::Serializable<SegmentationProcessJSONStruct>
{
	VM_JSON_FIELD(std::string, segmentation_file_name);
	VM_JSON_FIELD(int, segmentation_window_size);
	VM_JSON_FIELD(double, segmentation_threshold);
	VM_JSON_FIELD(int, segmentation_gpu);
};

struct SimilarityProcessJSONStruct : public vm::json::Serializable<SimilarityProcessJSONStruct>
{
	VM_JSON_FIELD(std::string, similarity_file);
};

struct InputFileJSONStruct : public vm::json::Serializable<InputFileJSONStruct>
{
	VM_JSON_FIELD(DataPathJSONStruct, data_path);
	VM_JSON_FIELD(FileNameJSONStruct, file_name);
	VM_JSON_FIELD(DataPrepareJSONStruct, data_prepare);
	VM_JSON_FIELD(TrainProcessJSONStruct, train_process);
	VM_JSON_FIELD(ValuePredictionJSONStruct, value_prediction);
	VM_JSON_FIELD(SegmentationProcessJSONStruct, segmenation_process);
	VM_JSON_FIELD(SimilarityProcessJSONStruct, similarity_process);

};
