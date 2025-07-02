#pragma once
#include <opencv2/opencv.hpp> 
#include <chrono>



class Preprocess {

public:
	enum class Color : int {
		NONE 	= 0,
		RED		= 1,
		GREEN	= 2,
		BLUE	= 3,
		OTHER	= 4
	};

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
	cv::Mat currentMatrix;

	

public:
	Preprocess() {};
	~Preprocess();

	void defineMedia();
	void initializeDefaults(const cv::Mat& img);
	void run();

	Color detectColor(const cv::Vec3b& hsvColor);

	void findEntities(cv::Mat& rgbFrame, cv::Mat& mask);

	bool findLargestSquareContour(const cv::Mat& thresh, std::vector<cv::Point>& bestApprox);
	std::vector<cv::Point2f> orderPoints(std::vector<cv::Point>& pts);
	cv::Mat warpToSquare(const cv::Mat& image, const std::vector<cv::Point2f>& srcPts, float side = 500.f);
	void extractGridLines(const cv::Mat& binary, cv::Mat& horizontal, cv::Mat& vertical, int side);
	void countGridLines(const std::vector<std::vector<cv::Point>>& horContours,
                               		const std::vector<std::vector<cv::Point>>& verContours,
                               		int side, int& rowCount, int& colCount);
	void processGrid(const cv::Mat& binary, const cv::Mat& horizontal, const cv::Mat& vertical, float side, const cv::Mat& warped);
	cv::Mat elaborateFrame(cv::Mat& image);

	

};