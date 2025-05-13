#include "Preprocess.h"

Preprocess::~Preprocess() {}


bool Preprocess::isHandPresent(cv::Mat referenceFrame, cv::Mat currentFrame) {
	cv::Mat diff, gray;
	cv::absdiff(referenceFrame, currentFrame, diff);
	cv::cvtColor(diff, gray, cv::COLOR_BGR2GRAY);
	cv::threshold(gray, gray, 30, 225, cv::THRESH_BINARY);
	double motion = cv::sum(gray)[0] / 255;
	return motion > 5000;
}


bool Preprocess::isChanged(cv::Mat& referenceFrame, cv::Mat& currentFrame, double threshold) {

	//emptyGridMask(referenceFrame, currentFrame);
	cv::Mat gray1, gray2;

	cv::cvtColor(referenceFrame, gray1, cv::COLOR_BGR2GRAY);
	cv::cvtColor(currentFrame, gray2, cv::COLOR_BGR2GRAY);

	cv::GaussianBlur(gray1, gray1, cv::Size(5, 5), 0);
	cv::GaussianBlur(gray2, gray2, cv::Size(5, 5), 0);


	// Absolute difference
	cv::Mat diff;
	cv::absdiff(gray1, gray2, diff);


	// Threshold the difference
	cv::Scalar meanVal, stddevVal;
    cv::meanStdDev(diff, meanVal, stddevVal);
    double threshVal = meanVal[0] + .5 * stddevVal[0]; // Can tune the multiplier
	cv::Mat thresh;
	cv::threshold(diff, thresh, 25, 255, cv::THRESH_BINARY);

	// * ho spostato questo prima - mikki
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	cv::morphologyEx(thresh, thresh, cv::MORPH_OPEN, kernel);
	cv::morphologyEx(thresh, thresh, cv::MORPH_CLOSE, kernel);

		cv::imshow("",thresh);


	double changedPixels = cv::countNonZero(thresh);
	double totalPixels = thresh.rows * thresh.cols;
	double changeRatio = changedPixels / totalPixels; 


	if (changeRatio > threshold && changeRatio < 0.1) {
		std::cout << "CHANGED % -> " << changeRatio << std::endl; 
		return true;
	}
	return false;

	return (changeRatio > threshold && changeRatio < 0.15); // questo return dovrebbe bastare
}


// * generate the fgMask by subtracting the background to the original image (grid)
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
	cv::threshold(diff, thresh, 35, 255, cv::THRESH_BINARY); 	// ho aumentato il threshol da 25 a 35 - mikki
	fgMask = thresh;
	
	// cv::imshow("EMPTY GRID MASK", fgMask);
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

	// cv::imshow("tette mask", diffGray);

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


// * find the position and generate the 4 vertices of the grid
void Preprocess::generateVertices() {
	offsetXL = getOffset(X, LEFT);
	offsetXR = getOffset(X, RIGHT);
	offsetYT = getOffset(Y, TOP);
	offsetYB = getOffset(Y, BOTTOM);


	// * get the four points of the rectangle (grid)
	pt1 = cv::Point(offsetXL, offsetYT);	// top left
	pt2 = cv::Point(offsetXR, offsetYT);	// top right
	pt3 = cv::Point(offsetXL, offsetYB);	// bottom left
	pt4 = cv::Point(offsetXR, offsetYB); 	// bottom right

	// visualizeVertices()
}


// * test: visualize the vertices on a copy of fgMask
void Preprocess::visualizeVertices() {
	cv::Mat fgMaskCpy = fgMask;
	cv::circle(fgMaskCpy, pt1, 10, cv::Scalar(150));
	cv::circle(fgMaskCpy, pt4, 10, cv::Scalar(60));
	cv::circle(fgMaskCpy, pt2, 10, cv::Scalar(100));
	cv::circle(fgMaskCpy, pt3, 10, cv::Scalar(20));

	cv::imshow("AAAAAAAA", fgMaskCpy);
}


