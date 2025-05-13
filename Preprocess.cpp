#include "Preprocess.h"

Preprocess::~Preprocess() {}

bool Preprocess::isChanged(cv::Mat& referenceFrame, cv::Mat& currentFrame, double threshold) {

	//emptyGridMask(referenceFrame, currentFrame);
	cv::Mat gray1, gray2;
	cv::cvtColor(referenceFrame, gray1, cv::COLOR_BGR2GRAY);
	cv::cvtColor(currentFrame, gray2, cv::COLOR_BGR2GRAY);

	////cv::imshow("gray1", gray1);
	////cv::imshow("gray2",gray2);

	cv::GaussianBlur(gray1, gray1, cv::Size(5, 5), 0);
	cv::GaussianBlur(gray2, gray2, cv::Size(5, 5), 0);

	//// Absolute difference
	cv::Mat diff;
	cv::absdiff(gray1, gray2, diff);

	//// Threshold the difference
	cv::Mat thresh;
	cv::threshold(diff, thresh, 25, 255, cv::THRESH_BINARY);
	//::imshow("diff", diff);
	/*cv::Ptr<cv::BackgroundSubtractorMOG2> pBackSub = cv::createBackgroundSubtractorMOG2();

	pBackSub->apply(gray1, thresh, 1.0);
	pBackSub->apply(gray2, thresh, 0.0);*/

	double changedPixels = cv::countNonZero(thresh);
	double totalPixels = thresh.rows * thresh.cols;
	double changeRatio = changedPixels / totalPixels; 
	//cv::threshold(fgMask, fgMask, 100, 255, cv::THRESH_BINARY);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	cv::morphologyEx(thresh, thresh, cv::MORPH_OPEN, kernel);
	cv::morphologyEx(thresh, thresh, cv::MORPH_CLOSE, kernel);
	std::cout << "CHANGED % -> " << changeRatio << std::endl; 
	//cv::imshow("diff ", thresh);
	if (changeRatio > threshold && changeRatio < 0.8) {
		
		return true; 
	}
	if (changeRatio > 0.8) return false;
	
	return false;

}

void Preprocess::emptyGridMask(cv::Mat& original, cv::Mat& background) {
	//cv::Ptr<cv::BackgroundSubtractorMOG2> pBackSub = cv::createBackgroundSubtractorMOG2();

	//pBackSub->apply(background, fgMask, 1.0);
	//pBackSub->apply(original, fgMask, 0.0);

	//// * remove noise from the masked image
	//cv::threshold(fgMask, fgMask, 100, 255, cv::THRESH_BINARY);
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	//cv::morphologyEx(fgMask, fgMask, cv::MORPH_OPEN, kernel);
	//cv::morphologyEx(fgMask, fgMask, cv::MORPH_CLOSE, kernel);
	cv::Mat gray1, gray2;
	cv::cvtColor(background, gray1, cv::COLOR_BGR2GRAY);
	cv::cvtColor(original, gray2, cv::COLOR_BGR2GRAY);

	////cv::imshow("gray1", gray1);
	////cv::imshow("gray2",gray2);

	cv::GaussianBlur(gray1, gray1, cv::Size(5, 5), 0);
	cv::GaussianBlur(gray2, gray2, cv::Size(5, 5), 0);

	//// Absolute difference
	cv::Mat diff;
	cv::absdiff(gray1, gray2, diff);

	//// Threshold the difference
	cv::Mat thresh;
	cv::threshold(diff, thresh, 25, 255, cv::THRESH_BINARY);
	fgMask = thresh;
	cv::imshow("EMPTY GRID MASK", fgMask);
}


void Preprocess::generateEntityMask(cv::Mat& original, cv::Mat& backgroundGridImg) {
	
	/*cv::Ptr<cv::BackgroundSubtractorMOG2> pBackSub2 = cv::createBackgroundSubtractorMOG2();
	pBackSub2 -> apply(backgroundGridImg, entityMask, 1.0);
	pBackSub2 -> apply(original, entityMask, 0.0);*/
	
	cv::Mat diffBGR, diffGray, mask;

	// Absolute difference across all 3 color channels
	cv::absdiff(original, backgroundGridImg, diffBGR);
	cv::cvtColor(diffBGR, diffGray, cv::COLOR_BGR2GRAY);

	cv::threshold(diffGray, entityMask, 30, 255, cv::THRESH_BINARY);
	cv::morphologyEx(entityMask, entityMask, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1, -1), 1);

	cv::imshow("tette mask", diffGray);

}


int Preprocess::getOffset(Dir direction, Side side) {
	int length = (direction == X) ? fgMask.cols : fgMask.rows;
	int center = (direction == X) ? fgMask.rows : fgMask.cols;
	center = center / 2;

	for (int i = (side == LEFT || side == TOP) ? 0 : length - 1; (side == LEFT || side == TOP) ? i < length : i > 0; (side == LEFT || side == TOP) ? i++ : i--) {
		int x = (direction == X) ? i : center;
		int y = (direction == X) ? center : i;

		if (fgMask.at<uchar>(y, x) == 255)
			return (direction == X) ? x : y;
	}

	return 0;
}

void Preprocess::generateVertices() {
	offsetXL = getOffset(X, LEFT);
	offsetXR = getOffset(X, RIGHT);
	offsetYT = getOffset(Y, TOP);
	offsetYB = getOffset(Y, BOTTOM);


	// * get the four points of the rectangle (grid)
	// ! it should be calculated just once on the grid-only frame
	pt1 = cv::Point(offsetXL, offsetYT);							 // top left
	pt2 = cv::Point(fgMask.cols - offsetXR, offsetYB);				 // top right
	pt3 = cv::Point(offsetXL, fgMask.rows - offsetYB);				 // bottom left
	pt4 = cv::Point(fgMask.cols - offsetXR, fgMask.rows - offsetYB); // bottom right

	cv::circle(fgMask, pt1, 10, cv::Scalar(150));
	cv::circle(fgMask, pt4, 10, cv::Scalar(60));
	cv::circle(fgMask, pt2, 10, cv::Scalar(100));
	cv::circle(fgMask, pt3, 10, cv::Scalar(20));

	cv::imshow("AAAAAAAA", fgMask);
}


