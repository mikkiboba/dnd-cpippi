// HCI.cpp: definisce il punto di ingresso dell'applicazione.
//
#include <opencv2/opencv.hpp> 
#include <vector>

#include "HCI.h"

/* 
* notes:
* 1. firstly, we need a frame with the static background (to subtract the background)
* 2. secondly, we need a frame with the only the grid (to subtract the grid)
* 3. then we can work on the single frames
*/


// TODO: cv::createBackgroundSubtractorMOG2() or cv::createBackgroundSubtractorKNN()

// * images loaded in cv::Mat data structures

cv::Mat background;
cv::Mat frame;
cv::Mat frame2;
cv::Mat backgroundGrid;



void define_imgs() {

	#if defined(_WIN32) || defined(_WIN64)
		background = cv::imread("../../../dnd-cpippi/imgs/background.jpg");
		frame2 = cv::imread("../../../dnd-cpippi/imgs/backgroundGridPiedini.png");
		frame = cv::imread("../../../dnd-cpippi/imgs/backgroundGridPiedini2.png");
		backgroundGrid = cv::imread("../../../dnd-cpippi/imgs/backgroundGrid.png");
	#endif

	#if defined(__APPLE__) || defined(__MACH__)
		background = cv::imread("../imgs/background.jpg");
		frame2 = cv::imread("../imgs/backgroundGridPiedini.png");
		frame = cv::imread("../imgs/backgroundGridPiedini2.png");
		backgroundGrid = cv::imread("../imgs/backgroundGrid.png");
	#endif

}


// * temporal struct to record the info about a pawn on screen
struct Entity{
	cv::Vec2i screenPosition;
	cv::Vec2i gridPosition;
	cv::Vec3b color;
};

std::vector<Entity> entities;
std::set<std::pair<int, int>> occupiedGridCells;

// * to show the image on opencv fastly
void showImg(cv::Mat img) {
	cv::imshow("Image", img);
	cv::waitKey(0);
}


// * find the dimension of the grid matrix, based on the direction specified
int getDim(cv::Mat img, bool dirX, cv::Point pt) {
	int cellShift = 20;
	int count = 0;
	int length = dirX ? img.cols : img.rows; 

	int x = pt.x + cellShift;
	int y = pt.y + cellShift;

	for (int i = 1; i < length; i++) {
		int currX = x + (dirX ? i : 0);
		int currY = y + (dirX ? 0 : i);
		int precX = x + (dirX ? i-1 : 0);
		int precY = y + (dirX ? 0 : i-1);

		if (currX >= length || currY >= length) break;

		uchar currPixel = img.at<uchar>(currY, currX);
		uchar precPixel = img.at<uchar>(precY, precX);

		if (currPixel == 255 && precPixel == 0) count++;
	}
	std::cout << (dirX ? "Number of col lines " : "Number of row lines ") << count << std::endl;

	return count;
}


// ! funziona me è sus
void findElementsInLine(cv::Mat img, int start1, int start2, int end1, int end2, int cellSize, int offsetX, int offsetY) {

	int halfCellSize = cellSize / 2;

	for (int i = start1 + halfCellSize; i < end1; i += cellSize) {
		for (int j = start2; j < end2; j++) {
			uchar currPixel = img.at<uchar>(i, j);
	
			int gridX = std::ceil(static_cast<double>((j - offsetX)) / cellSize);
			int gridY = std::ceil(static_cast<double>((i - offsetY)) / cellSize);
			std::pair<int, int> gridCell(gridX, gridY);
	
			if (occupiedGridCells.count(gridCell)) continue;
	
			if (currPixel == 255) {
				cv::Vec3b pixelColor = frame.at<cv::Vec3b>(i, j);
				Entity e = {cv::Point2i(j, i), cv::Point2i(gridX, gridY), pixelColor};
				entities.push_back(e);
				occupiedGridCells.insert(gridCell);
			}
		}
	}

}



// * Get the offset on the image, based on the direction specified, from the broder to the first element
int getOffset(cv::Mat img, bool dirX) {
	int length = dirX ? img.cols : img.rows; 
	int center = dirX ? img.rows : img.cols;
	center = center / 2;

	for (int i = 0; i < length; i++) {	
		int x = dirX ? i : center;
		int y = dirX ? center : i;

		if (img.at<uchar>(y, x) == 255) 
			return dirX ? x : y;
	}

	return 0;
}


int main()
{
	define_imgs();
	// * checks if the images are loaded in the system
	// TODO: È un TEST, poi dovranno essere sostituite in qualche modo da quello che prende la webcam
	if (background.empty() || frame.empty() || backgroundGrid.empty()) {
		std::cerr << "Could not load images!" << std::endl;
		return -1;
	}

	// * subtract the background to every frame
	// ! the background must be static
	// TODO: È un TEST, poi questo dovrà essere fatto ad ogni frame
	cv::Ptr<cv::BackgroundSubtractorMOG2> pBackSub = cv::createBackgroundSubtractorMOG2();

	cv::Mat fgMask;

	pBackSub -> apply(background, fgMask, 1.0);
	pBackSub -> apply(frame, fgMask, 0.0);

	// * remove noise from the masked image
	cv::threshold(fgMask, fgMask, 100, 255, cv::THRESH_BINARY);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	cv::morphologyEx(fgMask, fgMask, cv::MORPH_OPEN, kernel);
	cv::morphologyEx(fgMask, fgMask, cv::MORPH_CLOSE, kernel);

	// * calculate the offset from the grid to the sides of the image
	// ! this implies that the grid is centered on the frame (the offset is the same on left-right/top-bottom) but it shouldn't always be the case
	// ! it should be calculated just once on the grid-only frame
	int offsetX = getOffset(fgMask, 1);
	int offsetY = getOffset(fgMask, 0);

	// * get the four points of the rectangle (grid)
	// ! it should be calculated just once on the grid-only frame
	cv::Point pt1(offsetX, 				offsetY);							// top left
	cv::Point pt2(fgMask.cols-offsetX, 	offsetY);							// top right
	cv::Point pt3(offsetX, 				fgMask.rows-offsetY);				// bottom left
	cv::Point pt4(fgMask.cols-offsetX, 	fgMask.rows-offsetY);				// bottom right

	// * get the rows and columns of the matrix
	int countRows = getDim(fgMask, 0, pt1);
	int countCols = getDim(fgMask, 1, pt1);

	// * subtract the grid from the frame to show only the "physical" elements
	cv::Ptr<cv::BackgroundSubtractorMOG2> pBackSub2 = cv::createBackgroundSubtractorMOG2();
	pBackSub2 -> apply(backgroundGrid, fgMask, 1.0);
	pBackSub2 -> apply(frame, fgMask, 0.0);

	// * hor and ver length of the grid 
	int hLen = fgMask.cols - (2*offsetX);
	int vLen = fgMask.rows - (2*offsetY);

	// * size of a cell
	int cellSize = hLen / countCols;
	int halfCellSize = cellSize / 2;

	// * here we detect elements in the cells
	//findElementsInLine(fgMask, pt1.x, pt1.y, pt3.y, pt2.x, cellSize, offsetX, offsetY);
	//findElementsInLine(fgMask, pt1.y, pt1.x, pt2.x, pt3.y, cellSize, offsetX, offsetY);

	for (int row = pt1.y + halfCellSize; row < pt3.y; row += cellSize) {
		for (int i = pt1.x; i < pt2.x; i++) {
			uchar currPixel = fgMask.at<uchar>(row, i);

			int gridX = std::ceil(static_cast<double>((i - offsetX)) / cellSize);
			int gridY = std::ceil(static_cast<double>((row - offsetY)) / cellSize);
			std::pair<int, int> gridCell(gridX, gridY);

			if (occupiedGridCells.count(gridCell)) continue;

			if (currPixel == 255) {
				cv::Vec3b pixelColor = frame.at<cv::Vec3b>(row, i);
				Entity e = {cv::Point2i(i, row), cv::Point2i(gridX, gridY), pixelColor};
				entities.push_back(e);
				occupiedGridCells.insert(gridCell);
			}
		}
	}

	for (int col = pt1.x + halfCellSize; col < pt2.x; col += cellSize) {
		for (int i = pt1.y; i < pt3.y; i++) {
			uchar currPixel = fgMask.at<uchar>(i, col);

			int gridX = std::ceil(static_cast<double>(col - offsetX) / cellSize);
			int gridY = std::ceil(static_cast<double>(i - offsetY) / cellSize);
			std::pair<int, int> gridCell(gridX, gridY);

			if (occupiedGridCells.count(gridCell)) continue;

			if (currPixel == 255) {
				cv::Vec3b pixelColor = frame.at<cv::Vec3b>(i, col);

				Entity e = {cv::Point2i(col, i), cv::Point2i(gridX, gridY), pixelColor};
				entities.push_back(e);
				occupiedGridCells.insert(gridCell);
			}
		}
	}

	for (const Entity& e : entities) {
		std::cout << e.gridPosition << std::endl;
		circle(frame, cv::Point(e.screenPosition[0], e.screenPosition[1]), 5.0, cv::Scalar(255, 0, 255), -1, 8);
	}

	return 0;
}