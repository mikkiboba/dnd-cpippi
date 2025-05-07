#pragma once
#include <opencv2/opencv.hpp> 

enum Colors {
	RED,
	GREEN,
	BLUE,
	YELLOW,
	PINK,
	ORANGE,
	PURPLE
};

class Preprocess {

private:
	cv::Mat original;
	cv::Mat fgMask; // grid mask
	cv::Mat entityMask;

	cv::Point pt1;				// top left
	cv::Point pt2;				// top right
	cv::Point pt3;				// bottom left
	cv::Point pt4;				// bottom right

public:
	Preprocess(cv::Mat& frame) {
		original = frame;	
	};
	~Preprocess();
	void emptyGridMask();
	int getOffset(bool dirX);
	void generateVertices();
	int getDim(bool dirX);

	inline cv::Mat const getFgMask() { return fgMask; }
	inline cv::Mat const getEntityMask() { return entityMask; }
	inline cv::Mat  const getOriginal() { return entityMask; }

	void preprocessFrame();
};