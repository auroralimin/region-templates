#include "TaskFeatures.h"
#include "TimeUtils.h"

TaskFeatures::TaskFeatures(DenseDataRegion2D* bgr, DenseDataRegion2D* mask) {
	this->bgr = bgr;
	this->mask = mask;
}

TaskFeatures::~TaskFeatures() {
}

bool TaskFeatures::run(int procType, int tid) {
    TimeUtils tu("tfeat1");
	int *bbox = NULL;
	int compcount;
	uint64_t t1 = Util::ClockGetTimeProfile();

	cv::Mat inputImage = this->bgr->getData();
	cv::Mat maskImage = this->mask->getData();

	std::cout << "nChannels:  "<< maskImage.channels() << std::endl;
	if(inputImage.rows > 0 && maskImage.rows > 0)
		nscale::ObjFeatures::calcFeatures(inputImage, maskImage);
	else
		std::cout << "Not Computing features" << std::endl;

	uint64_t t2 = Util::ClockGetTimeProfile();

    tu.markTimeUS("tfeat2");
    tu.markDiffUS("tfeat2", "tfeat1", "tfeat"); 
    tu.printDiff("tfeat");
    tu.outCsv("profiling.csv");
}
