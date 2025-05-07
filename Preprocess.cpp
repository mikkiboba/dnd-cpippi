#include "Preprocess.h"


Preprocess::~Preprocess() {}


void Preprocess::emptyGridMask(cv::Mat original, cv::Mat background) {
	cv::Ptr<cv::BackgroundSubtractorMOG2> pBackSub = cv::createBackgroundSubtractorMOG2();

	pBackSub->apply(background, fgMask, 1.0);
	pBackSub->apply(original, fgMask, 0.0);

	// * remove noise from the masked image
	cv::threshold(fgMask, fgMask, 100, 255, cv::THRESH_BINARY);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	cv::morphologyEx(fgMask, fgMask, cv::MORPH_OPEN, kernel);
	cv::morphologyEx(fgMask, fgMask, cv::MORPH_CLOSE, kernel);
}


void Preprocess::generateEntityMask(cv::Mat original, cv::Mat backgroundGridImg) {
	// * subtract the grid from the frame to show only the "physical" elements
	cv::Ptr<cv::BackgroundSubtractorMOG2> pBackSub2 = cv::createBackgroundSubtractorMOG2();
	pBackSub2 -> apply(backgroundGridImg, entityMask, 1.0);
	pBackSub2 -> apply(original, entityMask, 0.0);
}


int Preprocess::getOffset(bool dirX) {
	int length = dirX ? fgMask.cols : fgMask.rows;
	int center = dirX ? fgMask.rows : fgMask.cols;
	center = center / 2;

	for (int i = 0; i < length; i++) {
		int x = dirX ? i : center;
		int y = dirX ? center : i;

		if (fgMask.at<uchar>(y, x) == 255)
			return dirX ? x : y;
	}

	return 0;
}

void Preprocess::generateVertices() {
	int offsetX = getOffset(1);
	int offsetY = getOffset(0);

	// * get the four points of the rectangle (grid)
	// ! it should be calculated just once on the grid-only frame
	pt1 = cv::Point(offsetX, offsetY);							 // top left
	pt2 = cv::Point(fgMask.cols - offsetX, offsetY);				 // top right
	pt3 = cv::Point(offsetX, fgMask.rows - offsetY);				 // bottom left
	pt4 = cv::Point(fgMask.cols - offsetX, fgMask.rows - offsetY); // bottom right
}
