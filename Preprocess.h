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
	void emptyGridMask(cv::Mat& original, cv::Mat& background);
	void generateEntityMask(cv::Mat& original, cv::Mat& backgroundGridImg);
	int getOffset(bool dirX);
	void generateVertices();

	// i const e & velocizzano restituendo direttamente la reference senza fare copie , dovrebbere essere safe nel nostro caso
	inline const cv::Mat& getFgMask() const { return fgMask; }   
	inline const cv::Mat& getEntityMask() const { return entityMask; }
	inline const cv::Point& getPt1() const { return pt1; }
	inline const cv::Point& getPt2() const { return pt2; }
	inline const cv::Point& getPt3() const { return pt3; }
	inline const cv::Point& getPt4() const { return pt4; }

};