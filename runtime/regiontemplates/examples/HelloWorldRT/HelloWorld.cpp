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
#include "TimeUtils.h"
#include "OutUtils.h"
#include "RegionTemplate.h"
#include "RegionTemplateCollection.h"

#include "SysEnv.h"
#include "Segmentation.h"
#include "FeatureExtraction.h"

#define NUM_PIPELINE_INSTANCES	1

void parseInputArguments(int argc, char **argv, std::string &inputFolder,
                         int *pGpu, int *pCpu, int oTiles[3]){
	// Used for parameters parsing
	for(int i = 0; i < argc-1; i++){
		if(argv[i][0] == '-' && argv[i][1] == 'i'){
			inputFolder = argv[1+(i++)];
		}else if(argv[i][0] == '-' && argv[i][1] == 'p' && argv[i][2] == 'g'){
            *pGpu = atoi(argv[1+(i++)]);
		}else if(argv[i][0] == '-' && argv[i][1] == 'p' && argv[i][2] == 'c'){
            *pCpu = atoi(argv[1+(i++)]);
		}else if(argv[i][0] == '-' && argv[i][1] == 'o' && argv[i][2] == 'g'){
            oTiles[0] = atoi(argv[1+(i++)]);
		}else if(argv[i][0] == '-' && argv[i][1] == 'o' && argv[i][2] == 'a'){
            oTiles[1] = atoi(argv[1+(i++)]);
		}else if(argv[i][0] == '-' && argv[i][1] == 'o' && argv[i][2] == 'c'){
            oTiles[2] = atoi(argv[1+(i++)]);
        }
	}
}

void printInputArguments(std::string inputFolderPath,
                         int pGpu, int pCpu, int oTiles[3]){
    std::cout << COLOR(blue) << "[SPLIT] " << OFF;
    std::cout << "Input: " << inputFolderPath << std::endl;
    std::cout << COLOR(blue) << "[SPLIT] " << OFF;
    std::cout << "Gpu Percentage: " << pGpu << std::endl;
    std::cout << COLOR(blue) << "[SPLIT] " << OFF;
    std::cout << "Cpu Percentage: " << pCpu << std::endl;
    std::cout << COLOR(blue) << "[SPLIT] " << OFF;
    std::cout << "Gpu Order: " << oTiles[0] << std::endl;
    std::cout << COLOR(blue) << "[SPLIT] " << OFF;
    std::cout << "Ave Order: " << oTiles[1] << std::endl;
    std::cout << COLOR(blue) << "[SPLIT] " << OFF;
    std::cout << "Cpu Order: " << oTiles[2] << std::endl;
}

RegionTemplateCollection* RTFromFiles(std::string inputFolderPath,
                                      int pGpu, int pCpu, int oTiles[3]){
    TimeUtils tu("t1");

	// Search for input files in folder path
	FileUtils fileUtils("svs");
	std::vector<std::string> fileList;
	fileUtils.traverseDirectoryRecursive(inputFolderPath, fileList);
	RegionTemplateCollection* rtCollection = new RegionTemplateCollection();
	rtCollection->setName("inputimage");

    /* TODO: remove comment
	std::cout << "Input Folder: "<< inputFolderPath <<std::endl;
	cout << endl << " FILELIST SIZE: " << fileList.size() << endl;
    */
    printInputArguments(inputFolderPath, pGpu, pCpu, oTiles);

	// Create one region template instance for each input data file
	// (creates representations without instantiating them)
    Tiler tiler;
	for(int i = 0; i < fileList.size(); i++){
		// TODO: remove comment
        // cout << endl << " FILE: " << fileList[i] << endl;

        std::vector<BoundingBox> bTiles;
        bTiles = tiler.divSplit(fileList[i].c_str(), pGpu, pCpu, oTiles);
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

    tu.markTimeUS("t2");
    tu.markDiffUS("t2", "t1", "RFFromFiles"); 
    tu.printDiffs();
    tu.outCsv("profiling.csv");
    return rtCollection;
}

int main (int argc, char **argv){
    TimeUtils tu("begin");

    // Folder when input data images are stored
    std::string inputFolderPath;
    int pGpu = 100, pCpu = 0, oTiles[3] = {1, 2, 3}; 
    std::vector<RegionTemplate *> inputRegionTemplates;
    RegionTemplateCollection *rtCollection;

    parseInputArguments(argc, argv, inputFolderPath, &pGpu, &pCpu, oTiles);
    // Handler to the distributed execution system environment
    SysEnv sysEnv;

    // Tell the system which libraries should be used
    sysEnv.startupSystem(argc, argv, "libcomponentsrt.so");

    // Create region templates description without instantiating data
    rtCollection = RTFromFiles(inputFolderPath, pGpu, pCpu, oTiles);

    tu.markTimeUS("init");
    tu.markDiffUS("init", "begin", "Initialization");
    tu.printDiff("Initialization");

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

    tu.markTimeUS("end");
    tu.markDiffUS("end", "begin", "Total"); 
    tu.printDiff("Total");
    tu.outCsv("profiling.csv");
    return 0;
}

