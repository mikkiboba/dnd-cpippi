#include <opencv2/opencv.hpp>


cv::VideoCapture bgVid;
cv::Mat frameBg;
cv::VideoCapture gridVid;
cv::Mat frameGrid;
cv::VideoCapture loopVid;




bool isLargeQuadrilateral(const std::vector<cv::Point>& contour, std::vector<cv::Point>& approx, double minArea = 1000.0) {
    cv::approxPolyDP(contour, approx, cv::arcLength(contour, true) * .02, true);
    return (approx.size() == 4 && std::fabs(cv::contourArea(approx)) > minArea && cv::isContourConvex(approx));
}


cv::Mat thiccThighs(cv::Mat& frame, cv::Mat& backgroundGridImg) {

    cv::Mat gray, gayGrid;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(backgroundGridImg, gayGrid, cv::COLOR_BGR2GRAY);

    cv::Mat blurred, gayBlurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);
    cv::GaussianBlur(gayGrid, gayBlurred, cv::Size(5, 5), 0);

    cv::Mat diffGray;
    cv::absdiff(gray, gayGrid, diffGray);

    cv::Mat thresh;
    cv::threshold(diffGray, thresh, 25, 255, cv::THRESH_BINARY);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	cv::morphologyEx(thresh, thresh, cv::MORPH_OPEN, kernel);
	cv::morphologyEx(thresh, thresh, cv::MORPH_CLOSE, kernel);

    cv::Mat edges;
    cv::Canny(thresh, edges, 50, 150);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    double maxArea = 0;
    std::vector<cv::Point> biggestQuad;

    for (const std::vector<cv::Point>& contour : contours) {
        std::vector<cv::Point> approx;
        if (isLargeQuadrilateral(contour, approx)) {
            double area = cv::contourArea(approx);
            if (area > maxArea) {
                maxArea = area;
                biggestQuad = approx;
            }
        }
    }

    if (biggestQuad.size() == 4) {
        // Draw the quadrilateral
        cv::polylines(frame, biggestQuad, true, cv::Scalar(0, 255, 0), 3);

        for (int i = 0; i < 4; ++i) {
            cv::circle(frame, biggestQuad[i], 5, cv::Scalar(0, 0, 255), -1);
            cv::putText(frame, std::to_string(i), biggestQuad[i], cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);
        }
    }

    return frame;

}


void defineMedia() {
    
    #if defined(_WIN32) || defined(_WIN64)
        bgVid = cv::VideoCapture("../../../dnd-cpippi/imgs/bgtette.mp4");
    #endif
    #if defined(__APPLE__) || defined(__MACH__)
        bgVid = cv::VideoCapture("../imgs/bgtette.mp4");
    #endif
    bgVid.read(frameBg);
    #if defined(_WIN32) || defined(_WIN64)
        gridVid = cv::VideoCapture("../../../dnd-cpippi/imgs/tetteGrid.mp4");
    #endif
    #if defined(__APPLE__) || defined(__MACH__)
        gridVid = cv::VideoCapture("../imgs/tetteGrid.mp4");
    #endif
    gridVid.read(frameGrid);
    #if defined(_WIN32) || defined(_WIN64)
        loopVid = cv::VideoCapture("../../../dnd-cpippi/imgs/tetteFull.mp4");
    #endif
    #if defined(__APPLE__) || defined(__MACH__)
        loopVid = cv::VideoCapture("../imgs/tetteFull.mp4");
    #endif
}

int defaultRows = -1;
int defaultCols = -1;

int defaultPixelHor = 0;
int defaultPixelVer = 0;

int defaultCelHor = 0;
int defaultCelVer = 0;

int defaultInCelHor = 0;
int defaultInCelVer = 0;

int defaultOffsetHor = 0;
int defaultOffsetVer = 0;

int gridRows = 0;
int gridCols = 0;

cv::Vec3b defaultColor = cv::Vec3b(0,0,0);


int count = 0;


std::pair<int, int> transitionToGridCoordinates(int& x, int& y) {

    int col = x / defaultCelHor;
    int row = y / defaultCelVer;

    return {row, col};

}


enum TestColors {
    TEST_RED = 1,
    TEST_GREEN = 2,
    TEST_BLUE = 3,
    TEST_OTHER = 4,
    TEST_NONE = 0
};

int detectColor(const cv::Vec3b& hsvColor) {
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


cv::Mat previousValidHoles;


void findEntitties(cv::Mat& clothed, cv::Mat& nude) {
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

    // * iterations on the cells
    for (int i = defaultCelHor / 2; i <= defaultPixelHor - (defaultCelHor / 2); i += defaultCelHor) {
        for (int j = defaultCelVer / 2; j <= defaultPixelVer - (defaultCelVer / 2); j += defaultCelVer) {
            
            // * cell position
            int currRow = j / defaultCelVer;
            int currCol = i / defaultCelHor;

            // * skips if the cell is already marked
            if (holes.at<float>(currRow, currCol) != TEST_NONE)
                continue;

            // * iteration on the patch
            for (int dy = -defaultInCelVer/2; dy <= defaultInCelVer/2; ++dy) {
                for (int dx = -defaultInCelHor/2; dx <= defaultInCelHor/2; ++dx) {
                    int x = i + dx;
                    int y = j + dy;

                    // debug: show the cells in copy
                    cv::circle(copy, cv::Point(x, y), 1, cv::Scalar(0, 255, 0), 1);

                    // * checks if it's a valid position
                    if (x >= 0 && x < nude.cols && y >= 0 && y < nude.rows) {
                        int nudeValue = (int)nude.at<uchar>(y, x);

                        // * if there is a non-black value there must be something in the hsv image
                        if (nudeValue != 0) {
                            cv::Vec3b color = clothed.at<cv::Vec3b>(y, x);
                            holes.at<float>(currRow, currCol) = detectColor(color);

                            // * thanks chatgpt, i didn't know i could exit from loops like this
                            goto nextCell;
                        }
                    }
                }
            }

            nextCell:;
        }
    }

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

    /*
    for (int i = defaultCelHor/2; i <= defaultPixelHor - (defaultCelHor/2); i += defaultCelHor) {
        for (int j = defaultCelVer/2; j <= defaultPixelVer - (defaultCelVer/2); j += defaultCelVer){

            int currRow = (j / defaultCelVer);
            int currCol = (i / defaultCelHor);

            int y = j - defaultInCelVer/2;
            while (y <= j + defaultInCelVer/2) {
                if (holes.at<float>(currRow, currCol) != TEST_NONE)
                    break;

                if (y >= 0 && y < nude.rows && i >= 0 && i < nude.cols) {
                    int nudeValue = (int)nude.at<uchar>(y, i);
                    if (nudeValue != 0) {
                        cv::Vec3b color = clothed.at<cv::Vec3b>(y, i);
                        holes.at<float>(currRow, currCol) = detectColor(color);
                        break;
                    }
                }

                cv::circle(copy, cv::Point(i, y), 1, cv::Scalar(0,255,0), 1);
                y++;
            }
        }
    }

    for (int j = defaultCelVer/2; j <= defaultPixelVer - (defaultCelVer/2); j += defaultCelVer){
        for (int i = defaultCelHor/2; i <= defaultPixelHor - (defaultCelHor/2); i += defaultCelHor) {

            int currRow = (j / defaultCelVer);
            int currCol = (i / defaultCelHor);

            int x = i - defaultInCelHor/2;
            while (x <= i + defaultInCelVer/2) {
                if (holes.at<float>(currRow, currCol) != TEST_NONE)
                    break;

                if (x >= 0 && x < nude.rows && j >= 0 && j < nude.cols) {
                    int nudeValue = (int)nude.at<uchar>(j, x);
                    if (nudeValue != 0) {
                        cv::Vec3b color = clothed.at<cv::Vec3b>(j, x);
                        //std::cout << color << std::endl;
                        holes.at<float>(currRow, currCol) = detectColor(color);
                        break;
                    }
                }

                cv::circle(copy, cv::Point(x, j), 1, cv::Scalar(0,255,0), 1);
                x++;
            }
        }
    }

    cv::Mat holesDiff;
    cv::compare(previousValidHoles, holes, holesDiff, cv::CMP_NE);
    if (cv::countNonZero(holesDiff) > 0) {
        holes.copyTo(previousValidHoles);
        std::cout << previousValidHoles << std::endl;
    }


    /*for (int i = 0; i < defaultCols; i++) {
        for (int j = 0; j < defaultRows; j++) {
            float c = holes.at<float>(j, i);
            if (c != TEST_NONE)
                std::cout << c << " (" << i << "," << j << ")" << std::endl;
        }
    }

    
    for (int i = defaultOffsetHor; i < defaultPixelHor - defaultOffsetHor - defaultInCelHor; i += defaultInCelHor + 2*defaultOffsetHor) {
        for (int j = defaultOffsetVer + static_cast<int>(defaultInCelVer/2); j < defaultPixelVer - defaultOffsetVer - defaultInCelHor; i += defaultInCelVer + 2*defaultOffsetVer) {

            bool found = false;

            int x = i;
            int y = j;

            std::cout << x << "," << y << std::endl;
            exit(0);

            while(y <= i+defaultInCelVer && !found) {

                int puntoG = nude.at<int>(y, x);

                if (puntoG != 0) {
                    found = true;

                    cv::Vec3i color = clothed.at<cv::Vec3i>(y, x);
                    holes.at<cv::Vec3i>(y/defaultCelVer, x/defaultCelHor) = color;
                }
                y++;
            }
        }
    }*/
}


void palleColorate(cv::Mat& img) {
    cv::Mat data;
    img.convertTo(data, CV_32F);
    data = data.reshape(1, img.rows * img.cols); // Each row is a pixel with 3 values (BGR)

    // Apply K-means clustering
    int K = 3; // Number of color clusters (you can try 2-5 depending on complexity)
    cv::Mat labels, centers;
    cv::kmeans(data, K, labels,
               cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 10, 1.0),
               3, cv::KMEANS_PP_CENTERS, centers);

    // Find which cluster is most distinct (assume the smallest is the object)
    std::vector<int> cluster_counts(K, 0);
    for (int i = 0; i < labels.rows; i++) {
        cluster_counts[labels.at<int>(i)]++;
    }

    // Find the cluster with the smallest count (likely the unique object)
    int min_cluster = std::min_element(cluster_counts.begin(), cluster_counts.end()) - cluster_counts.begin();

    // Create mask for that cluster
    cv::Mat mask(img.size(), CV_8UC1);
    for (int i = 0; i < labels.rows; i++) {
        mask.at<uchar>(i / img.cols, i % img.cols) = (labels.at<int>(i) == min_cluster) ? 255 : 0;
    }

    // Optional: Clean up mask
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1,-1), 2);

    // Find contours and draw bounding boxes
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        cv::Rect box = cv::boundingRect(contour);
        cv::rectangle(img, box, cv::Scalar(0, 0, 255), 2);
    }

    // Show result
    cv::imshow("Detected Object", img);
    cv::imshow("Mask", mask);
    cv::waitKey(0);
}


cv::Mat elaborateFrame(cv::Mat& image) {

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
            } else {
                ordered[0] = pts[1];
                ordered[1] = pts[0];
            }

            if (pts[2].x < pts[3].x) {
                ordered[3] = pts[2]; // bottom-left
                ordered[2] = pts[3]; // bottom-right
            } else {
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


void boobs() {

    defineMedia();
    cv::Mat frame;
    while(true) {
        loopVid.read(frame);
        if(frame.empty()) break;

        cv::Mat uu = elaborateFrame(frame);

        //cv::imshow("frame", uu);
        //int key = cv::waitKey(0);
        //if (key == 'q') break;
    }

}


