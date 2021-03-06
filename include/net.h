//
// Created by 芦yafei  on 14/6/9.
//

#ifndef LU_NET_NET_H
#define LU_NET_NET_H

#include <vector>
#include <string.h>
#include <eigen3/Eigen/Dense>
#include <map>

namespace lu_net {
    typedef std::uint32_t label_t;
    typedef float float_t;
    typedef std::vector<float_t> vec_t;
    typedef std::vector<vec_t> tensor_t;

    enum class content_type {
        weights,    //save/load the weights
        model,      //save/load the network architecture
        weights_and_model   //save/load both the weights and the architecture
    };

    enum class file_format {
        binary,
        json
    };

    enum class net_phase {
        train,
        test
    };

    struct result {
        result() : num_success(0), num_total(0) {}

        int num_success;
        int num_total;
        std::map<label_t, std::map<label_t, int> > confusion_matrix;  //不用初始化？

        float accuracy() const {
            return float(num_success * 100.0 / num_total);
        }
    };

    class Net {
    public:
        Net() {};

        virtual ~Net() {};

        std::vector<int> layers_neuron_num;
        int num_layers = 0;
        float learning_rate = 0.0;
        float lmbda = 0.0;              // Regularization parameter, vary with the trainnig data size.
        float batch_loss = 0.0;         // Loss of a batch of data.
        int output_interval = 0;        // Interval of loss print out, measured in epoch
        float fine_tune_factor = 0.0;   // finetune factor of learning rate.

        // initialize net:generate weights matrices、layer matrices and bias matrices
        // bias default all zero
        void initNet(const std::vector<int> layers_neuron_num, float learning_rate, float lmbda);

        // initialize the weights matrices
        void initWeights(const double w = 0);

        //Initial the bias matrices
        void initBias(const double w = 0);

        //Predict just one sample
        int predict_one(const vec_t &input);

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
        template <typename E, typename Optimizer>
        bool train(Optimizer &optimizer, const std::vector<vec_t> &inputs, const std::vector<label_t> &class_labels, int batch_size, int epoch);

        result test(const std::vector<vec_t> &inputs, const std::vector<label_t> &class_labels);

        bool save(const std::string &filename,
                  content_type what = content_type::weights_and_model,
                  file_format format = file_format::binary);

    private:
        std::vector<Eigen::VectorXf> as;    // Store all the a vectors (activation of the neuron), layer by layer.
        std::vector<Eigen::MatrixXf> weights;
        std::vector<Eigen::VectorXf> bias;
        std::vector<Eigen::VectorXf> gradient;
        std::vector<Eigen::VectorXf> zs;    // Store all the z vectors(weighted input), layer by layer.

        /**
        * train on one minibatch.
         *
        * @param size is the number of data points to use in this batch
        */
        template <typename E, typename Optimizer>
        void train_once(Optimizer &optimizer,
                        const tensor_t *in,
                        const tensor_t *t,
                        int size,
                        int n);

        /**
        * trains on one minibatch, i.e. runs forward and backward propagation to calculate
        * the gradient of the loss function with respect to the network parameters (weights),
        * then calls the optimizer algorithm to update the weights
        *
        * @param batch_size the number of data points to use in this batch
        */
        template <typename E, typename Optimizer>
        void train_onebatch(Optimizer &optimizer,
                            const tensor_t *in,
                            const tensor_t *t,
                            int batch_size,
                            int n);

        template <typename E, typename Optimizer>
        void update_batch(Optimizer &optimizer,
                          const std::vector<tensor_t> &in,
                          const std::vector<tensor_t> &t,
                          int batch_size,
                          int n);

        //Backward
        void farward(Eigen::VectorXf x);

        //Forward
        template <typename E>
        void backward(const Eigen::VectorXf &y, std::vector<Eigen::MatrixXf> &nabla_w, std::vector<Eigen::VectorXf> &nabla_b);

        label_t fprop_max_index(const Eigen::VectorXf &in);
    };
}
#endif //LU_NET_NET_H
