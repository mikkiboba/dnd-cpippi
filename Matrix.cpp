#include "Matrix.h"

#include <opencv2/opencv.hpp>


Matrix::~Matrix() {}


void Matrix::printEntities() {
    for (const Entity& e : getEntities()) {
		std::cout << e.gridPosition << std::endl;
    }
}


void Matrix::initPreprocess(cv::Mat& original, cv::Mat& background) {
    preprocess.emptyGridMask(original, background);
    preprocess.generateVertices();
    
} 

void Matrix::preprocessEntities(cv::Mat& original, cv::Mat& background) {
    preprocess.generateEntityMask(original, background);
}

int Matrix::getDim(bool dirX) {
    int cellShift = 20;
    int count = 0;
    int length = dirX ? preprocess.getFgMask().cols : preprocess.getFgMask().rows;

    int x = preprocess.getPt1().x + cellShift;
    int y = preprocess.getPt1().y + cellShift;

    for (int i = 1; i < length; i++) {
        int currX = x + (dirX ? i : 0);
        int currY = y + (dirX ? 0 : i);
        int precX = x + (dirX ? i - 1 : 0);
        int precY = y + (dirX ? 0 : i - 1);

        if (currX >= length || currY >= length) break;

        uchar currPixel = preprocess.getFgMask().at<uchar>(currY, currX);
        uchar precPixel = preprocess.getFgMask().at<uchar>(precY, precX);

        if (currPixel == 255 && precPixel == 0) count++;
    }
    std::cout << (dirX ? "Number of col lines " : "Number of row lines ") << count << std::endl;

    return count;
}

void Matrix::findElementsInLine(cv::Mat original) {
    // * hor and ver length of the grid 

    int hLen = preprocess.getFgMask().cols - (2*preprocess.getOffset(1));
    int vLen = preprocess.getFgMask().rows - (2*preprocess.getOffset(0));

    // * size of a cell
    countCols = getDim(1);


    int cellSize = hLen / countCols;
    int halfCellSize = cellSize / 2;

    // * here we detect elements in the cells
    for (int row = preprocess.getPt1().y + halfCellSize; row < preprocess.getPt3().y; row += cellSize) {
        for (int i = preprocess.getPt1().x; i < preprocess.getPt2().x; i++) {
            uchar currPixel = preprocess.getEntityMask().at<uchar>(row, i);
            int gridX = std::ceil(static_cast<double>((i - preprocess.getOffset(1))) / cellSize);
            int gridY = std::ceil(static_cast<double>((row - preprocess.getOffset(0))) / cellSize);
            std::pair<int, int> gridCell(gridX, gridY);
            if (occupiedGridCells.count(gridCell)) continue;
            if (currPixel == 255) {
                cv::Vec3b pixelColor = original.at<cv::Vec3b>(row, i);
                occupiedGridCells.insert(gridCell);
                Entity e = {cv::Point2i(i, row), cv::Point2i(gridX, gridY), pixelColor};
                std::cout << "Color of entity in " << e.gridPosition << " = " << detectColor(pixelColor) << std::endl;
                insertEntity(e);
            }
        }
    }

    for (int col = preprocess.getPt1().x + halfCellSize; col < preprocess.getPt2().x; col += cellSize) {
        for (int i = preprocess.getPt1().y; i < preprocess.getPt3().y; i++) {
            uchar currPixel = preprocess.getEntityMask().at<uchar>(i, col);

            int gridX = std::ceil(static_cast<double>(col - preprocess.getOffset(1)) / cellSize);
            int gridY = std::ceil(static_cast<double>(i - preprocess.getOffset(0)) / cellSize);
            std::pair<int, int> gridCell(gridX, gridY);

            if (occupiedGridCells.count(gridCell)) continue;

            if (currPixel == 255) {
                cv::Vec3b pixelColor = original.at<cv::Vec3b>(i, col);
                occupiedGridCells.insert(gridCell);
                Entity e = {cv::Point2i(col, i), cv::Point2i(gridX, gridY), pixelColor};
                std::cout << "Color of entity in " << e.gridPosition << " = " << detectColor(pixelColor) << std::endl;
				insertEntity(e);
            }
        }
    }
}

int Matrix::detectColor(const cv::Vec3b& color) {
    uchar b = color[0];
    uchar g = color[1];
    uchar r = color[2];

    if (r > 200 && g < 100 && b < 100)
        return RED;
    else if (g > 200 && r < 100 && b < 100)
        return GREEN;
    else if (b > 200 && r < 100 && g < 100)
        return BLUE;
    else if (r > 200 && g > 200 && b < 100)
        return YELLOW;
    else if (g > 200 && b > 200 && r < 100)
        return CYAN;
    else if (r > 200 && b > 200 && g < 100)
        return MAGENTA;
    else if (r > 200 && g > 100 && g < 180 && b < 100)
        return ORANGE;
    else if (r > 100 && r < 180 && g > 100 && g < 180 && b > 100 && b < 180)
        return GRAY;
    else if (r > 100 && b > 100 && g < 100)
        return PURPLE;
    else if (r > 200 && g > 200 && b > 200)
        return WHITE;
    else if (r < 50 && g < 50 && b < 50)
        return BLACK;
    else
        return OTHER;
}
