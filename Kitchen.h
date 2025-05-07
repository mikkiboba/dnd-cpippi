#pragma once

#include "Matrix.h"


class Kitchen {

    private:
        Matrix grid;
        cv::Mat bgColorImage;
        cv::Mat bgGridImage;
        cv::Mat frameImage;
    
    public:
        Kitchen() {
            defineImgs();
            grid = Matrix();
        }
        ~Kitchen();
        void showImg(cv::Mat img);
        void defineImgs();
        void letHimCook() ;
        void cooking();



};