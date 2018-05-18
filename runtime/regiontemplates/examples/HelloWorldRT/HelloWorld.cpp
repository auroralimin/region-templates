/*
 * HelloWorld.cpp
 *
 *  Created on: Feb 15, 2012
 *      Author: george
 */

#include <sstream>
#include <stdlib.h>
#include <iostream>

#include "opencv2/opencv.hpp"

#include "QuadTree.h"
#include "FileUtils.h"
#include "TimeUtils.h"
#include "OutUtils.h"
#include "RegionTemplate.h"
#include "RegionTemplateCollection.h"
#include "utilitiesSvs.h"

#include "SysEnv.h"
#include "Segmentation.h"

#define NUM_PIPELINE_INSTANCES	1

void parseInputArguments(int argc, char **argv, std::string &inputFolder,
                         int *ogIdeal, int *ogMax, bool *reduced){
	// Used for parameters parsing
	for(int i = 0; i < argc-1; i++){
		if(std::string(argv[i]) == "-i"){
			inputFolder = argv[1+(i++)];
		}else if(std::string(argv[i]) == "-oi"){
            *ogIdeal = atoi(argv[1+(i++)]);
		}else if(std::string(argv[i]) == "-om"){
            *ogMax = atoi(argv[1+(i++)]);
		}else if(std::string(argv[i]) == "--reduced"){
            *reduced = true;
        }
	}
}

void printInputArguments(std::string inputFolderPath, int ogIdeal, int ogMax, bool reduced){
    std::cout << COLOR(blue) << "[SPLIT] " << OFF;
    std::cout << "Input: " << inputFolderPath << std::endl;
    std::cout << COLOR(blue) << "[SPLIT] " << OFF;
    std::cout << "Ideal OG: " << ogIdeal << std::endl;
    std::cout << COLOR(blue) << "[SPLIT] " << OFF;
    std::cout << "Max OG: " << ogMax << std::endl;
    if (reduced) {
        std::cout << COLOR(blue) << "[SPLIT] " << OFF;
        std::cout << "Reduced approach!" << std::endl;
    } else {
        std::cout << COLOR(blue) << "[SPLIT] " << OFF;
        std::cout << "Non Reduced approach!" << std::endl;
    }
}

RegionTemplateCollection* RTFromFiles(std::string inputFolderPath, int ogIdeal, int ogMax, bool reduced){
    TimeUtils tu("t1");

	// Search for input files in folder path
	FileUtils fileUtils("svs");
	std::vector<std::string> fileList;
	fileUtils.traverseDirectoryRecursive(inputFolderPath, fileList);
	RegionTemplateCollection* rtCollection = new RegionTemplateCollection();
	rtCollection->setName("inputimage");

    printInputArguments(inputFolderPath, ogIdeal, ogMax, reduced);
	for(int i = 0; i < fileList.size(); i++){
        QuadTree node = QuadTree(fileList[i]);
        for (int j = 0; j < ogMax; j++) {
            node.subdivide();
        }
        if (reduced) {
            tu.markTimeUS("reduced_begin");
            node.calculateRatios();
            node.backgroundAwareMerge(ogIdeal, 0.5);
            tu.markTimeUS("reduced_end");
            tu.markDiffUS("reduced_end", "reduced_begin", "reduced");
            tu.printDiff("reduced");
        }

        std::vector<QuadTree> tiles = node.treeToVec();
        std::cout << "root: " << node.getBb() << std::endl;

        std::cout << "qt size: " << tiles.size() << std::endl;
        std::vector<QuadTree>::iterator it;

        std::string fName = fileList[i].substr(0, fileList[i].find(".svs"));
        while (fName.find("/") != std::string::npos) {
            fName = fName.substr(fName.find("/") + 1, fName.length());
        }
        for (it = tiles.begin(); it < tiles.end(); it++) {
            std::ostringstream oss;
            oss << it->getBb();
            std::cout << "BB: " << it->getBb() << ", OG: << " << it->getOg()
                      << std::endl; 

            DenseDataRegion2D *ddr2d = new DenseDataRegion2D();
            ddr2d->setId(oss.str());
            ddr2d->setName(std::string("BGR"));
            ddr2d->setInputType(DataSourceType::FILE_SYSTEM);
            ddr2d->setOutputType(DataSourceType::FILE_SYSTEM);
            ddr2d->setBb(it->getBb());
            ddr2d->setIsAppInput(true);
            ddr2d->setInputFileName(fileList[i]);
            ddr2d->setOutputExtension(DataRegion::SVS);
            ddr2d->setRatio(1 - it->getRatio());

            RegionTemplate *rt = new RegionTemplate();
            rt->setName("tile");
            rt->insertDataRegion(ddr2d);
            rtCollection->addRT(rt);
        }
    }

    //tu.markTimeUS("t2");
    //tu.markDiffUS("t2", "t1", "set"); 
    //tu.printDiff("set");
    tu.outCsv("profiling.csv");
    return rtCollection;
}

void unbalancedGraph(RegionTemplateCollection **rtCollection, SysEnv *sysEnv,
        int nqueue) {
    std::vector<float> rsum(nqueue, 0);
    int nRT = (*rtCollection)->getNumRTs();
    for(int i = 0; i < nRT; i++){
        DataRegion *dr = (*rtCollection)->getRT(i)->getDataRegion(0);
        float ratio = dr->getRatio();
        Segmentation *seg = new Segmentation();
        seg->addRegionTemplateInstance((*rtCollection)->getRT(i),
                (*rtCollection)->getRT(i)->getName());
        sysEnv->executeComponent(seg, (i*nqueue)/nRT);
        rsum[i%nqueue] += 1 - ratio;
    }

    for (int i = 0; i < nqueue; i++)
        std::cout << "RSUM[" << i << "] = " << rsum[i] << std::endl;
}

void balancedGraph(RegionTemplateCollection **rtCollection, SysEnv *sysEnv,
        int nqueue) {
    std::vector<float> rsum(nqueue, 0);
    /*
    int index = 0;
    for(int i = 0; i < (*rtCollection)->getNumRTs(); i++){
        DataRegion *dr = (*rtCollection)->getRT(i)->getDataRegion(0);
        float ratio = dr->getRatio();
        Segmentation *seg = new Segmentation();
        seg->addRegionTemplateInstance((*rtCollection)->getRT(i),
                (*rtCollection)->getRT(i)->getName());
        sysEnv->executeComponent(seg, index);
        rsum[index] += 1 - ratio;
        index = std::distance(rsum.begin(),
                std::min_element(rsum.begin(), rsum.end()));
    } */
    int nRT = (*rtCollection)->getNumRTs();
    for(int i = 0; i < nRT; i++){
        DataRegion *dr = (*rtCollection)->getRT(i)->getDataRegion(0);
        float ratio = dr->getRatio();
        Segmentation *seg = new Segmentation();
        seg->addRegionTemplateInstance((*rtCollection)->getRT(i),
                (*rtCollection)->getRT(i)->getName());
        sysEnv->executeComponent(seg, i%nqueue);
        rsum[i%nqueue] += 1 - ratio;
    }

    for (int i = 0; i < nqueue; i++)
        std::cout << "RSUM[" << i << "] = " << rsum[i] << std::endl;
}


int main (int argc, char **argv){
    // Folder when input data images are stored
    std::string inputFolderPath;
    int ogIdeal = 0, ogMax = 0; 
    bool reduced = false;
    parseInputArguments(argc, argv, inputFolderPath, &ogIdeal, &ogMax, &reduced);

    TimeUtils tu("begin");

    // Handler to the distributed execution system environment
    SysEnv sysEnv;

    // Tell the system which libraries should be used and if manager will
    // have a single queue or not
    bool singleQueue = false;
    if (reduced)
        sysEnv.startupSystem(argc, argv, "libcomponentsrt.so", singleQueue, true);
    else
        sysEnv.startupSystem(argc, argv, "libcomponentsrt.so", singleQueue, false);

    int nqueue = sysEnv.getNqueue();
    if (singleQueue) nqueue = 1;
    std::cout << "Number of Queues = " << nqueue << std::endl;

    // Create region templates description without instantiating data
    RegionTemplateCollection *rtCollection;
    rtCollection = RTFromFiles(inputFolderPath, ogIdeal, ogMax, reduced);

    tu.markTimeUS("init");
    tu.markDiffUS("init", "begin", "init");
    tu.printDiff("init");

    if (reduced)
        balancedGraph(&rtCollection, &sysEnv, nqueue);
    else
        unbalancedGraph(&rtCollection, &sysEnv, nqueue);

    // End Create Dependency Graph
    sysEnv.startupExecution();

    // Finalize all processes running and end execution
    sysEnv.finalizeSystem();

    delete rtCollection;

    tu.markTimeUS("end");
    tu.markDiffUS("end", "init", "processing"); 
    tu.printDiff("processing");
    tu.markDiffUS("end", "begin", "total"); 
    tu.printDiff("total");
    tu.outCsv("profiling.csv");
    return 0;
}

