#include "Kitchen.h"


Kitchen::~Kitchen() {}


void Kitchen::defineImgs() {

	#if defined(_WIN32) || defined(_WIN64)
        bgColorImage = cv::imread("../../../dnd-cpippi/imgs/background.jpg");
		//frame2 = cv::imread("../../../dnd-cpippi/imgs/backgroundGridPiedini.png");
		frameImage = cv::imread("../../../dnd-cpippi/imgs/backgroundGridPiedini2.png");
		bgGridImage = cv::imread("../../../dnd-cpippi/imgs/backgroundGrid.png");
	#endif

	#if defined(__APPLE__) || defined(__MACH__)
        bgColorImage = cv::imread("../imgs/background.jpg");
		//frame2 = cv::imread("../imgs/backgroundGridPiedini.png");
		frameImage = cv::imread("../imgs/backgroundGridPiedini2.png");
		bgGridImage = cv::imread("../imgs/backgroundGrid.png");
	#endif

	if (bgColorImage.empty() || frameImage.empty() || bgGridImage.empty()) {
		std::cerr << "Could not load images!" << std::endl;
	}

}


// * for debug
void Kitchen::showImg(cv::Mat& img) {
	cv::imshow("Image", img);
	cv::waitKey(0);
}


void Kitchen::letHimCook() {
	// * 1. frame del piano di gioco
	// -> proietta griglia
	// * 2. frame del piano di gioco + griglia
	grid.initPreprocess(bgGridImage, bgColorImage);
	// ...
	// ...
	// -> proietta resto (opzionale)
	// * 3. loop di frame
	cooking();
	// -> analizziamo i frame per frame
	// cooking()
	grid.printEntities();
}


void Kitchen::cooking() {

	grid.preprocessEntities(frameImage, bgGridImage);
	grid.findElementsInLine(frameImage);
}