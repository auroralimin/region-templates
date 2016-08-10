/*
 * Segmentation.h
 *
 *  GENERATED CODE
 *  DO NOT CHANGE IT MANUALLY!!!!!
 */

#ifndef Segmentation_H_
#define Segmentation_H_

#include "RTPipelineComponentBase.h"
#include "Util.h"
#include "FileUtils.h"
#include "Task.h"
#include "DenseDataRegion2D.h"
#include "ReusableTask.hpp"

#include "opencv2/opencv.hpp"
#include "opencv2/gpu/gpu.hpp"
#include "HistologicalEntities.h"

// PipelineComponent header
class Segmentation : public RTPipelineComponentBase {

private:
	// data region id
	// IMPORTANT: this need to be set during the creation of this object
	int workflow_id;

	list<ReusableTask*> mergable_t1;

public:
	Segmentation();
	virtual ~Segmentation();

	void set_workflow_id(int id) {workflow_id = id;};

	void print();

	int run();
};

// Task header
class TaskSegmentation: public ReusableTask {
private:

	// all other variables
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	double T1;
	double T2;
	unsigned char G1;
	int minSize;
	int maxSize;
	unsigned char G2;
	int minSizePl;
	int minSizeSeg;
	int maxSizeSeg;
	int fillHolesConnectivity;
	int reconConnectivity;
	int watershedConnectivity;


public:

	// data regions
	DenseDataRegion2D* normalized_rt_temp;
	DenseDataRegion2D* segmented_rt_temp;

	TaskSegmentation() {};
	TaskSegmentation(list<ArgumentBase*> args, RegionTemplate* inputRt);

	virtual ~TaskSegmentation();

	bool run(int procType=ExecEngineConstants::CPU, int tid=0);

	virtual bool reusable(ReusableTask* t);

	virtual int serialize(char *buff);
	virtual int deserialize(char *buff);
	virtual ReusableTask* clone();
	virtual int size();

	void print();
};

#endif /* Segmentation_H_ */
