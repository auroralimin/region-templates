/*
 * SysEnv.h
 *
 *  Created on: Feb 15, 2012
 *      Author: george
 */

#ifndef SYSENV_H_
#define SYSENV_H_

#include <mpi.h>
#include <string.h>
#include <string>
#include <stdint.h>
#include "Types.hpp"
#include "Manager.h"
#include "Argument.h"


class SysEnv {
private:
	Manager* manager;

	Manager *getManager() const;
    void setManager(Manager *manager);

    // System parameters
    int cpus, gpus, policy, windowSize;
    int nqueue;
    bool dataLocalityAware;
    bool prefetching;
    bool cacheOnRead;
    bool componentDataAwareSchedule;

public:
	SysEnv();
	virtual ~SysEnv();

	void parseInputArguments(int argc, char**argv);

	int startupSystem(int argc, char **argv, std::string componentsLibName, bool singleQueue = true, bool canSteal = false);
	int executeComponent(PipelineComponentBase *compInstance, int n = 0);

	int startupExecution();
    	int finalizeSystem();

	int getWorkerSize(){
		return manager->getWorkerSize();
	}
	char *getComponentResultData(int id){
		return manager->getComponentResultData(id);
	}
	bool eraseResultData(int id){
		return manager->eraseResultData(id);
	}
    int getNqueue(){
        return nqueue;
    }
};

#endif /* SYSENV_H_ */
