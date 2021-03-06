
#include <sstream>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "FileUtils.h"
#include "RegionTemplate.h"
#include "RegionTemplateCollection.h"

#include "SysEnv.h"
#include "NormalizationComp.h"
#include "Segmentation.h"
#include "FeatureExtraction.h"
#include "DiffMaskComp.h"


namespace patch{
	template < typename T > std::string to_string(const T& n)
	{
		std::ostringstream stm;
		stm << n;
		return stm.str();
	}
}
void parseInputArguments(int argc, char**argv, std::string &inputFolder){
	// Used for parameters parsing
	for(int i = 0; i < argc-1; i++){
		if(argv[i][0] == '-' && argv[i][1] == 'i'){
			inputFolder = argv[i+1];
		}
	}
}


RegionTemplateCollection* RTFromFiles(std::string inputFolderPath){
	// Search for input files in folder path
	FileUtils fileUtils("tif");
	std::vector<std::string> fileList;
	fileUtils.traverseDirectoryRecursive(inputFolderPath, fileList);
	RegionTemplateCollection* rtCollection = new RegionTemplateCollection();
	rtCollection->setName("inputimage");

	std::cout << "Input Folder: "<< inputFolderPath << " nFiles: "<<fileList.size() <<std::endl;

	// Create one region template instance for each input data file
	// (creates representations without instantiating them)
	for(int i = 0; i < fileList.size(); i++){
		DenseDataRegion2D *ddr2d = new DenseDataRegion2D();
		ddr2d->setName("RAW");
		ddr2d->setInputType(DataSourceType::FILE_SYSTEM);
		ddr2d->setOutputType(DataSourceType::FILE_SYSTEM);
		ddr2d->setId(fileList[i]);
		RegionTemplate *rt = new RegionTemplate();
		rt->setName("tile");
		rt->insertDataRegion(ddr2d);
		rtCollection->addRT(rt);
	}

	return rtCollection;
}


int main (int argc, char **argv){
	// Folder when input data images are stored
	std::string inputFolderPath;
	std::vector<RegionTemplate *> inputRegionTemplates;
	RegionTemplateCollection *rtCollection;

	parseInputArguments(argc, argv, inputFolderPath);

	// Handler to the distributed execution system environment
	SysEnv sysEnv;

	// Tell the system which libraries should be used
	sysEnv.startupSystem(argc, argv, "libcomponentnsffsdiff.so");

	// Create region templates description without instantiating data
	rtCollection = RTFromFiles(inputFolderPath);

	int numParameters = 2;

	// Build application dependency graph
	// Instantiate application dependency graph
	for(int i = 0; i < rtCollection->getNumRTs(); i++){
		int previousFeatureCompId = 0;
		for(int j = 0; j < numParameters; j++){
			DiffMaskComp *diff = NULL;
			NormalizationComp *norm = new NormalizationComp();
			norm->addArgument(new ArgumentString(patch::to_string(j)));
			norm->addRegionTemplateInstance(rtCollection->getRT(i), rtCollection->getRT(i)->getName());

			Segmentation *seg = new Segmentation();
			seg->addArgument(new ArgumentString(patch::to_string(j)));
			seg->addRegionTemplateInstance(rtCollection->getRT(i), rtCollection->getRT(i)->getName());
			seg->addDependency(norm->getId());

			FeatureExtraction *fe =  new FeatureExtraction();
			fe->addArgument(new ArgumentString(patch::to_string(j)));
			fe->addRegionTemplateInstance(rtCollection->getRT(i), rtCollection->getRT(i)->getName());
			fe->addDependency(seg->getId());

			if(j > 0){
				diff = new DiffMaskComp();
				// region template name
				diff->addRegionTemplateInstance(rtCollection->getRT(i), rtCollection->getRT(i)->getName());
				diff->addDependency(previousFeatureCompId);
				diff->addDependency(fe->getId());
			}

			previousFeatureCompId = fe->getId();

			sysEnv.executeComponent(norm);
			sysEnv.executeComponent(seg);
			sysEnv.executeComponent(fe);
			if(diff != NULL)
				sysEnv.executeComponent(diff);
		}
	}

	// End Create Dependency Graph
	sysEnv.startupExecution();

	// Finalize all processes running and end execution
	sysEnv.finalizeSystem();

	delete rtCollection;

	return 0;
}



