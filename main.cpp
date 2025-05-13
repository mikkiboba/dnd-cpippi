#include "Kitchen.h"

int main() {

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