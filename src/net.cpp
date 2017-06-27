//
// Created by 芦yafei  on 17/6/9.
//

#include <iostream>
#include "../include/net.h"
#include <eigen3/Eigen/Dense>
#include "../include/function.h"
#include "../include/io.h"

using namespace std;
using namespace Eigen;

namespace lu_net {

    void Net::initNet(std::vector<int> layers_neuron_num) {
        num_layers = layers_neuron_num.size();
        fine_tune_factor = 1.01;    //设置学习率变化因子
        output_interval = 10;  //设置训练loss输出间隔

        // resize(int n,element)表示调整容器v的大小为n，调整后的每个元素的值为element，默认为0，
        // resize()会改变容器的容量和当前元素个数
        layers.resize(num_layers);

        //Generate every layer.
        for (int i = 0; i < num_layers; i++) {
            layers[i] = VectorXf::Zero(layers_neuron_num[i]);
        }
        std::cout << "Genarate layers, sucessfully!" << std::endl;

        //Generate every weights matrix and bias，index 0 is unused, use num_layers size for uniform index
        weights.resize(num_layers);
        bias.resize(num_layers);
        gradient.resize(num_layers);
        zs.resize(num_layers);

        cout << "Generate weights matrices and bias successfuly!" << endl;
        cout << "initialize Net, done!" << endl;
    }

    // initialize weights matrices
    void Net::initWeights(double w) {
        for (int i = 1; i < num_layers; i++) {
            weights[i] = MatrixXf::Random(layers[i].rows(), layers[i - 1].rows());
        }
    }

    // initialize bias vectors
    void Net::initBias(double w) {
        for (int i = 1; i < num_layers; i++) {
            bias[i] = VectorXf::Zero(layers[i].rows());
        }
    }

    //farward
    void Net::farward(VectorXf x, VectorXf y) {
        layers[0] = x;

        for (int i = 1; i < num_layers; i++){
            //weighted input
            VectorXf z = weights[i] * layers[i - 1] + bias[i];
            zs[i] = z;
            layers[i] = sigmoid(z);
        }

        //caculate loss on output layer
        calcLoss(layers[num_layers - 1], y, output_error, loss);
    }

    /*compute the partial derivatives for the output activations.
     * */
    VectorXf Net::cost_derivative(VectorXf output_activations, VectorXf y) {
        return (output_activations - y);
    }


    /**
     * compute the w and b gradient of the cost function C_x
     * */
    void Net::backward(const VectorXf &y, vector<MatrixXf> &nabla_w, vector<VectorXf> &nabla_b) {
        //最后一层的error
        VectorXf delta = cost_derivative(layers[num_layers - 1], y).array() * sigmoid_prime(zs[num_layers -1]).array();
        nabla_b[num_layers - 1] = delta;
        nabla_w[num_layers - 1] = delta * layers[num_layers -2].transpose();

        for (int i = num_layers - 2; i >= 1; i--) {
            delta = (weights[i + 1].transpose() * delta).array() * sigmoid_prime(zs[i]).array();
            nabla_b[i] = delta;
            nabla_w[i] = delta * layers[i - 1].transpose();
        }
    }


    /**
     * Update the network's weights and biases by applying gradient descent using backpropagation to a single mini batch.
     * The mini_batch is a list of tuples (x, y), and lr is the learning rate.
     * */
    void Net::update_batch(const vector<tensor_t>& in, const vector<tensor_t>& t, int batch_size) {
        vector<MatrixXf> acum_nabla_w;
        acum_nabla_w.resize(num_layers);

        vector<VectorXf> acum_nabla_b;
        acum_nabla_b.resize(num_layers);

        vector<MatrixXf> delta_nabla_w;
        vector<VectorXf> delta_nabla_b;

        for(int i = 0; i < batch_size; i++) {
            //VectorXf x(&in[i][0], in[i][0].size());
            VectorXf x(in[i][0].size());
            VectorXf y(t[i][0].size());

            for (int k = 0; k < in[i][0].size(); ++k) {
                x[i] = in[i][0][k];
            }

            for (int k = 0; k < t[i][0].size(); ++k) {
                y[i] = t[i][0][k];
            }

            farward(x, y);
            backward(y, delta_nabla_w, delta_nabla_b);

            //将一批样本的改变累加到一起
            for (int j = 1; j < num_layers; ++j) {
                acum_nabla_w[j] = acum_nabla_w[j] + delta_nabla_w[j];
                acum_nabla_b[j] = acum_nabla_b[j] + delta_nabla_b[j];
            }
        }

        //一批样本改变的平均值作为最后的改变
        for (int k = 1; k < num_layers; ++k) {
            weights[k] = weights[k] - learning_rate / batch_size * acum_nabla_w[k];
            bias[k] = bias[k] - learning_rate / batch_size * acum_nabla_b[k];
        }
    }


    /**
    * trains on one minibatch, i.e. runs forward and backward propagation to calculate
    * the gradient of the loss function with respect to the network parameters (weights),
    * then calls the optimizer algorithm to update the weights
    *
    * @param batch_size the number of data points to use in this batch
    */
    void Net::train_onebatch(const tensor_t* in, const tensor_t* t, int batch_size) {
        vector<tensor_t> in_batch(&in[0], &in[0] + batch_size);
        vector<tensor_t> t_batch(&t[0], &t[0] + batch_size);

        update_batch(in_batch, t_batch, batch_size);
    }


    /**
    * train on one minibatch
    *
    * @param size is the number of data points to use in this batch
    */
    void Net::train_once(const tensor_t *in,
                    const tensor_t *t,
                    int size) {
        if (size == 1) {

        } else {
            train_onebatch(in, t, size);
        }
    }


    /**
     * trains the network for a fixed number of epochs (for classification task)
     *
     * This method takes label_t argument and convert to target vector automatically.
     * To train correctly, output dimension of last layer must be greater or equal to
     * number of label-ids.
     *
     * @param inputs             array of input data
     * @param class_labels       array of label-id for each input data(0-origin)
     * @param batch_size         number of samples per parameter update
     * @param epoch              number of training epochs
     */
    bool Net::train(const vector<vec_t> &inputs, const vector<label_t> &class_labels, int batch_size, int epoch) {
        if (inputs.size() != class_labels.size()) {
            return false;
        }
        if (inputs.size() < batch_size || class_labels.size() < batch_size) {
            return false;
        }

        //转化成tensor_t类型
        std::vector<tensor_t> input_tensor, output_tensor, t_cost_tensor;
        normalize_tensor(inputs, input_tensor);
        normalize_tensor(class_labels, output_tensor);

        for (int iter = 0; iter < epoch; iter++) {
            for (int i = 0; i < inputs.size(); i += batch_size) {
                train_once(&input_tensor[i], &output_tensor[i],
                                  static_cast<int>(min<int>(batch_size, inputs.size() - i)));
            }
        }

        cout << "end training." << endl;

        return true;
    }
}