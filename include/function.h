//
// Created by 芦yafei  on 14/6/12.
//

#ifndef LU_NET_FUNCTION_H
#define LU_NET_FUNCTION_H

#include <iostream>
#include <eigen3/Eigen/Dense>
#include <vector>

using namespace std;
using namespace Eigen;

namespace lu_net{

    //generate guassian random value
    float_t gaussian_random(float_t x);

    // Finding the index of max value in vector
    template <typename T>
    int max_index(const vector<T> &vec) {
        auto begin_iterator = begin(vec);
        return max_element(begin_iterator, end(vec)) - begin_iterator;
    }

    // Checking for finite within vectors/matrices
    template<typename Derived>
    inline bool has_finite(const Eigen::MatrixBase<Derived>& x)
    {
        return ( (x - x).array() == (x - x).array()).all();
    }

    // Checking for NaNs within vectors/matrices
    template<typename Derived>
    inline bool has_nan(const Eigen::MatrixBase<Derived>& x)
    {
        return ((x.array() == x.array())).all();
    }
}

#endif //LU_NET_FUNCTION_H
