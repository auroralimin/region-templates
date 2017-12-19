/*
 * FeatureExtraction.cpp
 *
 *  Created on: Feb 16, 2013
 *      Author: george
 */

#include "FeatureExtraction.h"
#include "TimeUtils.h"
//#include "TaskSum.h"

FeatureExtraction::FeatureExtraction() {
	this->setComponentName("FeatureExtraction");
	this->addInputOutputDataRegion("tile", "BGR", RTPipelineComponentBase::INPUT);
	this->addInputOutputDataRegion("tile", "mask", RTPipelineComponentBase::INPUT);
}

FeatureExtraction::~FeatureExtraction() {

}

int FeatureExtraction::run()
{
    TimeUtils tu("t1");

	RegionTemplate * inputRt = this->getRegionTemplateInstance("tile");

	if(inputRt != NULL){
		std::cout << "\tfound RT named tile"<< std::endl;
		DenseDataRegion2D *bgr = dynamic_cast<DenseDataRegion2D*>(inputRt->getDataRegion("BGR"));
		DenseDataRegion2D *mask = dynamic_cast<DenseDataRegion2D*>(inputRt->getDataRegion("mask"));
		if(bgr != NULL && mask != NULL){
			std::cout << "Feature computation: Data Region is not null"<< std::endl;

			// Create processing task
			TaskFeatures * feaTask = new TaskFeatures(bgr, mask);

			this->executeTask(feaTask);
		}
	}else{
		std::cout << "\tDid not find RT named tile"<< std::endl;
	}

    tu.markTimeUS("t2");
    std::ostringstream oss;
    oss << this->getComponentName() << " " << this->getId();
    tu.markDiffUS("t2", "t1", oss.str()); 
    tu.printDiffs();
    tu.outCsv("profiling.csv");

	return 0;
}

// Create the component factory
PipelineComponentBase* componentFactoryFE() {
	return new FeatureExtraction();
}

// register factory with the runtime system
bool registeredB = PipelineComponentBase::ComponentFactory::componentRegister("FeatureExtraction", &componentFactoryFE);


