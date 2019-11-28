import sys
sys.path.append('./')

import keras
import os

import numpy as np
import tensorflow as tf
from keras import backend as K
from keras import losses
from keras.models import Sequential, Model
from keras.optimizers import Adam
import json
import sys

from keras.layers import Dense, BatchNormalization, Flatten, \
    Reshape, Conv3DTranspose, Input, Dropout, Activation

def doValuePrediction(X,Y):


    from keras.models import Sequential


    # print(X)
    # print(X.shape)
    # print(Y)
    # print(Y.shape)

    model = Sequential()
    model.add(Dense(256, input_dim=100, init='uniform', activation='relu'))
    model.add(Dense(1024, init='uniform', activation='relu'))
    model.add(Dense(512, init='uniform', activation='relu'))
    model.add(Dense(Y.shape[1], init='uniform'))
    model.add(Activation('sigmoid'))

    model.compile(loss='binary_crossentropy', optimizer='adam', metrics=['accuracy'])
    # Fit the model
    model.fit(X, Y, epochs=10)

    # evaluate the model
    scores = model.evaluate(X, Y)
    print("%s: %.2f%%" % (model.metrics_names[1], scores[1] * 100))
    # calculate predictions
    predictions = model.predict(X)
    # print(predictions)
    # round predictions
    # i = 0
    # for x in predictions:
    #     print(i,x, Y[i])
    #     i = i + 1

    print('Train process for value prediction is over.')
    return predictions, Y

class MyEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, np.integer):
            return int(obj)
        elif isinstance(obj, np.floating):
            return float(obj)
        elif isinstance(obj, np.ndarray):
            return obj.tolist()

        else:
            return super(MyEncoder, self).default(obj)


def transferToJson(threshold):
    global a
    i = 0
    for x in predictions:
        # print(x)

        if np.max(x) < threshold:
            x[:] = 0
        else:
            x[np.where(x == np.max(x))] = 1
        x[np.where(x != 1)] = 0
        # print(i, x, Y[i])
        predictions[i] = x
        i = i + 1
        # print(x)
    word_name_vector = data['word_name_vector']
    with open(graph_json_file) as a:
        graph_group_data = json.load(a)
        # print(graph_group_data)
    graph_node_data = graph_group_data['nodes']
    i = 0
    id = 0
    max_id = 0
    for x in word_name_vector:
        j = 0
        for y in graph_node_data:
            if y['id'].isdigit():
                if y['id'] == word_name_vector[i]:
                    buf = np.array(np.where(predictions[i] == 1))
                    # print(buf, buf.shape,buf.shape[0], buf.shape[1])
                    if buf.shape[1] == 0:
                        id = 0
                    else:
                        id = buf[0] + 1

                    graph_node_data[j]['group'] = int(id)
                    max_id = max(max_id, id)
                    break

            j = j + 1

        i = i + 1
    i = 1
    for y in graph_node_data:
        if not y['id'].isdigit():
            y['group'] = int(max_id + i)
            i += 1
    graph_group_data['nodes'] = graph_node_data
    with open(graph_classification_file, "w") as f:
        json.dump(graph_group_data, f, cls=MyEncoder)
        print("The value prediction has been finished...")


if __name__ == '__main__':

    # a = sys.argv[0]
    # print('sys.argv[0]:', a)
    argv = np.array(sys.argv)
    print('sys.argv:', argv)
    data_json = ''
    threshold = 0.1
    if argv.shape[0]>=2:
        data_json = argv[1]
    else:
        data_json = R'E:\project\science_project\VoxelClassification\x64\Release\configuration_json\TOOTH_8bit_128_128_160_data.json'

    with open(data_json, 'r') as load_f:
        data_json = json.load(load_f)

    print(data_json)

    file_prefix = data_json['data_path']['file_prefix']
    json_file = file_prefix + data_json['value_prediction']['word_embedding_json']
    graph_json_file = file_prefix + data_json['data_prepare']['graph_json_file']
    graph_classification_file = file_prefix + data_json['value_prediction']['graph_classification_file']
    threshold = data_json['value_prediction']['filter_threshold']
    if argv.shape[0] >= 3:
        threshold = float(argv[2])
    #json_file = "J:/PTE buffer/jet_mixfrac_0051_word_label_json.json"

    with open(json_file) as load_json:
        data = json.load(load_json)
        # print(data)

    X = data['word_vector']
    Y = data['label_vector']
    X = np.array(X)
    Y = np.array(Y)

    predictions, Y = doValuePrediction(X,Y)

    # print(predictions.shape)
    # print(np.where(predictions == np.max(predictions, axis=1)))
    # predictions[np.where(np.max(predictions,axis=1) == predictions)]=1
    # predictions[np.where(predictions!=1)]=0

    transferToJson(threshold)



