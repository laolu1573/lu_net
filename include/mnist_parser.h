#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "net.h"
#include <glog/logging.h>

using namespace std;
using namespace lu_net;

int ReverseInt(int i) {
    unsigned char ch1, ch2, ch3, ch4;
    ch1 = i & 255;
    ch2 = (i >> 8) & 255;
    ch3 = (i >> 16) & 255;
    ch4 = (i >> 24) & 255;
    return ((int) ch1 << 24) + ((int) ch2 << 16) + ((int) ch3 << 8) + ch4;
}

void read_Mnist_Label(string filename, vector<label_t> &labels) {
    ifstream file(filename, ios::binary);
    if (file.is_open()) {
        int magic_number = 0;
        int number_of_images = 0;
        file.read((char *) &magic_number, sizeof(magic_number));
        file.read((char *) &number_of_images, sizeof(number_of_images));
        magic_number = ReverseInt(magic_number);
        number_of_images = ReverseInt(number_of_images);
        //cout << "magic number = " << magic_number << endl;
        //cout << "number of cifar-10 = " << number_of_images << endl;

        for (int i = 0; i < number_of_images; i++) {
            unsigned char label = 0;
            file.read((char *) &label, sizeof(label));
            labels.push_back((uint32_t) label);
        }

    }
}

void read_Mnist_Images(string filename, vector<vec_t> &images) {
    ifstream file(filename, ios::binary);
    if (file.is_open()) {
        int magic_number = 0;
        int number_of_images = 0;
        int n_rows = 0;
        int n_cols = 0;
        unsigned char label;
        file.read((char *) &magic_number, sizeof(magic_number));
        file.read((char *) &number_of_images, sizeof(number_of_images));
        file.read((char *) &n_rows, sizeof(n_rows));
        file.read((char *) &n_cols, sizeof(n_cols));
        magic_number = ReverseInt(magic_number);
        number_of_images = ReverseInt(number_of_images);
        n_rows = ReverseInt(n_rows);
        n_cols = ReverseInt(n_cols);

        //cout << "magic number = " << magic_number << endl;
        LOG(INFO) << "number of images = " << number_of_images;
        //cout << "rows = " << n_rows << endl;
        //cout << "cols = " << n_cols << endl;

        for (int i = 0; i < number_of_images; i++) {
            vec_t tp;
            for (int r = 0; r < n_rows; r++) {
                for (int c = 0; c < n_cols; c++) {
                    unsigned char image = 0;
                    file.read((char *) &image, sizeof(image));
                    tp.push_back(int(image) / 255.0);
                }
            }
            images.push_back(tp);
        }
    }
}