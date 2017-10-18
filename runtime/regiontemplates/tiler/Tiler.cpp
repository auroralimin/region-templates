#include "Tiler.h"

#include <cstdio>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "utilitiesSvs.h"

Tiler::Tiler() {}

Tiler::~Tiler() {}

void Tiler::defaultSplit(const char* imgName, std::string out,
                         int64_t tSize, float threshold) {
    const int ImageDimension = 2;
    
    this->out = out;
    this->tSize = tSize;

    osr = openslide_open(imgName);

    int mag = gth818n::getMagnification(osr);

    lSizeW = 0;
    lSizeH = 0;
    lSizeLevel = 0;

    gth818n::getLargestLevelSize(osr, lSizeLevel, lSizeW, lSizeH);

    std::cout << "Largest level: " << lSizeLevel << " has size: " << lSizeW
              << ", " << lSizeH << std::endl;

    nTileW = lSizeW/tSize + 1;
    nTileH = lSizeH/tSize + 1;

    breakTiles(out, "");
}

void Tiler::approxSplit(const char* imgName, std::string out, unsigned int pGpu,
                        unsigned int pCpu, int64_t tSizes[3]) {
    int pAve = 100 - pGpu - pCpu;
    if (pAve < 0) {
        std::cerr << "Error: Gpu percentage plus Cpu percentage\
                      must be less or equal to 100.\n";
        exit(-1);
    }

    const int ImageDimension = 2;

    osr = openslide_open(imgName);
    this->out = out;

    mag = gth818n::getMagnification(osr);

    lSizeW = 0;
    lSizeH = 0;
    lSizeLevel = 0;

    gth818n::getLargestLevelSize(osr, lSizeLevel, lSizeW, lSizeH);
    std::cout << "Largest level: " << lSizeLevel << " has size: " << lSizeW
              << ", " << lSizeH << std::endl;

    tSize = tSizes[0];
    nTileW = ((lSizeW*pGpu)/100)/tSize;
    nTileH = (lSizeH/tSize) + 1;

    std::cout << "Breaking large tiles..." << std::endl;
    breakTiles("1", "2");
    int64_t topX = nTileW*tSize;

    if (pAve > 0) {
        tSize = tSizes[1];
        nTileW = ((lSizeW*pAve)/100)/tSize;
        nTileH = (lSizeH/tSize) + 1;
        std::cout << "Breaking medium tiles..." << std::endl;
        breakTiles("2", "3", topX);
        topX = nTileW*tSize;
    }

    tSize = tSizes[2];
    nTileW = ((lSizeW*pCpu)/100)/tSize;
    nTileH = (lSizeH/tSize) + 1;
    std::cout << "Breaking small tiles..." << std::endl;
    breakTiles( "3", "rest", topX);
}

void Tiler::divSplit(const char* imgName, std::string out, int pGpu,
                     int pCpu, int oTiles[3]) {
    int pAve = 100 - pGpu - pCpu;
    if (pAve < 0) {
        std::cerr << "Error: Gpu percentage plus Cpu percentage\
                      must be less or equal to 100.\n";
        exit(-1);
    }

    const int ImageDimension = 2;

    osr = openslide_open(imgName);
    this->out = out;

    mag = gth818n::getMagnification(osr);

    lSizeW = 0;
    lSizeH = 0;
    lSizeLevel = 0;

    gth818n::getLargestLevelSize(osr, lSizeLevel, lSizeW, lSizeH);
    std::cout << "Largest level: " << lSizeLevel << " has size: " << lSizeW
              << ", " << lSizeH << std::endl;

    int nTiles[3];
    nTiles[0] = pow(2, oTiles[0]);
    float wGpuP = 100/pow(4, oTiles[0]);
    nTiles[0] = (nTiles[0]*nTiles[0]) - (float)(100 - pGpu)/wGpuP;
    nTiles[1] = (float)(pAve/(float)(100/pow(4, oTiles[1])));
    float wAveP = 100/pow(4, oTiles[1]);
    nTiles[2] = (float)(pCpu/(float)(100/pow(4, oTiles[2])));
    int coordinates[4] = {0, 0, lSizeW, lSizeH};
    std::cout << "nTiles = {" << nTiles[0] << ", " << nTiles[1] << ", "
              << nTiles[2] << "}" << std::endl;
    recursiveBreak(oTiles, nTiles, coordinates); 
}

void Tiler::recursiveBreak(int oValue[3], int nValue[3], int coordinates[4],
                           int o) {
    static int oSum[3] = {0, 0, 0};
    for (int i = 3; i >= 0; i--) {
        if (oValue[i] == o) {
            if ((oSum[i]+1) <= nValue[i]) {
                oSum[i]++;
                int64_t topLeftX = coordinates[0];
                int64_t topLeftY = coordinates[1];

                int64_t thisTileSizeX = std::min(lSizeW/nValue[i],
                                                 lSizeW - topLeftX);
                int64_t thisTileSizeY = std::min(lSizeH/nValue[i],
                                                 lSizeH - topLeftY);

                if (0 == thisTileSizeX || 0 == thisTileSizeY) continue;

                uint32_t* dest = new uint32_t[thisTileSizeX*thisTileSizeY];

                openslide_read_region(osr, dest, topLeftX, topLeftY, lSizeLevel,
                                      thisTileSizeX, thisTileSizeY);

                cv::Mat thisTile(thisTileSizeY, thisTileSizeX, CV_8UC3,
                                 cv::Scalar(0, 0, 0));
                gth818n::osrRegionToCVMat(dest, thisTile);

                delete[] dest;
                char tileName[1000];
                sprintf(tileName, "%s_%d_%d_%d_%d_%d_%d.tif", out.c_str(), o,
                        coordinates[0], coordinates[1], coordinates[2],
                        coordinates[3], mag);
                cv::imwrite(tileName, thisTile);
                return;
            }
            break;
        }
    }
    if (o < oValue[2]) {
        o++;
        int x = coordinates[0], y = coordinates[1];
        int w = coordinates[2], h = coordinates[3];
        int c[4][4] = {{x      , y      , w/2, h/2},
                       {x + w/2, y      , w/2, h/2},
                       {x      , y + h/2, w/2, h/2},
                       {x + w/2, y + h/2, w/2, h/2}};
        for (int i = 0; i < 4; i++)
            recursiveBreak(oValue, nValue, c[i], o); 
        return;
    }
    std::cout << "oSum[0] = " << oSum[0] << ", oSum[1] = " << oSum[1]
              << ", oSum[2] = " << oSum[2]<< std::endl;
}

void Tiler::breakTiles(std::string id, std::string rest, int64_t topX) {
    for (int64_t iTileW = 0; iTileW < nTileW; ++iTileW) {
        for (int64_t iTileH = 0; iTileH < nTileH; ++iTileH) {
            std::cout<<iTileW<<", "<<iTileH<<std::endl<<std::flush;

            int64_t topLeftX = topX + (iTileW*tSize);
            int64_t topLeftY = (iTileH*tSize);

            int64_t thisTileSizeX = std::min(tSize, lSizeW - topLeftX);
            int64_t thisTileSizeY = std::min(tSize, lSizeH - topLeftY);

            if (0 == thisTileSizeX || 0 == thisTileSizeY) continue;

            uint32_t* dest = new uint32_t[thisTileSizeX*thisTileSizeY];

            openslide_read_region(osr, dest, topLeftX, topLeftY, lSizeLevel,
                                  thisTileSizeX, thisTileSizeY);

            cv::Mat thisTile(thisTileSizeY, thisTileSizeX, CV_8UC3,
                             cv::Scalar(0, 0, 0));
            gth818n::osrRegionToCVMat(dest, thisTile);

            delete[] dest;
            char tileName[1000];
            if (thisTileSizeX < tSize || thisTileSizeY < tSize) {
                sprintf(tileName, "%s_%s_%ld_%ld_%ld_%ld_%d.tif",
                        out.c_str(), rest.c_str(), thisTileSizeX,
                        thisTileSizeY, topLeftX, topLeftY, mag);
            } else {
                sprintf(tileName, "%s_%s_%ld_%ld_%ld_%ld_%d.tif",
                        out.c_str(), id.c_str(), tSize, tSize,
                        topLeftX, topLeftY, mag);
            }
            cv::imwrite(tileName, thisTile);
        }
    }
}

