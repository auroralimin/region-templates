/*
 * HelloWorld.cpp
 *
 *  Created on: Feb 15, 2012
 *      Author: george
 */

#include <sstream>
#include <stdlib.h>
#include <iostream>

#include </usr/local/cuda-8.0/include/cuda_runtime_api.h>
#include "opencv2/opencv.hpp"
#include "opencv2/gpu/gpumat.hpp"

#include "Tiler.h"
#include "FileUtils.h"
#include "TimeUtils.h"
#include "OutUtils.h"
#include "RegionTemplate.h"
#include "RegionTemplateCollection.h"
#include "utilitiesSvs.h"

#include "SysEnv.h"
#include "Segmentation.h"

#define NUM_PIPELINE_INSTANCES	1
#define BALANCED false
#define REDUCED true

void parseInputArguments(int argc, char **argv, std::string &inputFolder,
                         int *pGpu, int *pCpu, int oTiles[3], bool *usesGpu){
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
        }else if(argv[i][0] == '-' && argv[i][1] == 'g' &&
                 atoi(argv[1+(i++)]) > 0){
            *usesGpu = true;
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

float backgroundRatio(cv::Mat bgr) {
    unsigned char blue = 220, green = 220, red = 220;
    ::cciutils::SimpleCSVLogger *logger = NULL;
    ::cciutils::cv::IntermediateResultHandler *iresHandler = NULL;
    cv::Mat background = ::nscale::HistologicalEntities::getBackground(bgr,
                         blue, green, red, logger, iresHandler);
    int bgArea = countNonZero(background);
    return (float)bgArea / (float)(bgr.size().area());
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
    std::ofstream out;
    out.open("ratio.csv", std::ios_base::app);
    out << "Tile name, Ratio" << std::endl;
#if REDUCED
    std::cout << "REDUCED!!!" << std::endl;
#else
    std::cout << "NOT REDUCED!!!" << std::endl;
#endif
	for(int i = 0; i < fileList.size(); i++){
		// TODO: remove comment
        // cout << endl << " FILE: " << fileList[i] << endl;
        openslide_t *osr; 
        int32_t lSizeLevel = 0;
        int64_t lSizeW = 0, lSizeH = 0;
#if REDUCED
        osr = openslide_open(fileList[i].c_str());
        gth818n::getLargestLevelSize(osr, lSizeLevel, lSizeW, lSizeH);
#endif
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
            oss << "(" << it->getUb().getX() << ","
                << it->getUb().getY() << ")";
            oss << "(" << it->getLb().getX() << ","
                << it->getLb().getY() << ")";
            std::cout << oss.str() << std::endl;

            float bRatio = 0.0; 
#if REDUCED
            int64_t topLeftX = it->getUb().getX();
            int64_t topLeftY = it->getUb().getY();
            int64_t thisTileSizeX = it->getLb().getX() - it->getUb().getX();
            int64_t thisTileSizeY = it->getLb().getY() - it->getUb().getY();
            uint32_t* dest = new uint32_t[thisTileSizeX*thisTileSizeY];

            openslide_read_region(osr, dest, topLeftX, topLeftY, lSizeLevel,
                    thisTileSizeX, thisTileSizeY);
            cv::Mat chunkData(thisTileSizeY, thisTileSizeX, CV_8UC3,
                    cv::Scalar(0, 0, 0));
            gth818n::osrRegionToCVMat(dest, chunkData);
            bRatio = backgroundRatio(chunkData);
            out << "\"" << oss.str() << "\"," << (1 - bRatio) << std::endl;
            delete[] dest;
#if BALANCED
            if (bRatio >= 0.9)
                continue;
#endif
#endif
            DenseDataRegion2D *ddr2d = new DenseDataRegion2D();
            ddr2d->setId(oss.str());
            ddr2d->setName(std::string("BGR"));
            ddr2d->setInputType(DataSourceType::FILE_SYSTEM);
            ddr2d->setOutputType(DataSourceType::FILE_SYSTEM);
            ddr2d->setBb(*it);
            ddr2d->setIsAppInput(true);
            ddr2d->setInputFileName(fileList[i]);
            ddr2d->setOutputExtension(DataRegion::SVS);
            ddr2d->setRatio(1 - bRatio);

            RegionTemplate *rt = new RegionTemplate();
            rt->setName("tile");
            rt->insertDataRegion(ddr2d);
            rtCollection->addRT(rt);
        }
#if REDUCED
        openslide_close(osr);
#endif
    }

    out.close();
    tu.markTimeUS("t2");
    tu.markDiffUS("t2", "t1", "set"); 
    tu.printDiff("set");
    tu.outCsv("profiling.csv");
    return rtCollection;
}

void unbalancedGraph(RegionTemplateCollection **rtCollection, SysEnv *sysEnv,
        int nqueue) {
    std::vector<float> rsum(nqueue, 0);
    for(int i = 0; i < (*rtCollection)->getNumRTs(); i++){
        DataRegion *dr = (*rtCollection)->getRT(i)->getDataRegion(0);
        float ratio = dr->getRatio();
        if (ratio >= 0.9)
            std::cout << dr->getId() << ": Background - " << ratio << "\n";
        else
            std::cout << dr->getId() << ": Not Background - " << ratio << "\n";

        Segmentation *seg = new Segmentation();
        seg->addRegionTemplateInstance((*rtCollection)->getRT(i),
                (*rtCollection)->getRT(i)->getName());
        sysEnv->executeComponent(seg, i%nqueue);
        rsum[i%nqueue] += 1 - ratio;
    }
    for (int i = 0; i < nqueue; i++)
        std::cout << "RSUM[" << i << "] = " << rsum[i] << std::endl;
}

void balancedGraph(RegionTemplateCollection **rtCollection, SysEnv *sysEnv,
        int nqueue) {
    std::vector<float> rsum(nqueue, 0);
    int index = 0;
    for(int i = 0; i < (*rtCollection)->getNumRTs(); i++){
        DataRegion *dr = (*rtCollection)->getRT(i)->getDataRegion(0);
        float ratio = dr->getRatio();
        if (ratio >= 0.9)
            std::cout << dr->getId() << ": Background - " << ratio << "\n";
        else
            std::cout << dr->getId() << ": Not Background - " << ratio << "\n";

        Segmentation *seg = new Segmentation();
        seg->addRegionTemplateInstance((*rtCollection)->getRT(i),
                (*rtCollection)->getRT(i)->getName());
        sysEnv->executeComponent(seg, index);
        rsum[index] += 1 - ratio;
        index = std::distance(rsum.begin(),
                std::min_element(rsum.begin(), rsum.end()));
    }

    for (int i = 0; i < nqueue; i++)
        std::cout << "RSUM[" << i << "] = " << rsum[i] << std::endl;
}


int main (int argc, char **argv){
    // Folder when input data images are stored
    std::string inputFolderPath;
    int pGpu = 100, pCpu = 0, oTiles[3] = {1, 2, 3}; 
    bool usesGpu = false;
    parseInputArguments(argc, argv, inputFolderPath,
            &pGpu, &pCpu, oTiles, &usesGpu);

    if (usesGpu){
        cudaSetDevice(0);
        cudaFree(0);
        cv::gpu::GpuMat test;
        test.create(1, 1, CV_8U);
        test.release();
    }

    TimeUtils tu("begin");

    // Handler to the distributed execution system environment
    SysEnv sysEnv;

    // Tell the system which libraries should be used and if manager will
    // have a single queue or not
    sysEnv.startupSystem(argc, argv, "libcomponentsrt.so", false);
    int nqueue = sysEnv.getNqueue();

    // Create region templates description without instantiating data
    RegionTemplateCollection *rtCollection;
    rtCollection = RTFromFiles(inputFolderPath, pGpu, pCpu, oTiles);

    tu.markTimeUS("init");
    tu.markDiffUS("init", "begin", "init");
    tu.printDiff("init");

    // Instantiate application dependency graph
    //unbalancedGraph(&rtCollection, &sysEnv, 1);
#if BALANCED
    std::cout << "BALANCED!!!" << std::endl;
    balancedGraph(&rtCollection, &sysEnv, nqueue);
#else
    std::cout << "UNBALANCED!!!" << std::endl;
    unbalancedGraph(&rtCollection, &sysEnv, nqueue);
#endif

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

