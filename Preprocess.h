#pragma once
#include <opencv2/opencv.hpp> 
#include <chrono>
enum TestColors {
	TEST_RED = 1,
	TEST_GREEN = 2,
	TEST_BLUE = 3,
	TEST_OTHER = 4,
	TEST_NONE = 0
};


class Preprocess {
private:
	cv::VideoCapture loopVid;


	int defaultRows = -1;
	int defaultCols = -1;

	int defaultPixelHor = 0;
	int defaultPixelVer = 0;

	int defaultCelHor = 0;
	int defaultCelVer = 0;

	int defaultInCelHor = 0;
	int defaultInCelVer = 0;

	int defaultOffsetHor = 0;
	int defaultOffsetVer = 0;

	int gridRows = 0;
	int gridCols = 0;

	cv::Vec3b defaultColor = cv::Vec3b(0, 0, 0);


	int count = 0;
	cv::Mat previousValidHoles;

	

public:
	Preprocess() {};
	~Preprocess();

	void defineMedia();
	int detectColor(const cv::Vec3b& hsvColor);
	void findEntitties(cv::Mat& clothed, cv::Mat& nude);
	cv::Mat elaborateFrame(cv::Mat& image);
	void boobs();
	

};