#include "Tiler.h"

#include <cstdio>
#include <iostream>

// openslide
#include "openslide.h"

// openCV
#include <opencv2/opencv.hpp>

// local
#include "utilitiesSvs.h"

struct tileInfo_t{
    openslide_t *osr;
    int64_t nTileW, nTileH, tSize, lSizeW, lSizeH;
    int32_t lSizeLevel;
    int mag;
    std::string outStr;
};


void breakTiles(struct tileInfo_t info, std::string id, std::string rest = "rest", int64_t topX = 0);
void recursiveBreak(int oValue[3], int nValue[3], int coordinates[4], struct tileInfo_t info, int o = 0);

void defaultSplit(const char* imgName, std::string outStr, int64_t tSize, float threshold)
{
    const int ImageDimension = 2;
    struct tileInfo_t info;

    info.osr = openslide_open(imgName);
    info.outStr = outStr;

    int mag = gth818n::getMagnification(info.osr);

    info.lSizeW = 0;
    info.lSizeH = 0;
    info.lSizeLevel = 0;

    gth818n::getLargestLevelSize(info.osr, info.lSizeLevel, info.lSizeW, info.lSizeH);

    std::cout<<"Largest level: "<< info.lSizeLevel<<" has size: " << info.lSizeW << ", " << info.lSizeH << std::endl;

    info.nTileW = info.lSizeW/info.tSize + 1;
    info.nTileH = info.lSizeH/info.tSize + 1;

    breakTiles(info, outStr, "");
}

void approxSplit(const char* imgName, std::string outStr, unsigned int pGpu, unsigned int pCpu, int64_t tMin, int64_t tMax, int64_t tAve)
{
    int pAve = 100 - pGpu - pCpu;
    if (pAve < 0)
    {
        std::cerr<<"Error: Gpu percentage plus Cpu percentage must be less or equal to 100.\n";
        exit(-1);
    }

    const int ImageDimension = 2;
    struct tileInfo_t info;

    info.osr = openslide_open(imgName);
    info.outStr = outStr;

    info.mag = gth818n::getMagnification(info.osr);

    info.lSizeW = 0;
    info.lSizeH = 0;
    info.lSizeLevel = 0;

    gth818n::getLargestLevelSize(info.osr, info.lSizeLevel, info.lSizeW, info.lSizeH);
    std::cout<<"Largest level: "<< info.lSizeLevel<<" has size: " << info.lSizeW << ", " << info.lSizeH << std::endl;

    info.tSize = tMax;
    info.nTileW = ((info.lSizeW*pGpu)/100)/info.tSize;
    info.nTileH = (info.lSizeH/info.tSize) + 1;

    std::cout << "Breaking large tiles..." << std::endl;
    breakTiles(info, "1", "2");
    int64_t topX = info.nTileW*info.tSize;

    if (pAve > 0) {
        info.tSize = tAve;
        info.nTileW = ((info.lSizeW*pAve)/100)/info.tSize;
        info.nTileH = (info.lSizeH/info.tSize) + 1;
        std::cout << "Breaking medium tiles..." << std::endl;
        breakTiles(info, "2", "3", topX);
        topX = info.nTileW*info.tSize;
    }

    info.tSize = tMin;
    info.nTileW = ((info.lSizeW*pCpu)/100)/info.tSize;
    info.nTileH = (info.lSizeH/info.tSize) + 1;
    std::cout << "Breaking small tiles..." << std::endl;
    breakTiles(info, "3", "rest", topX);
}

void divSplit(const char* imgName, std::string outStr, int pGpu, int pCpu, int oTiles[3])
{
    int pAve = 100 - pGpu - pCpu;
    if (pAve < 0)
    {
        std::cerr<<"Error: Gpu percentage plus Cpu percentage must be less or equal to 100.\n";
        exit(-1);
    }

    const int ImageDimension = 2;
    struct tileInfo_t info;

    info.osr = openslide_open(imgName);
    info.outStr = outStr;

    info.mag = gth818n::getMagnification(info.osr);

    info.lSizeW = 0;
    info.lSizeH = 0;
    info.lSizeLevel = 0;

    gth818n::getLargestLevelSize(info.osr, info.lSizeLevel, info.lSizeW, info.lSizeH);
    std::cout<<"Largest level: "<< info.lSizeLevel<<" has size: " << info.lSizeW << ", " << info.lSizeH << std::endl;

    int nTiles[3];
    nTiles[0] = pow(2, oTiles[0]);
    float wGpuP = 100/pow(4, oTiles[0]);
    nTiles[0] = (nTiles[0]*nTiles[0]) - (float)(100 - pGpu)/wGpuP;
    nTiles[1] = (float)(pAve/(float)(100/pow(4, oTiles[1])));
    float wAveP = 100/pow(4, oTiles[1]);
    nTiles[2] = (float)(pCpu/(float)(100/pow(4, oTiles[2])));
    int coordinates[4] = {0, 0, info.lSizeW, info.lSizeH};
    std::cout << "nTiles = {" << nTiles[0] << ", " << nTiles[1] << ", " << nTiles[2] << "}" << std::endl;
    recursiveBreak(oTiles, nTiles, coordinates, info); 
}

void recursiveBreak(int oValue[3], int nValue[3], int coordinates[4], struct tileInfo_t info, int o)
{
    static int oSum[3] = {0, 0, 0};
    for (int i = 3; i >= 0; i--)
    {
        if (oValue[i] == o)
        {
            if ((oSum[i]+1) <= nValue[i]) {
                oSum[i]++;
                //std::cout << "SALVA IMAGEM o = " << o << std::endl;
                int64_t topLeftX = coordinates[0];
                int64_t topLeftY = coordinates[1];

                int64_t thisTileSizeX = std::min(info.lSizeW/nValue[i], info.lSizeW - topLeftX);
                int64_t thisTileSizeY = std::min(info.lSizeH/nValue[i], info.lSizeH - topLeftY);

                if (0 == thisTileSizeX || 0 == thisTileSizeY)
                {
                    continue;
                }

                uint32_t* dest = new uint32_t[thisTileSizeX*thisTileSizeY];

                openslide_read_region(info.osr, dest, topLeftX, topLeftY, info.lSizeLevel, thisTileSizeX, thisTileSizeY);

                cv::Mat thisTile(thisTileSizeY, thisTileSizeX, CV_8UC3, cv::Scalar(0, 0, 0));
                gth818n::osrRegionToCVMat(dest, thisTile);

                delete[] dest;
                char tileName[1000];
                sprintf(tileName, "%s_%d_%d_%d_%d_%d_%d.tif", info.outStr.c_str(), o,
                        coordinates[0], coordinates[1], coordinates[2], coordinates[3], info.mag);
                cv::imwrite(tileName, thisTile);
                return;
            }
            break;
        }
    }
    if (o < oValue[2])
    {
        o++;
        int x = coordinates[0], y = coordinates[1], w = coordinates[2], h = coordinates[3];
        int c[4][4] = {{x      , y      , w/2, h/2},
                       {x + w/2, y      , w/2, h/2},
                       {x      , y + h/2, w/2, h/2},
                       {x + w/2, y + h/2, w/2, h/2}};
        for (int i = 0; i < 4; i++)
            recursiveBreak(oValue, nValue, c[i], info, o); 
        return;
    }
    std::cout << "oSum[0] = " << oSum[0] << ", oSum[1] = " << oSum[1]  << ", oSum[2] = " << oSum[2]<< std::endl;
}

void breakTiles(struct tileInfo_t info, std::string id, std::string rest, int64_t topX)
{
    for (int64_t iTileW = 0; iTileW < info.nTileW; ++iTileW)
    {
        for (int64_t iTileH = 0; iTileH < info.nTileH; ++iTileH)
        {
            std::cout<<iTileW<<", "<<iTileH<<std::endl<<std::flush;

            int64_t topLeftX = topX + (iTileW*info.tSize);
            int64_t topLeftY = (iTileH*info.tSize);

            int64_t thisTileSizeX = std::min(info.tSize, info.lSizeW - topLeftX);
            int64_t thisTileSizeY = std::min(info.tSize, info.lSizeH - topLeftY);

            if (0 == thisTileSizeX || 0 == thisTileSizeY)
            {
                continue;
            }

            uint32_t* dest = new uint32_t[thisTileSizeX*thisTileSizeY];

            openslide_read_region(info.osr, dest, topLeftX, topLeftY, info.lSizeLevel, thisTileSizeX, thisTileSizeY);

            cv::Mat thisTile(thisTileSizeY, thisTileSizeX, CV_8UC3, cv::Scalar(0, 0, 0));
            gth818n::osrRegionToCVMat(dest, thisTile);

            delete[] dest;
            char tileName[1000];
            if (thisTileSizeX < info.tSize || thisTileSizeY < info.tSize)
            {
                sprintf(tileName, "%s_%s_%ld_%ld_%ld_%ld_%d.tif", info.outStr.c_str(), rest.c_str(), thisTileSizeX, thisTileSizeY, topLeftX, topLeftY, info.mag);
            }
            else
            {
                sprintf(tileName, "%s_%s_%ld_%ld_%ld_%ld_%d.tif", info.outStr.c_str(), id.c_str(), info.tSize, info.tSize, topLeftX, topLeftY, info.mag);
            }
            cv::imwrite(tileName, thisTile);
        }
    }
}

