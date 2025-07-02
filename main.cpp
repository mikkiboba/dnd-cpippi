#include "Preprocess.h"

#include <opencv2/opencv.hpp>


void showImg(cv::Mat img) {
    cv::imshow("", img);
    cv::waitKey(0);
}


int main() {
    Preprocess p = Preprocess();
    p.run();
    return 0;
}