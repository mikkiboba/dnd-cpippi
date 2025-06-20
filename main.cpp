#include "Kitchen.h"
#include "tette.cpp"

#include <opencv2/opencv.hpp>


void showImg(cv::Mat img) {
    cv::imshow("", img);
    cv::waitKey(0);
}


void bbbb() {

    cv::VideoCapture grid("../imgs/tetteFull.mp4");

    cv::Mat img;
    grid.read(img);

    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    cv::Size patternSize(4,6); // e.g. 7x7 inner corners

    // Vector to store the corner points
    std::vector<cv::Point2f> corners;

    // Try to find the chessboard corners
    bool found = cv::findChessboardCorners(
        gray,
        patternSize,
        corners,
        cv::CALIB_CB_ADAPTIVE_THRESH + 
        cv::CALIB_CB_NORMALIZE_IMAGE +
        cv::CALIB_CB_FAST_CHECK
    );

    if (found) {
        // Refine corner locations for better accuracy
        cv::cornerSubPix(
            gray,
            corners,
            cv::Size(11, 11),
            cv::Size(-1, -1),
            cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1)
        );

        // Draw corners on the original image
        cv::drawChessboardCorners(img, patternSize, corners, found);
        cv::imshow("Detected Chessboard", img);
        cv::waitKey(0);
    } else {
        std::cout << "Chessboard corners not found." << std::endl;
    }
}


void aaaa() {

    cv::VideoCapture grid("../imgs/tetteGrid.mp4");
    cv::Mat img;
    grid.read(img);

    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(5,5), 0);

    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150, 3);

    std::vector<cv::Vec2f> lines;
    cv::HoughLines(edges, lines, 1, CV_PI/180, 150);
    
    cv::Mat lineImg = img.clone();
    for (size_t i = 0; i < lines.size(); i++) {
        float rho = lines[i][0];
        float theta = lines[i][1];
        cv::Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        pt1.x = cvRound(x0 + 1000 * (-b));
        pt1.y = cvRound(y0 + 1000 * (a));
        pt2.x = cvRound(x0 - 1000 * (-b));
        pt2.y = cvRound(y0 - 1000 * (a));
        line(lineImg, pt1, pt2, cv::Scalar(0, 0, 255), 2);
    }

    cv::imshow("", lineImg);
    cv::waitKey(0);

}


int main() {

    boobs();
    return 0;

    // * TEST: background image definition
	#if defined(_WIN32) || defined(_WIN64)
        cv::VideoCapture cap("../../../dnd-cpippi/imgs/bgtette.mp4"); // open the video file
	#endif
    
	#if defined(__APPLE__) || defined(__MACH__)
		cv::VideoCapture cap("../imgs/bgtette.mp4");
	#endif

    if (!cap.isOpened()) {  // check if we succeeded
        std::cerr << "Can not open Video file" << std::endl;
        return -1;
    }
    else std::cout << "Video opened" << std::endl;

    Kitchen videoKitchen = Kitchen(cap);
    videoKitchen.letHimCook();

    return 0;
}