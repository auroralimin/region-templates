/*
 * HelloWorld.cpp
 *
 *  Created on: Feb 15, 2012
 *      Author: george
 */

#include <sstream>
#include <stdlib.h>
#include <iostream>

#include "Tiler.h"
#include "FileUtils.h"
#include "RegionTemplate.h"
#include "RegionTemplateCollection.h"

#include "SysEnv.h"
#include "Segmentation.h"
#include "FeatureExtraction.h"

#define NUM_PIPELINE_INSTANCES	1


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
	FileUtils fileUtils("svs");
	std::vector<std::string> fileList;
	fileUtils.traverseDirectoryRecursive(inputFolderPath, fileList);
	RegionTemplateCollection* rtCollection = new RegionTemplateCollection();
	rtCollection->setName("inputimage");

	std::cout << "Input Folder: "<< inputFolderPath <<std::endl;

	cout << endl << " FILELIST SIZE: " << fileList.size() << endl;

	// Create one region template instance for each input data file
	// (creates representations without instantiating them)
    Tiler tiler;
	for(int i = 0; i < fileList.size(); i++){
		cout << endl << " FILE: " << fileList[i] << endl;

        int oTiles[3] = {2, 3, 4};
        std::vector<BoundingBox> bTiles;
        bTiles = tiler.divSplit(fileList[i].c_str(), 100, 0, oTiles);
        tiler.printTiles(bTiles);

        std::vector<BoundingBox>::iterator it = bTiles.begin();
        std::string fName = fileList[i].substr(0, fileList[i].find(".svs"));
        while (fName.find("/") != std::string::npos) {
            fName = fName.substr(fName.find("/") + 1, fName.length());
        }
        for (it = bTiles.begin(); it < bTiles.end(); it++) {
            std::ostringstream oss;
            oss << fName << "_";
            oss << "(" << it->getUb().getX() << "," << it->getUb().getY() << ")";
            oss << "(" << it->getLb().getX() << "," << it->getLb().getY() << ")";
            std::cout << oss.str() << std::endl;
            
            DenseDataRegion2D *ddr2d = new DenseDataRegion2D();
            ddr2d->setId(oss.str());
            ddr2d->setName(std::string("BGR"));
            ddr2d->setInputType(DataSourceType::FILE_SYSTEM);
            ddr2d->setOutputType(DataSourceType::FILE_SYSTEM);
            ddr2d->setBb(*it);
            ddr2d->setIsAppInput(true);
            ddr2d->setInputFileName(fileList[i]);
            ddr2d->setOutputExtension(DataRegion::SVS);

            RegionTemplate *rt = new RegionTemplate();
            rt->setName("tile");
            rt->insertDataRegion(ddr2d);
            rtCollection->addRT(rt);
        }
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
    sysEnv.startupSystem(argc, argv, "libcomponentsrt.so");

    // Create region templates description without instantiating data
    rtCollection = RTFromFiles(inputFolderPath);

    // Build application dependency graph

    // Instantiate application dependency graph
    for(int i = 0; i < rtCollection->getNumRTs(); i++){
        Segmentation *seg = new Segmentation();
        seg->addRegionTemplateInstance(rtCollection->getRT(i), rtCollection->getRT(i)->getName());
        FeatureExtraction *fe =  new FeatureExtraction();

        fe->addRegionTemplateInstance(rtCollection->getRT(i), rtCollection->getRT(i)->getName());
        fe->addDependency(seg->getId());

        sysEnv.executeComponent(seg);
        sysEnv.executeComponent(fe);
    }

    // End Create Dependency Graph
    sysEnv.startupExecution();

    // Finalize all processes running and end execution
    sysEnv.finalizeSystem();

    delete rtCollection;

    return 0;
}

