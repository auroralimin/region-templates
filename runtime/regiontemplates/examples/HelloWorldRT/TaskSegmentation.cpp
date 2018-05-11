#include "TaskSegmentation.h"
#include "TimeUtils.h"

#include "ObjFeatures.h"
#include "opencv2/gpu/gpu.hpp"

#include </usr/local/cuda-8.0/include/cuda_runtime_api.h>

TaskSegmentation::TaskSegmentation(DenseDataRegion2D* bgr, DenseDataRegion2D* mask) {
	this->bgr = bgr;
	this->mask = mask;
}

TaskSegmentation::~TaskSegmentation() {
}

bool TaskSegmentation::run(int procType, int tid) {
    TimeUtils tu("tseg1");
    TimeUtils tu2("c_init");
	cv::Mat inputImage = this->bgr->getData();
	cv::Mat outMask;
    int compcount = 0, *bbox = NULL;

	this->bgr->setInputType(DataSourceType::DATA_SPACES);

    if (procType == ExecEngineConstants::GPU) {
	    ::nscale::gpu::HistologicalEntities::segmentNuclei(inputImage, outMask, compcount, bbox);
        //if(bbox!=NULL)
        //    free(bbox);
    } else {
        ::nscale::HistologicalEntities::segmentNuclei(inputImage, outMask);
    }

	this->mask->setData(outMask);
    outMask.release();
    inputImage.release();

    tu.markTimeUS("tseg2");
    tu.markDiffUS("tseg2", "tseg1", "tseg"); 
    tu.printDiff("tseg");
    tu.outCsv("profiling.csv");
    tu2.markTimeUS("c_end");
    std::ostringstream ss1;
    ss1 << "" << bgr->getId();
    tu2.markDiffUS("c_init", "c_end", ss1.str());
    tu2.outCsv("seg_time.csv");
}

