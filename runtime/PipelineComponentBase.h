/*
 * PipelineComponentBase.h
 *
 *  Created on: Feb 16, 2012
 *      Author: george
 */

#ifndef PIPELINECOMPONENTBASE_H_
#define PIPELINECOMPONENTBASE_H_

#include <vector>
#include <string>
#include <map>
#include <list>

#include "Argument.h"
#include "Task.h"
//#include "Worker.h"
//#include "Manager.h"

#ifdef WITH_RT
#include "Cache.h"
//class Cache;
#endif

class PipelineComponentBase;
class Manager;


// Define factory function type that creates objects of type PipelineComponentBase and its subclasses
typedef PipelineComponentBase* (componetFactory_t)();

class PipelineComponentBase: public Task {
private:
	// Contain pointers to all arguments associated to this pipeline component
	std::vector<ArgumentBase*> arguments;

	// Holds the string name of the component, which should be the same used to register with the ComponentFactory
	std::string component_name;

	char *resultData;
	int resultDataSize;

	// Resource manager used to execute this pipeline. This pointer is initialized at
	// the node level, when a Worker instantiates the pipeline component
	ExecutionEngine *resourceManager;

	// Simply set, and get resourceManager value
    void setResourceManager(ExecutionEngine *resourceManager);
    ExecutionEngine *getResourceManager() const;

	// Unique identifier of the class instance.
	int id;

	// Component type: may be a baseline pipeline component of a region template component
	int type;

	// Auxiliary class variable used to assign an unique id to each class object instance.
	static int instancesIdCounter;

	friend class Worker;
	friend class Manager;

	// merely here for user needs. not used by the system 
	std::string name;

	// holds the ids of the arguments before the component is
	// ready to be executed. used only by FGO at this time
	// OBS: only used on the workflow construction.
	std::list<int> input_arguments;
	std::list<int> output_arguments;

protected:
	// is this component at the worker or manager side?
	int location;
	// this is used to pass cache stored in the Worker to the Pipeline component
#ifdef WITH_RT
    Cache *cache;
    virtual void setCache(Cache *cache){
    	std::cout << "EXEC FATHER setCache" << std::endl;
    	return;
    };
    Cache *getCache(){return cache;};
#endif
    // calculate how much data will be reused if given component
    // is schedule for execution with Worker=workerId
    virtual long getAmountOfDataReuse(int workerId);

public:
	PipelineComponentBase();
	virtual ~PipelineComponentBase();

	// This is the function implemented by the user in the subclasses, and this
	// is the function executed by the runtime system which should contain the computation
	// associated to this pipeline component. Presumably, exposed a pipeline of tasks that
	// are further assigned to the Resource Manager.
	virtual int run(){return 1;};

	// This function is called before the subpipeline is created, and it is intended to
	// dispatch any tasks regarding IO operation need by the pipeline component. It returns
	// the id of the created IO task;
	virtual int createIOTask(){return -1;};

	// Add an argument to the end of the list
	void addArgument(ArgumentBase *arg);

	// Retrieve "index"th argument, if it exists, otherwise NULL is returned
	ArgumentBase *getArgument(int index);

	// Get current number of arguments associated to this component.
	int getArgumentsSize();

	// Return name of the component
    std::string getComponentName() const;

    // Yep, set the name of the component
    void setComponentName(std::string component_name);

    // Serialization size: number of bytes need to store components
    virtual int size();

    // Write component data to a buffer
    virtual int serialize(char *buff);

    // Initialize component data from a buffer generated by serialize function
    virtual int deserialize(char *buff);

    virtual PipelineComponentBase* clone();

    // Return Id to this component instance
	int getId() const;

	// Set component instance id
	void setId(int id);

	// Dispatch task for execution
	void executeTask(Task *task);

	int getType() const;
	void setType(int type);
	int getLocation() const;
	virtual void setLocation(int location);
	int getResultDataSize() const;
	char* getResultData() const;

	void setResultData(char* data, int dataSize);

	std::string getName() {return name;};
	void setName(std::string name) {this->name = name;};

	void addInput(int i) {input_arguments.push_back(i);};
	void addOutput(int i) {output_arguments.push_back(i);};

	std::list<int> getInputs() {return input_arguments;};
	std::list<int> getOutputs() {return output_arguments;};

	void replaceInput(int old_a, int new_a);
	void replaceOutput(int old_a, int new_a);

	// Factory class is used to build "reflection", and instantiate objects of
	// PipelineComponentBase subclasses that register with it
    class ComponentFactory{
    private:
		// This maps name of component types to the function that creates instances of those components
    	static std::map<std::string,componetFactory_t*> factoryMap;

    public:
    	// Used to register the component factory function with this factory class
    	static bool componentRegister(std::string name, componetFactory_t *compFactory);

    	// Retrieve pointer to function that creates components registered with name="name"
    	static componetFactory_t *getComponentFactory(std::string name);

    	// Retrieve instance of component registered as "name"
    	static PipelineComponentBase *getCompoentFromName(std::string name);
        ExecutionEngine *getResourceManager() const;

    };

    static const int PIPELINE_COMPONENT_BASE = 1;
    static const int RT_COMPONENT_BASE = 2;

    static const int MANAGER_SIDE	=	1;
    static const int WORKER_SIDE	=	2;

};

inline void PipelineComponentBase::replaceInput(int old_a, int new_a) {
	input_arguments.remove(old_a);
	input_arguments.push_back(new_a);
}

inline void PipelineComponentBase::replaceOutput(int old_a, int new_a) {
	output_arguments.remove(old_a);
	output_arguments.push_back(new_a);
}

#endif /* PIPELINECOMPONENTBASE_H_ */
