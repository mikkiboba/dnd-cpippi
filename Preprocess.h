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
	cv::Mat fgMask; // grid mask
	cv::Mat entityMask;

	cv::Point pt1;				// top left
	cv::Point pt2;				// top right
	cv::Point pt3;				// bottom left
	cv::Point pt4;				// bottom right

public:
	Preprocess() {};
	~Preprocess();
	void emptyGridMask(cv::Mat original, cv::Mat background);
	void generateEntityMask(cv::Mat original, cv::Mat backgroundGridImg);
	int getOffset(bool dirX);
	void generateVertices();

	inline cv::Mat const getFgMask() { return fgMask; }
	inline cv::Mat const getEntityMask() { return entityMask; }
	inline cv::Point const getPt1() { return pt1; }
	inline cv::Point const getPt2() { return pt2; }
	inline cv::Point const getPt3() { return pt3; }
	inline cv::Point const getPt4() { return pt4; }


	void preprocessFrame();
};