#pragma once
#include <opencv2/opencv.hpp> 

enum Colors {
	RED,
	GREEN,
	BLUE,
	YELLOW,
	CYAN,
	MAGENTA,
	ORANGE,
	GRAY,
	PURPLE,
	WHITE,
	BLACK,
	OTHER
};

enum Dir {
	X,
	Y
};

enum Side {
	RIGHT,
	LEFT,
	TOP,
	BOTTOM
};

class Preprocess {
private:
	cv::Mat fgMask; // grid mask
	cv::Mat entityMask;

	cv::Point pt1;				// top left
	cv::Point pt2;				// top right
	cv::Point pt3;				// bottom left
	cv::Point pt4;				// bottom right

	int offsetXL;
	int offsetXR;
	int offsetYT;
	int offsetYB;

	bool fgMaskCreated = false;

public:
	Preprocess() {};
	~Preprocess();
	bool isChanged(cv::Mat& referenceFrame, cv::Mat& currentFrame, double threshold);
	void emptyGridMask(cv::Mat& original, cv::Mat& background);
	void generateEntityMask(cv::Mat& original, cv::Mat& backgroundGridImg);
	int getOffset(Dir direction, Side side);
	void generateVertices();

	// i const e & velocizzano restituendo direttamente la reference senza fare copie , dovrebbere essere safe nel nostro caso
	inline  cv::Mat& getFgMask()  { return fgMask; }   
	inline const cv::Mat& getEntityMask() const { return entityMask; }
	inline const cv::Point& getPt1() const { return pt1; }
	inline const cv::Point& getPt2() const { return pt2; }
	inline const cv::Point& getPt3() const { return pt3; }
	inline const cv::Point& getPt4() const { return pt4; }

	inline const int& getOffsetXL() const { return offsetXL; }
	inline const int& getOffsetXR() const { return offsetXR; }
	inline const int& getOffsetYT() const { return offsetYT; }
	inline const int& getOffsetYB() const { return offsetYB; }


	void stab();

};