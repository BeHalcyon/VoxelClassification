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
print('hello world')

from keras.layers import Dense, BatchNormalization, Flatten, \
    Reshape, Conv3DTranspose, Input, Dropout

def doValuePrediction(json_file):


    from keras.models import Sequential

    with open(json_file) as load_json:
        data = json.load(load_json)
        print(data)

    X = data['word_vector']
    Y = data['label_vector']
    X = np.array(X)
    Y = np.array(Y)
    print(X)
    print(X.shape)
    print(Y)
    print(Y.shape)

    model = Sequential()
    model.add(Dense(X.shape[0], input_dim=100, init='uniform', activation='relu'))
    model.add(Dense(1024, init='uniform', activation='relu'))
    model.add(Dense(4, init='uniform', activation='sigmoid'))

    model.compile(loss='binary_crossentropy', optimizer='adam', metrics=['accuracy'])
    # Fit the model
    model.fit(X, Y, epochs=5)

    # evaluate the model
    scores = model.evaluate(X, Y)
    print("%s: %.2f%%" % (model.metrics_names[1], scores[1] * 100))
    # calculate predictions
    predictions = model.predict(X)
    # print(predictions)
    # round predictions
    # i = 0
    # for x in predictions:
    #     print(i,x)
    #     i = i + 1

    print('Train process for value prediction is over.')

json_file = 
doValuePrediction(json_file)