#include "Preprocess.h"

Preprocess::~Preprocess() {}

void Preprocess::defineMedia() {
#if defined(_WIN32) || defined(_WIN64)
    loopVid = cv::VideoCapture("../../../dnd-cpippi/imgs/tetteFull.mp4");
#endif
#if defined(__APPLE__) || defined(__MACH__)
    loopVid = cv::VideoCapture("../imgs/tetteFull.mp4");
#endif
}

int Preprocess::detectColor(const cv::Vec3b& hsvColor) {
    int h = hsvColor[0];
    int s = hsvColor[1];
    int v = hsvColor[2];

    if (s < 50 || v < 50) {
        return TEST_NONE;  // Too gray or dark to classify
    }

    if ((h < 10) || (h > 160 && h <= 180))
        return TEST_RED;
    else if (h >= 35 && h < 85)
        return TEST_GREEN;
    else if (h >= 90 && h < 130)
        return TEST_BLUE;
    else
        return TEST_OTHER;
}


void Preprocess::findEntitties(cv::Mat& clothed, cv::Mat& nude) {
    cv::Mat copy = clothed.clone();

    // * convert color image to hsv to detect colors better
    cv::cvtColor(clothed, clothed, cv::COLOR_BGR2HSV);

    // * set the default values
    // ! this is done only once
    if (defaultPixelVer == 0 && defaultPixelHor == 0) {

        // * vertical/horizontal number of pixels of the image
        // ? since it's warped, they should be similar
        defaultPixelVer = nude.rows;
        defaultPixelHor = nude.cols;

        std::cout << "pixVer: " << defaultPixelVer << " pixHor: " << defaultPixelHor << std::endl;

        // * vertical/horizontal number of pixel for a single cell
        defaultCelHor = defaultPixelHor / defaultCols;
        defaultCelVer = defaultPixelVer / defaultRows;

        std::cout << "celHor: " << defaultCelHor << " celVer: " << defaultCelVer << std::endl;

        // * horizontal/vertical number of pixel for the patch
        defaultInCelHor = defaultCelHor / 3;
        defaultInCelVer = defaultCelVer / 3;

        std::cout << "inCelHor: " << defaultInCelHor << " inCelVer: " << defaultInCelVer << std::endl;

        // * horizontal/vertical number of pixel for the offset
        // ! we dont need it anymore
        defaultOffsetHor = (defaultCelHor - defaultInCelHor) / 2;
        defaultOffsetVer = (defaultCelVer - defaultInCelVer) / 2;

        std::cout << "offsetHor: " << defaultOffsetHor << " offsetVer: " << defaultOffsetVer << std::endl;

        // * matrix that contains the last valid positions
        // * this is the one we send to the app
        previousValidHoles = cv::Mat(defaultRows, defaultCols, CV_32F, TEST_NONE);

    }

    // * matrix of the positions in the frame
    cv::Mat holes(defaultRows, defaultCols, CV_32F, TEST_NONE);

    int halfInVer = defaultInCelVer / 2;
    int halfInHor = defaultInCelHor / 2;
    int halfCelHor = defaultCelHor / 2;
    int halfCelVer = defaultCelVer / 2;

    // Parallel loop
    cv::parallel_for_(cv::Range(0, defaultRows * defaultCols), [&](const cv::Range& range) {
        for (int index = range.start; index < range.end; ++index) {
            int currRow = index / defaultCols;
            int currCol = index % defaultCols;

            int i = currCol * defaultCelHor + halfCelHor;
            int j = currRow * defaultCelVer + halfCelVer;

            if (holes.at<float>(currRow, currCol) != TEST_NONE)
                continue;

            for (int dy = -halfInVer; dy <= halfInVer; ++dy) {
                int y = j + dy;
                if (y < 0 || y >= nude.rows) continue;

                for (int dx = -halfInHor; dx <= halfInHor; ++dx) {
                    int x = i + dx;
                    if (x < 0 || x >= nude.cols) continue;

                    int nudeValue = nude.at<uchar>(y, x);
                    if (nudeValue != 0) {
                        const cv::Vec3b& color = clothed.at<cv::Vec3b>(y, x);
                        holes.at<float>(currRow, currCol) = detectColor(color);
                        goto patchDone;
                    }
                }
            }
        patchDone:;
        }
    });


    // * checks if the previous valid matrix is different from the current frame matrix
    // * if different, then a change has occured and update the valid matrix
    cv::Mat holesDiff;
    cv::compare(previousValidHoles, holes, holesDiff, cv::CMP_NE);
    if (cv::countNonZero(holesDiff) > 0) {
        holes.copyTo(previousValidHoles);
        std::cout << previousValidHoles << std::endl;
    }

    //cv::imshow("", copy);
    //cv::waitKey(0);


}


cv::Mat Preprocess::elaborateFrame(cv::Mat& image) {

    cv::Mat gray, blur, thresh;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blur, cv::Size(5, 5), 0);

    cv::adaptiveThreshold(blur, thresh, 255, cv::ADAPTIVE_THRESH_MEAN_C,
        cv::THRESH_BINARY_INV, 15, 4);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(thresh.clone(), contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    double maxArea = 0;
    int maxIdx = -1;
    std::vector<cv::Point> bestApprox;

    for (int i = 0; i < contours.size(); ++i) {
        double area = cv::contourArea(contours[i]);
        if (area < 100) continue;

        std::vector<cv::Point> approx;
        cv::approxPolyDP(contours[i], approx, 0.02 * cv::arcLength(contours[i], true), true);

        if (approx.size() == 4 && cv::isContourConvex(approx)) {
            if (area > maxArea) {
                maxArea = area;
                maxIdx = i;
                bestApprox = approx;
            }
        }
    }

    if (maxIdx >= 0) {
        // Draw the largest square
        std::vector<std::vector<cv::Point>> drawContoursVec = { bestApprox };
        //cv::drawContours(image, drawContoursVec, 0, cv::Scalar(0, 255, 0), 2);

        // ----- Grid Estimation -----
        // Step 1: Order corners
        auto orderPoints = [](std::vector<cv::Point>& pts) -> std::vector<cv::Point2f> {
            std::vector<cv::Point2f> ordered(4);
            std::sort(pts.begin(), pts.end(), [](cv::Point a, cv::Point b) { return a.y < b.y; });

            if (pts[0].x < pts[1].x) {
                ordered[0] = pts[0]; // top-left
                ordered[1] = pts[1]; // top-right
            }
            else {
                ordered[0] = pts[1];
                ordered[1] = pts[0];
            }

            if (pts[2].x < pts[3].x) {
                ordered[3] = pts[2]; // bottom-left
                ordered[2] = pts[3]; // bottom-right
            }
            else {
                ordered[3] = pts[3];
                ordered[2] = pts[2];
            }

            return ordered;
        };

        std::vector<cv::Point2f> orderedPts = orderPoints(bestApprox);

        // Step 2: Warp the square to top-down view
        float side = 500.0f;
        std::vector<cv::Point2f> dstPts = {
            cv::Point2f(0, 0),
            cv::Point2f(side - 1, 0),
            cv::Point2f(side - 1, side - 1),
            cv::Point2f(0, side - 1)
        };

        cv::Mat M = cv::getPerspectiveTransform(orderedPts, dstPts);
        cv::Mat warped;
        cv::warpPerspective(image, warped, M, cv::Size(side, side));

        // Step 3: Enhance and threshold
        cv::Mat grayWarped, binary;
        cv::cvtColor(warped, grayWarped, cv::COLOR_BGR2GRAY);
        cv::adaptiveThreshold(grayWarped, binary, 255, cv::ADAPTIVE_THRESH_MEAN_C,
            cv::THRESH_BINARY_INV, 15, 4);


        // Step 4: Morphological line detection
        int morphSize = side / 20;  // adjust based on grid size

        // Horizontal lines
        cv::Mat hor_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(morphSize, 1));
        cv::Mat horizontal;
        cv::morphologyEx(binary, horizontal, cv::MORPH_OPEN, hor_kernel);




        // Vertical lines
        cv::Mat ver_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, morphSize));
        cv::Mat vertical;
        cv::morphologyEx(binary, vertical, cv::MORPH_OPEN, ver_kernel);

        // Step 5: Count contours
        std::vector<std::vector<cv::Point>> horContours, verContours;
        cv::findContours(horizontal, horContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        cv::findContours(vertical, verContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // Step 5: Filter and count meaningful lines (ignore short ones)
        int rowCount = -1;
        int colCount = -1;

        for (const auto& contour : horContours) {
            cv::Rect bbox = cv::boundingRect(contour);
            if (bbox.width > side * 0.5) { // horizontal line must be > 50% width
                rowCount++;
                // Optional: draw for debugging
                // cv::drawContours(warped, std::vector<std::vector<cv::Point>>{contour}, -1, cv::Scalar(255, 0, 0), 1);
            }
        }

        for (const auto& contour : verContours) {
            cv::Rect bbox = cv::boundingRect(contour);
            if (bbox.height > side * 0.5) { // vertical line must be > 50% height
                colCount++;
                // Optional: draw for debugging
                // cv::drawContours(warped, std::vector<std::vector<cv::Point>>{contour}, -1, cv::Scalar(0, 0, 255), 1);
            }
        }

        if (defaultRows == -1 && defaultCols == -1) {
            defaultRows = rowCount;
            defaultCols = colCount;
        }


        //std::cout << "Rows: " << rowCount << std::endl;
        //std::cout << "Cols: " << colCount << std::endl;


        if (rowCount == defaultRows && colCount == defaultCols) {

            cv::Mat bra;
            cv::absdiff(binary, horizontal, bra);
            cv::absdiff(bra, vertical, bra);

            int boobsCup = 4;
            cv::Mat takeItOff = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(boobsCup, boobsCup));
            cv::morphologyEx(bra, bra, cv::MORPH_OPEN, takeItOff);
            cv::medianBlur(bra, bra, 3); // Kernel size must be odd

            findEntitties(warped, bra);

        }


        // Optionally draw warped image for debugging
        // cv::imshow("Warped", warped);
        // cv::imshow("Horizontal Lines", horizontal);
        // cv::imshow("Vertical Lines", vertical);
    }

    return image;
}


void Preprocess::boobs() {

    defineMedia();
    cv::Mat frame;
    while (true) {
        loopVid.read(frame);
        if (frame.empty()) break;

        cv::Mat uu = elaborateFrame(frame);

        //cv::imshow("frame", uu);
        //int key = cv::waitKey(0);
        //if (key == 'q') break;
    }

}
