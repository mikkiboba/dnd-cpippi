#pragma once
#include "Matrix.h"


class Kitchen {
    private:
        Matrix grid;
        cv::Mat bgColorImage;
        cv::Mat bgGridImage;
        cv::Mat frameImage;

        cv::VideoCapture video;
    
    public:
        Kitchen() 
            : grid() {
            defineImgs(true);
        }

        Kitchen(cv::VideoCapture& vid) 
            : video(vid), grid() {
            //defineImgs(false);
            //video.read(bgColorImage);
        }
        ~Kitchen();
        void showImg(cv::Mat& img);
        void defineImgs(bool quack);
        void defineImgsFromVideo(int n);
        void letHimCook() ;
        bool cooking();
};