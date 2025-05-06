// HCI.cpp: definisce il punto di ingresso dell'applicazione.
//
#include <opencv2/opencv.hpp> 

#include "HCI.h"


// TODO: cv::createBackgroundSubtractorMOG2() or cv::createBackgroundSubtractorKNN()

struct Pedina{
	int id;
	cv::Vec2i position;
	cv::Vec3i color;
};

int getDim(cv::Mat img, bool xy, cv::Point pt) {
	int cellShift = 20;
	int count = 0;
	int length = (xy == 0) ? img.cols : img.rows; 
	uchar prec;
	for (int i = 0; i<length; i++) {
		if (xy==0) {
			pt.y = pt.y + 1;
			prec = img.at<uchar>(pt.y - 1 + cellShift, pt.x+cellShift);
		}
		else {
			pt.x = pt.x + 1;
			prec = img.at<uchar>(pt.y + cellShift, pt.x -1 +cellShift);
		}
		uchar pixelVal = img.at<uchar>(pt.y + cellShift, pt.x+cellShift);		
		if (static_cast<int>(pixelVal) == 255 && static_cast<int>(prec)==0) {
			count++;		
		}
		
	}
	if (xy==0) {std::cout << "Number of row lines " << count << std::endl;}
	else {std::cout << "Number of col lines " << count << std::endl;}
	
	return count;

}

int getOffset(cv::Mat img, bool xy) {
	int length = (xy == 0) ? img.cols : img.rows; 
	int center = (xy == 0) ? img.rows : img.cols;
	center = round(center/2);
	cv::Point pos(0,0);
	for (int i = 0; i < length; i++) {	
		if (xy==1){ pos = cv::Point(center,i);}
		else pos = cv::Point(i, center);
		uchar pixelVal = img.at<uchar>(pos.y, pos.x);		
		if (static_cast<int>(pixelVal) == 255) {
			if (xy == 0) return pos.x;
			return pos.y;
		}
	}
	return 0;
}


void showImg(cv::Mat img) {
	cv::imshow("Image", img);
	cv::waitKey(0);
}


int main()
{
	cv::Vec2i inizioBordo = NULL;

	cv::Mat background = cv::imread("../imgs/background.jpg");
	cv::Mat frame = cv::imread("../imgs/backgroundGridPiedini.png");

	if (background.empty() || frame.empty()) {
		std::cerr << "Could not load images!" << std::endl;
		return -1;
	}

	cv::Ptr<cv::BackgroundSubtractorMOG2> pBackSub = cv::createBackgroundSubtractorMOG2();

	cv::Mat fgMask;

	pBackSub -> apply(background, fgMask, 1.0);
	pBackSub -> apply(frame, fgMask, 0.0);

	cv::threshold(fgMask, fgMask, 100, 255, cv::THRESH_BINARY);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	cv::morphologyEx(fgMask, fgMask, cv::MORPH_OPEN, kernel);
	cv::morphologyEx(fgMask, fgMask, cv::MORPH_CLOSE, kernel);

	int offsetX = 0;
	int offsetY = 0;

	offsetX = getOffset(fgMask, 0);
	offsetY = getOffset(fgMask, 1);

	cv::Point pt1(offsetX, offsetY);
	cv::Point pt2(fgMask.cols-offsetX, offsetY);
	cv::Point pt3(offsetX, fgMask.rows-offsetY);
	cv::Point pt4(fgMask.cols-offsetX, fgMask.rows-offsetY);

	/*
	cv::circle(frame, pt1, 20, cv::Scalar(255, 0, 0), -1);
	cv::circle(frame, pt2, 20, cv::Scalar(255, 0, 0), -1);
	cv::circle(frame, pt3, 20, cv::Scalar(255, 0, 0), -1);
	cv::circle(frame, pt4, 20, cv::Scalar(255, 0, 0), -1);*/
	showImg(fgMask);

	int countRows = getDim(fgMask, 0,pt1);
	int countCols = getDim(fgMask, 1,pt1);

	cv::Mat backgroundGrid = cv::imread("../imgs/backgroundGrid.png");
	cv::Ptr<cv::BackgroundSubtractorMOG2> pBackSub2 = cv::createBackgroundSubtractorMOG2();
	pBackSub2 -> apply(backgroundGrid, fgMask, 1.0);
	pBackSub2 -> apply(frame, fgMask, 0.0);


	int hLen = fgMask.cols - (2*offsetX);
	int vLen = fgMask.rows - (2*offsetY);

	int cellSize = round(hLen / countCols);

	int caccount = 0;
	int pipiount = 0;
	for (int row = pt1.y; row < pt3.y; row+=cellSize) {
		caccount++;
		for (int col = pt1.x; col < pt2.x; col+=cellSize) {
			pipiount++;
			bool found = false;
			for (int i = row; i < row+cellSize; i++) {
				int selectedPixel = static_cast<int>(fgMask.at<uchar>(i, row+round(cellSize/2)));
				cv::circle(frame, cv::Point(i, row+round(cellSize/2)), 5, cv::Scalar(255, 0, 0), -1);
				if (selectedPixel == 255) {
					cv::circle(frame, cv::Point(i, col+round(cellSize/2)), 20, cv::Scalar(255, 0, 0), -1);
					std::cout << static_cast<cv::Point3d>(frame.at<uchar>(i, col+round(cellSize/2))) << std::endl;
					found = true;
					showImg(frame);
					return 0;
				}
			}

			if(!found) {
				for (int i = col; i < col+cellSize; i++) {
					int selectedPixel = static_cast<int>(fgMask.at<uchar>(i, row+round(cellSize/2)));
					cv::circle(frame, cv::Point(i, col+round(cellSize/2)), 5, cv::Scalar(255, 0, 0), -1);
					if (selectedPixel == 255) {
						cv::circle(frame, cv::Point(i, row+round(cellSize/2)), 20, cv::Scalar(255, 0, 0), -1);
						std::cout << static_cast<cv::Point>(frame.at<uchar>(i, row+round(cellSize/2))) << std::endl;
						found = true;
						showImg(frame);
						return 0;
					}
				}
			}

		}
	}

    return 0;
}