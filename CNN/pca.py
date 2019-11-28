import matplotlib.pyplot as plt                 #加载matplotlib用于数据的可视化
from sklearn.decomposition import PCA           #加载PCA算法包
from sklearn.datasets import load_iris
import json
import sys
import numpy as np


if __name__ == '__main__':

    sys.path.append('./')

    argv = np.array(sys.argv)
    print('sys.argv:', argv)

    if argv.shape[0] >= 2:
        data_json = argv[1]
    else:
        data_json = R'E:\project\science_project\VoxelClassification\x64\Release\configuration_json\TOOTH_8bit_128_128_160_data.json'

    with open(data_json,'r') as f:
        json_data = json.load(f)

    file_prefix = json_data['data_path']['file_prefix']

    word_vector_json_file = file_prefix + json_data['value_prediction']['word_embedding_json']
    png_file = file_prefix + json_data['value_prediction']['pca_reduce_figure_file']
    group_json_file = file_prefix + json_data['value_prediction']['graph_classification_file']
    label_vector_file = file_prefix + json_data['file_name']['all_embedding_file']


    with open(word_vector_json_file, 'r') as f:
        json_data = json.load(f)

    x = np.array(json_data['word_vector'])
    word_name_vector = json_data['word_name_vector']

    with open(group_json_file, 'r') as f:
        group_json_data = json.load(f)

    node_vector = group_json_data['nodes']
    # print(len(node_vector))
    group_array = np.zeros(shape=[len(node_vector)])

    i = 0
    for word_name in word_name_vector:
        for node in node_vector:
            if node['id'] == word_name:
                group_array[i] = node['group']
                i += 1
                break

    # Get label vector
    label_name_vector = json_data['label_name_vector']

    # print(label_name_vector)

    for label_name in label_name_vector:
        for node in node_vector:
            if node['id'] == label_name:
                group_array[i] = node['group']
                i += 1
                break

    label_vector = []
    with open(label_vector_file, 'r') as f:
        label_vector = f.readlines()
    # print(len(label_vector))
    for i in label_vector[1:]:

        i = i[:-2]
        items = np.array(i.split(' '))
        for j in label_name_vector:
            # print(items[0], j)
            if items[0] == j:
                # print(len(items))
                buf = np.array([float(a) for a in items[1:]])
                buf = buf[np.newaxis, :]
                x = np.append(x, buf, axis=0)
                break

    # print(label_name_vector)
    # print(x)
    # print(x.shape)
    pca = PCA(n_components=2)  # 加载PCA算法，设置降维后主成分数目为2
    reduced_x = pca.fit_transform(x)  # 对样本进行降维

    # plt.scatter(reduced_x[:,0],reduced_x[:,1],c='g',marker='.')
    # plt.show()

    group_number = len(label_name_vector)

    color = ['k', 'r', 'g', 'b', 'c', 'm', 'w', 'y']

    for j in range(group_number + 1):
        cur_x, cur_y = [], []
        for i in range(len(reduced_x)):
            if group_array[i] == j:
                cur_x.append(reduced_x[i][0])
                cur_y.append(reduced_x[i][1])

        plt.scatter(cur_x, cur_y, c=color[j % (group_number + 1)], marker='.')

    # print(group_array)
    # print(group_number)
    # print(len(x))
    for j in range(group_number + 1, 2 * group_number + 1):
        cur_x, cur_y = [], []
        for i in range(len(reduced_x)):
            # print(j, i, group_array[i])
            if group_array[i] == j:
                # print(group_array[i])
                cur_x.append(reduced_x[i][0])
                cur_y.append(reduced_x[i][1])
        color_index = (j - group_number - 1) % (group_number + 1) + 1

        plt.scatter(cur_x, cur_y, c=color[color_index], marker='s')

    # 可视化
    plt.savefig(png_file)
    plt.show()

    print('PCA reduce dimension has been calculated.')
