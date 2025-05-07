#include "Preprocess.h"

void Preprocess::emptyGridMask() {
	cv::Ptr<cv::BackgroundSubtractorMOG2> pBackSub = cv::createBackgroundSubtractorMOG2();

	pBackSub->apply(original, fgMask, 1.0);
	pBackSub->apply(original, fgMask, 0.0);

	// * remove noise from the masked image
	cv::threshold(fgMask, fgMask, 100, 255, cv::THRESH_BINARY);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	cv::morphologyEx(fgMask, fgMask, cv::MORPH_OPEN, kernel);
	cv::morphologyEx(fgMask, fgMask, cv::MORPH_CLOSE, kernel);
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
	cv::Point pt1(offsetX, offsetY);							 // top left
	cv::Point pt2(fgMask.cols - offsetX, offsetY);				 // top right
	cv::Point pt3(offsetX, fgMask.rows - offsetY);				 // bottom left
	cv::Point pt4(fgMask.cols - offsetX, fgMask.rows - offsetY); // bottom right
}

int Preprocess::getDim(bool dirX) {
	int cellShift = 20;
	int count = 0;
	int length = dirX ? fgMask.cols : fgMask.rows;

	int x = pt1.x + cellShift;
	int y = pt1.y + cellShift;

	for (int i = 1; i < length; i++) {
		int currX = x + (dirX ? i : 0);
		int currY = y + (dirX ? 0 : i);
		int precX = x + (dirX ? i - 1 : 0);
		int precY = y + (dirX ? 0 : i - 1);

		if (currX >= length || currY >= length) break;

		uchar currPixel = fgMask.at<uchar>(currY, currX);
		uchar precPixel = fgMask.at<uchar>(precY, precX);

		if (currPixel == 255 && precPixel == 0) count++;
	}
	std::cout << (dirX ? "Number of col lines " : "Number of row lines ") << count << std::endl;

	return count;
}