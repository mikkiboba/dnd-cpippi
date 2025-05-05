// HCI.cpp: definisce il punto di ingresso dell'applicazione.
//
#include <opencv2/opencv.hpp> 
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

#include "HCI.h"


// TODO: cv::createBackgroundSubtractorMOG2() or cv::createBackgroundSubtractorKNN()



void showImg(cv::Mat img) {
	cv::imshow("Image", img);
	cv::waitKey(0);
}


int main()
{
	cv::Mat image = cv::imread("../imgs/background.jpg");

	if (image.empty()) {
        std::cerr << "Could not read the image" << std::endl;
        return 1;
    }

	cv::Mat hsv_image;
	cv::cvtColor(image, hsv_image, cv::COLOR_BGR2HSV);

	cv::Mat pixels;
	hsv_image.reshape(1, hsv_image.total()).convertTo(pixels, CV_32F);

	int clusters = 5;
	cv::Mat labels, centers;
    cv::kmeans(pixels, clusters, labels, cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 10, 1.0), 3, cv::KMEANS_PP_CENTERS, centers);

	int largest_cluster_idx = 0;
	int largest_cluster_size = 0;
	for (int i = 0; i < clusters; i++) {
		int cluster_size = cv::countNonZero(labels==i);
		if (cluster_size > largest_cluster_size) {
			largest_cluster_size = cluster_size;
			largest_cluster_idx = i;
		}
	}

	cv::Vec3f prevalent_color = centers.at<cv::Vec3f>(largest_cluster_idx);
	std::cout << "Prevalent color: " << prevalent_color << std::endl;

	cv::Mat hsv_pixel(1, 1, CV_32FC3, prevalent_color);
	cv::Mat bgr_pixel;
	cv::cvtColor(hsv_pixel, bgr_pixel, cv::COLOR_HSV2BGR);

	bgr_pixel.convertTo(bgr_pixel, CV_8UC3);

	cv::Mat color_image(100, 100, CV_8UC3, bgr_pixel.at<cv::Vec3b>(0, 0));
	showImg(color_image);


    return 0;
}