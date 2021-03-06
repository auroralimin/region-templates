#include <regiontemplates/comparativeanalysis/pixelcompare/PixelCompare.h>
#include "DiffMaskComp.h"

DiffMaskComp::DiffMaskComp() {
//	diffPercentage = 0.0;
    //task = new PixelCompare();
	this->setComponentName("DiffMaskComp");

}

DiffMaskComp::~DiffMaskComp() {

}

int DiffMaskComp::run() {

	RegionTemplate * inputRt = this->getRegionTemplateInstance("tile");
	
	// int parameterSegId = ((ArgumentInt*)this->getArgument(0))->getArgValue();
	std::string computed_mask_name;
	std::string reference_mask_name;
	float* diffPixels;

	int set_cout = 0;
	for(int i=0; i<this->getArgumentsSize(); i++){
		if (this->getArgument(i)->getName().compare("computed_mask") == 0) {
			computed_mask_name = (std::string)((ArgumentString*)this->getArgument(i))->getArgValue();
			set_cout++;
		}

		if (this->getArgument(i)->getName().compare("reference_mask") == 0) {
			reference_mask_name = (std::string)((ArgumentString*)this->getArgument(i))->getArgValue();
			set_cout++;
		}

		if (this->getArgument(i)->getName().compare("diffPixels") == 0) {
			diffPixels = (float*)((ArgumentFloatArray*)this->getArgument(i))->getArgValue();
			set_cout++;
		}

	}

	if (set_cout < this->getArgumentsSize())
		std::cout << __FILE__ << ":" << __LINE__ <<" Missing common arguments on Segmentation" << std::endl;

	// this->addInputOutputDataRegion("tile", inputDr, RTPipelineComponentBase::INPUT);

	// float *diffPixels = (float *) malloc(2 * sizeof(float));
	// diffPixels[0] = 0;
	// diffPixels[1] = 0;
	// //(data,data size)
	this->setResultData((char*)diffPixels, 2* sizeof(float));

	if(inputRt != NULL){

		// Mask computed in segmentation using specific application parameter set
		DenseDataRegion2D *computed_mask = dynamic_cast<DenseDataRegion2D*>(inputRt->getDataRegion(computed_mask_name, "", 0, dr_id));
		// Mask used as a reference
		DenseDataRegion2D *reference_mask = dynamic_cast<DenseDataRegion2D*>(inputRt->getDataRegion(reference_mask_name, "", 0, dr_id));

		if(computed_mask != NULL && reference_mask != NULL){
			// gambiarra
			diffPixels[0] =  this->getId();
            cout << "------------------------------------------------- Calling Diff Mask:" << endl;
            TaskDiffMask *tDiffMask = new PixelCompare(computed_mask, reference_mask, diffPixels);
            this->executeTask(tDiffMask);
			// Create processing task
//            if(task != NULL){
//                task->setDr1(computed_mask);
//                task->setDr2(reference_mask);
//                task->setDiff(diffPixels);
//
//                this->executeTask(task);
//            }
//            else {
//                std::cout << "DiffMaskComp: you didn't specify a task: " << std::endl;
//                inputRt->print();
//            }

		}else{
			std::cout << "DiffMaskComp: did not find data regions: " << std::endl;
			inputRt->print();
		}
	}else{
		std::cout << "\tTASK diff mask: Did not find RT named tile"<< std::endl;
	}

	return 0;
}

//void DiffMaskComp::setTask(TaskDiffMask* task){
//    this->task = task;
//}
// Create the component factory
PipelineComponentBase* componentFactoryDF() {
	return new DiffMaskComp();
}

// register factory with the runtime system
bool registeredDF = PipelineComponentBase::ComponentFactory::componentRegister("DiffMaskComp", &componentFactoryDF);


