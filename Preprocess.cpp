#include "Preprocess.h"

#include <opencv2/opencv.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <iostream>

#include <ifaddrs.h>
#include <cstring>

#include <chrono>


Preprocess::~Preprocess() {}


void Preprocess::defineMedia() {
#if defined(_WIN32) || defined(_WIN64)
    loopVid = cv::VideoCapture("../../../dnd-cpippi/imgs/tetteFull.mp4");
#endif
#if defined(__APPLE__) || defined(__MACH__)
    loopVid = cv::VideoCapture("../imgs/tetteFull.mp4");
#endif
}


Preprocess::Color Preprocess::detectColor(const cv::Vec3b& hsvColor) {
    const int h = hsvColor[0];
    const int s = hsvColor[1];
    const int v = hsvColor[2];

    constexpr int SAT_THRESH = 50;
    constexpr int VAL_THRESH = 50;

    if (s < SAT_THRESH || v < VAL_THRESH) {
        return Color::NONE;  // Too gray or dark to classify
    }

    if ((h < 10) || (h > 160 && h <= 180))  return Color::RED;
    else if (h >= 35 && h < 85)             return Color::GREEN;
    else if (h >= 90 && h < 130)            return Color::BLUE;

    return Color::OTHER;
}



void Preprocess::initializeDefaults(const cv::Mat& img) {
    
    // * vertical/horizontal number of pixels of the image
    // ? since it's warped, they should be similar
    defaultPixelVer = img.rows;
    defaultPixelHor = img.cols;

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
    currentMatrix = cv::Mat(defaultRows, defaultCols, CV_32S);
    currentMatrix.setTo(static_cast<int>(Color::NONE));
}


void Preprocess::findEntities(cv::Mat& rgbFrame, cv::Mat& mask) {
    
    // * convert color image to hsv to detect colors better
    cv::Mat hsvFrame;
    cv::cvtColor(rgbFrame, hsvFrame, cv::COLOR_BGR2HSV);
    
    // Split into individual channels
    std::vector<cv::Mat> hsvChannels;
    cv::split(hsvFrame, hsvChannels);
    
    // Create CLAHE objects for saturation and value channels
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(2.0);  // Adjust based on lighting conditions
    clahe->setTilesGridSize(cv::Size(8, 8));  // 8x8 grid
    
    // Apply CLAHE to saturation and value channels
    clahe->apply(hsvChannels[1], hsvChannels[1]);  // Saturation
    clahe->apply(hsvChannels[2], hsvChannels[2]);  // Value
    
    // Merge back the channels
    cv::merge(hsvChannels, hsvFrame);

    // * set the default values
    // ! this is done only once
    if (defaultPixelVer == 0 && defaultPixelHor == 0)
        initializeDefaults(rgbFrame);

    // * matrix of the positions in the frame
    cv::Mat frameMatrix(defaultRows, defaultCols, CV_32S);
    frameMatrix.setTo(static_cast<int>(Color::NONE));

    int halfInVer = defaultInCelVer / 2;
    int halfInHor = defaultInCelHor / 2;
    int halfCelHor = defaultCelHor / 2;
    int halfCelVer = defaultCelVer / 2;

    // Parallel loop
    
    cv::parallel_for_(cv::Range(0, defaultRows * defaultCols), [&](const cv::Range& range) {
        for (int index = range.start; index < range.end; ++index) {
            const int currRow = index / defaultCols;
            const int currCol = index % defaultCols;

            const int i = currCol * defaultCelHor + halfCelHor;
            const int j = currRow * defaultCelVer + halfCelVer;

            if (frameMatrix.at<int>(currRow, currCol) != static_cast<int>(Color::NONE))
                continue;

            bool foundColor = false;
            for (int dy = -halfInVer; dy <= halfInVer; ++dy) {
                const int y = j + dy;
                if (y < 0 || y >= mask.rows) continue;

                for (int dx = -halfInHor; dx <= halfInHor; ++dx) {
                    const int x = i + dx;
                    if (x < 0 || x >= mask.cols) continue;

                    int grayValue = mask.at<uchar>(y, x);
                    if (grayValue != 0) {
                        const cv::Vec3b& color = hsvFrame.at<cv::Vec3b>(y, x);
                        frameMatrix.at<int>(currRow, currCol) = static_cast<int>(detectColor(color));
                        foundColor = true;
                        break;
                    }
                }
            }
        }
    });

    

    
    // * checks if the previous valid matrix is different from the current frame matrix
    // * if different, then a change has occured and update the valid matrix
    
    cv::Mat matrixDiff;
    cv::compare(currentMatrix, frameMatrix, matrixDiff, cv::CMP_NE);
    if (cv::countNonZero(matrixDiff) > 0) {
        frameMatrix.copyTo(currentMatrix);
        std::cout << currentMatrix << std::endl;
        sendBool = true;
    }
    

    //cv::imshow("", copy);
    //cv::waitKey(0);


}


bool Preprocess::findLargestSquareContour(const cv::Mat& thresh, std::vector<cv::Point>& bestApprox) {

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(thresh.clone(), contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    double maxArea = 0;
    bool found = false;

    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area < 100) continue;

        std::vector<cv::Point> approx;
        cv::approxPolyDP(contour, approx, 0.02 * cv::arcLength(contour, true), true);

        if (approx.size() == 4 && cv::isContourConvex(approx) && area > maxArea) {
            maxArea = area;
            bestApprox = approx;
            found = true;
        }
    }
    return found;

}



std::vector<cv::Point2f> Preprocess::orderPoints(std::vector<cv::Point>& pts) {
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
}


cv::Mat Preprocess::warpToSquare(const cv::Mat& image, const std::vector<cv::Point2f>& srcPts, float side) {
    std::vector<cv::Point2f> dstPts = {
        cv::Point2f(0, 0),
        cv::Point2f(side - 1, 0),
        cv::Point2f(side - 1, side - 1),
        cv::Point2f(0, side - 1)
    };

    cv::Mat M = cv::getPerspectiveTransform(srcPts, dstPts);
    cv::Mat warped;
    cv::warpPerspective(image, warped, M, cv::Size(side, side));
    return warped;
}


void Preprocess::extractGridLines(const cv::Mat& binary, cv::Mat& horizontal, cv::Mat& vertical, int side) {
    int morphSize = side / 20;  // adjust based on grid size

    cv::Mat hor_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(morphSize, 1));
    cv::morphologyEx(binary, horizontal, cv::MORPH_OPEN, hor_kernel);

    cv::Mat ver_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, morphSize));
    cv::morphologyEx(binary, vertical, cv::MORPH_OPEN, ver_kernel);
}


void Preprocess::countGridLines(const std::vector<std::vector<cv::Point>>& horContours,
                               const std::vector<std::vector<cv::Point>>& verContours,
                               int side, int& rowCount, int& colCount) {
    rowCount = -1;
    colCount = -1;

    for (const auto& contour : horContours) {
        cv::Rect bbox = cv::boundingRect(contour);
        if (bbox.width > side * 0.5) { // horizontal line must be > 50% width
            rowCount++;
        }
    }

    for (const auto& contour : verContours) {
        cv::Rect bbox = cv::boundingRect(contour);
        if (bbox.height > side * 0.5) { // vertical line must be > 50% height
            colCount++;
        }
    }
}


void Preprocess::processGrid(const cv::Mat& binary, const cv::Mat& horizontal, const cv::Mat& vertical, float side, const cv::Mat& warped) {
    cv::Mat mask;
    cv::absdiff(binary, horizontal, mask);
    cv::absdiff(mask, vertical, mask);

    int size = 4;
    cv::Mat morphValue = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(size, size));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, morphValue);
    cv::medianBlur(mask, mask, 3);

    findEntities(const_cast<cv::Mat&>(warped), mask);
}


cv::Mat Preprocess::elaborateFrame(cv::Mat& image) {
    cv::Mat gray, blur, thresh;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blur, cv::Size(5, 5), 0);

    cv::adaptiveThreshold(blur, thresh, 255, cv::ADAPTIVE_THRESH_MEAN_C,
                          cv::THRESH_BINARY_INV, 15, 4);

    std::vector<cv::Point> bestApprox;
    if (!findLargestSquareContour(thresh, bestApprox)) {
        // No suitable square found, return original image
        return image;
    }

    std::vector<cv::Point2f> orderedPts = orderPoints(bestApprox);
    float side = 500.0f;
    cv::Mat warped = warpToSquare(image, orderedPts, side);

    cv::Mat grayWarped, binary;
    cv::cvtColor(warped, grayWarped, cv::COLOR_BGR2GRAY);
    cv::adaptiveThreshold(grayWarped, binary, 255, cv::ADAPTIVE_THRESH_MEAN_C,
                          cv::THRESH_BINARY_INV, 15, 4);

    cv::imshow("", warped);
    cv::waitKey(10);

    cv::Mat horizontal, vertical;
    extractGridLines(binary, horizontal, vertical, static_cast<int>(side));

    std::vector<std::vector<cv::Point>> horContours, verContours;
    cv::findContours(horizontal, horContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::findContours(vertical, verContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    int rowCount, colCount;
    countGridLines(horContours, verContours, static_cast<int>(side), rowCount, colCount);

    if (defaultRows == -1 && defaultCols == -1) {
        defaultRows = rowCount;
        defaultCols = colCount;
    }

    if (rowCount == defaultRows && colCount == defaultCols) {
        processGrid(binary, horizontal, vertical, side, warped);
    }

    return image;
}


void Preprocess::sendIntMat(int socket_fd, const cv::Mat& mat) {
    CV_Assert(mat.depth() == CV_32S || mat.depth() == CV_16U || mat.depth() == CV_8U);

    // Send header
    int header[4] = {mat.type(), mat.rows, mat.cols, mat.channels()};
    send(socket_fd, header, sizeof(header), 0);

    // Send data
    send(socket_fd, mat.data, mat.total() * mat.elemSize(), 0);

    std::cout << "sent (?)" << std::endl;
}

/*void Preprocess::runServer() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(6969);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);

    std::cout << "server started?" <<std::endl;

    while (sendBool) {
        std::cout << "Waiting for connection..." << std::endl;
        int client_socket = accept(server_fd, nullptr, nullptr);
        sendIntMat(client_socket, currentMatrix);
        sendBool = false;
        //close(client_socket);
    }
}*/

#include <netdb.h>

void Preprocess::runServer() {
    // Get and print server's IP addresses
    std::cout << "Server IP addresses:" << std::endl;
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        
        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) { // IPv4
            char host[NI_MAXHOST];
            getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                       host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
            std::cout << "  " << ifa->ifa_name << ": " << host << std::endl;
        }
    }
    freeifaddrs(ifaddr);

    // Create server socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(6969);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);

    std::cout << "\nServer started on port 6969" << std::endl;
    std::cout << "Clients should connect to one of the above IP addresses" << std::endl;

    while (sendBool) {
        std::cout << "\nWaiting for connection..." << std::endl;
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        std::cout << "Connection from: " << client_ip << std::endl;
        
        sendIntMat(client_socket, currentMatrix);
        sendBool = false;
        close(client_socket);
    }
    close(server_fd);
}


void openStream(cv::VideoCapture& capture, const std::string& streamURL, int delayMs = 1000) {

    capture.open(streamURL, cv::CAP_FFMPEG);

    while(!capture.isOpened()) {
        std::cerr << "Failed to connect to the webcam. Retrying in " << delayMs << "ms..." << std::endl;
        cv::waitKey(delayMs);
        capture.open(streamURL, cv::CAP_FFMPEG);
    }

}



void Preprocess::run() {

    defineMedia();

    std::string streamURL = "http://10.230.69.215:8080/video";
    // streamURL = "http://192.168.1.14:8080/video";
    cv::VideoCapture testVid;
    openStream(testVid, streamURL, 1000);
    testVid.set(cv::CAP_PROP_BUFFERSIZE, 10);
    testVid.set(cv::CAP_PROP_FPS, 15);


    cv::Mat frame;

    auto lastRun = std::chrono::steady_clock::now();

    while (true) {
        testVid.read(frame);
        if (frame.empty()) break;

        debug = false;
        
        cv::Mat uu = elaborateFrame(frame);
        auto now = std::chrono::steady_clock::now();

        // Run every 2 seconds
        if (sendBool && !debug) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastRun);
            if (elapsed.count() >= 2) {
                runServer();
                lastRun = now;  // Reset timer
            }
        }



        cv::imshow("frame", frame);
        cv::waitKey(10);
        //int key = cv::waitKey(0);
        //if (key == 'q') break;
    }

}
