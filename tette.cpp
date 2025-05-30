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
    bgVid = cv::VideoCapture("../imgs/tettecartaBG.mp4");
    bgVid.read(frameBg);
    gridVid = cv::VideoCapture("../imgs/tettecartaGRID.mp4");
    gridVid.read(frameGrid);
    loopVid = cv::VideoCapture("../imgs/tuttotettecarta.mp4");
}


cv::Mat elaborateFrame(cv::Mat& image) {

    // Convert to grayscale
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // Gaussian blur
    cv::Mat blur;
    cv::GaussianBlur(gray, blur, cv::Size(5, 5), 0);

    // Adaptive threshold
    cv::Mat thresh;
    cv::adaptiveThreshold(blur, thresh, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 3, 2);

    // Find contours
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(thresh, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    for(std::vector<cv::Point> c : contours) {
        std::cout << c << std::endl;
    }

    double max_area = 0;
    std::vector<cv::Point> best_cnt;

    for (size_t i = 0; i < contours.size(); ++i) {
        double area = cv::contourArea(contours[i]);
        if (area > 1000) {
            if (area > max_area) {
                max_area = area;
                best_cnt = contours[i];
                cv::drawContours(image, contours, static_cast<int>(i), cv::Scalar(0, 255, 0), 3);
            }
        }
    }

    // Create mask
    cv::Mat mask = cv::Mat::zeros(gray.size(), CV_8UC1);
    cv::drawContours(mask, std::vector<std::vector<cv::Point>>{best_cnt}, 0, cv::Scalar(255), -1);
    cv::drawContours(mask, std::vector<std::vector<cv::Point>>{best_cnt}, 0, cv::Scalar(0), 2);

    // Extract ROI
    cv::Mat out = cv::Mat::zeros(gray.size(), CV_8UC1);
    gray.copyTo(out, mask);

    // Blur again
    cv::GaussianBlur(out, blur, cv::Size(5, 5), 0);

    // Adaptive threshold again
    cv::adaptiveThreshold(blur, thresh, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 11, 2);

    // Find contours again
    cv::findContours(thresh, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    for (size_t i = 0; i < contours.size(); ++i) {
        double area = cv::contourArea(contours[i]);
        if (area > 500) {
            cv::drawContours(image, contours, static_cast<int>(i), cv::Scalar(0, 255, 0), 3);
        }
    }
    return image;
     
}

void boobs() {

    defineMedia();
    cv::Mat frame;
    while(true) {
        loopVid.read(frame);
        if(frame.empty()) break;

        cv::Mat uu = thiccThighs(frame, frameGrid);

        //cv::imshow("frame", uu);
        int key = cv::waitKey(0);
        if (key == 'q') break;
    }

}


