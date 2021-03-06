/*
 * TaskArgument.h
 *
 *  Created on: Apr 2, 2012
 *      Author: gteodor
 */

#ifndef TASKARGUMENT_H_
#define TASKARGUMENT_H_

#include <iostream>
#include "opencv2/gpu/gpu.hpp"
#include "ExecEngineConstants.h"

class TaskArgument {
private:
	// Unique identifier of the class instance.
	int id;

	// Auxiliary class variable used to assign an unique id to each class object instance.
	static int instancesIdCounter;

	// Lock used to guarantee atomicity in the task argument creation/id generation
	static pthread_mutex_t taskCreationLock;

	// input, input_output, output
	int type;

public:
	TaskArgument();
	virtual ~TaskArgument();

	int getType() const {
		return type;
	}

	void setType(int type) {
		this->type = type;
	}

	int getId() const {
		return id;
	}

	void setId(int id) {
		this->id = id;
	}

	virtual bool upload(cv::gpu::Stream& stream) {
		std::cout << "uploadMaster" << std::endl;
		return false;
	}

	;

	virtual bool download(cv::gpu::Stream& stream) {
		std::cout << "dowloadMaster" << std::endl;
		return false;
	}

	;

	virtual void deleteGPUData() {
	}

	;
};

#endif /* TASKARGUMENT_H_ */
