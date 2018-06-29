#include "Tiler.h"

#include <cstdio>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "utilitiesSvs.h"

Tiler::Tiler() {}

Tiler::~Tiler() {}

std::vector<BoundingBox> Tiler::defaultSplit(const char* imgName, int64_t tSize,
                                             float threshold) {
    const int ImageDimension = 2;
    
    this->tSize = tSize;

    openslide_t *osr = openslide_open(imgName);
    int mag = gth818n::getMagnification(osr);

    lSizeW = 0;
    lSizeH = 0;
    lSizeLevel = 0;

    gth818n::getLargestLevelSize(osr, lSizeLevel, lSizeW, lSizeH);

# ifdef TILER_DEBUG
        std::cout << "Largest level: " << lSizeLevel << " has size: " << lSizeW
            << ", " << lSizeH << std::endl;
#endif

    nTileW = lSizeW/tSize + 1;
    nTileH = lSizeH/tSize + 1;

    return breakTiles();
}

std::vector<BoundingBox> Tiler::approxSplit(const char* imgName,
                                            unsigned int pGpu,
                                            unsigned int pCpu,
                                            int64_t tSizes[3]) {
    int pAve = 100 - pGpu - pCpu;
    if (pAve < 0) {
        std::cerr << "Error: Gpu percentage plus Cpu percentage\
                      must be less or equal to 100.\n";
        exit(-1);
    }

    const int ImageDimension = 2;

    openslide_t *osr = openslide_open(imgName);
    int mag = gth818n::getMagnification(osr);

    lSizeW = 0;
    lSizeH = 0;
    lSizeLevel = 0;

    gth818n::getLargestLevelSize(osr, lSizeLevel, lSizeW, lSizeH);
# ifdef TILER_DEBUG
        std::cout << "Largest level: " << lSizeLevel << " has size: " << lSizeW
            << ", " << lSizeH << std::endl;
#endif

    tSize = tSizes[0];
    nTileW = ((lSizeW*pGpu)/100)/tSize;
    nTileH = (lSizeH/tSize) + 1;

# ifdef TILER_DEBUG
    std::cout << "Breaking large tiles..." << std::endl;
#endif
    std::vector<BoundingBox> auxTiles, bTiles = breakTiles();
    int64_t topX = nTileW*tSize;

    if (pAve > 0) {
        tSize = tSizes[1];
        nTileW = ((lSizeW*pAve)/100)/tSize;
        nTileH = (lSizeH/tSize) + 1;
        // std::cout << "Breaking medium tiles..." << std::endl;
        auxTiles = breakTiles(topX);
        bTiles.insert(bTiles.end(), auxTiles.begin(), auxTiles.end());
        topX = nTileW*tSize;
    }

    tSize = tSizes[2];
    nTileW = ((lSizeW*pCpu)/100)/tSize;
    nTileH = (lSizeH/tSize) + 1;
# ifdef TILER_DEBUG
    std::cout << "Breaking small tiles..." << std::endl;
#endif
    auxTiles = breakTiles(topX);
    bTiles.insert(bTiles.end(), auxTiles.begin(), auxTiles.end());

    return bTiles;
}

std::vector<BoundingBox> Tiler::divSplit(const char* imgName, int pGpu,
                                         int pCpu, int oTiles[3]) {
    if ((oTiles[0] > oTiles[1]) || (oTiles[1] > oTiles[2])) {
        std::cerr << "Error: Gpu order must be lower than average order and "
                  << "average order must be lower than Cpu order." << std::endl;
    }

    int pAve = 100 - pGpu - pCpu;
    if (pAve < 0) {
        std::cerr << "Error: Gpu percentage plus Cpu percentage\
                      must be less or equal to 100.\n";
        exit(-1);
    }

    const int ImageDimension = 2;

    openslide_t *osr = openslide_open(imgName);
    int mag = gth818n::getMagnification(osr);

    lSizeW = 0;
    lSizeH = 0;
    lSizeLevel = 0;

    gth818n::getLargestLevelSize(osr, lSizeLevel, lSizeW, lSizeH);
    
# ifdef TILER_DEBUG
        std::cout << "Largest level: " << lSizeLevel << " has size: " << lSizeW
            << ", " << lSizeH << std::endl;
#endif

    int nTiles[3] = {0, 0, 0};
    nTiles[0] = pow(2, oTiles[0]);
    float wGpuP = 100/pow(4, oTiles[0]);
    nTiles[0] = (nTiles[0]*nTiles[0]) - (float)(100 - pGpu)/wGpuP;
    nTiles[1] = (float)(pAve/(float)(100/pow(4, oTiles[1])));
    float wAveP = 100/pow(4, oTiles[1]);
    nTiles[2] = (float)(pCpu/(float)(100/pow(4, oTiles[2])));
    int coordinates[4] = {0, 0, lSizeW, lSizeH};
# ifdef TILER_DEBUG
        std::cout << "nTiles = {" << nTiles[0] << ", " << nTiles[1] << ", "
            << nTiles[2] << "}" << std::endl;
#endif

    return recursiveBreak(oTiles, nTiles, coordinates); 
}

void Tiler::printTiles(std::vector<BoundingBox> bTiles) {
    std::cout << "[" << std::endl;
    std::vector<BoundingBox>::iterator it = bTiles.begin();

    for (it = bTiles.begin(); it < bTiles.end(); it++) {
        Point ub = it->getUb(), lb = it->getLb();

        std::cout << "    {(" << ub.getX() << ", " << ub.getY() << "), ";
        std::cout << "(" << lb.getX() << ", " << lb.getY() << ")}" << std::endl;
    }
    std::cout << "]" << std::endl;
}

std::vector<BoundingBox> Tiler::recursiveBreak(int oValue[3], int nValue[3],
                                               int coordinates[4], int o) {
    std::vector<BoundingBox> bTiles, auxTiles;
    static int oSum[3] = {0, 0, 0};
    int x = coordinates[0], y = coordinates[1];
    int w = coordinates[2], h = coordinates[3];

    for (int i = 2; i >= 0; i--) {
        if (oValue[i] == o) {
            if ((oSum[i]+1) <= nValue[i]) {
                oSum[i]++;

                Point ub(x, y);
                Point lb(x + w, y + h);
                bTiles.insert(bTiles.end(), BoundingBox(lb, ub));
                return bTiles;
            }
            break;
        }
    }
    if (o < oValue[2]) {
        o++;
        int c[4][4] = {{x      , y      , w/2, h/2},
                       {x + w/2, y      , w/2, h/2},
                       {x      , y + h/2, w/2, h/2},
                       {x + w/2, y + h/2, w/2, h/2}};
        for (int i = 0; i < 4; i++) {
            auxTiles = recursiveBreak(oValue, nValue, c[i], o); 
            bTiles.insert(bTiles.end(), auxTiles.begin(), auxTiles.end());
        }
        return bTiles;
    }
# ifdef TILER_DEBUG
        std::cout << "oSum[0] = " << oSum[0] << ", oSum[1] = " << oSum[1]
            << ", oSum[2] = " << oSum[2] << std::endl;
#endif
    return bTiles;
}

std::vector<BoundingBox> Tiler::breakTiles(int topX) {
    std::vector<BoundingBox> bTiles;
    
    for (int iTileW = 0; iTileW < nTileW; ++iTileW) {
        for (int iTileH = 0; iTileH < nTileH; ++iTileH) {
# ifdef TILER_DEBUG
            std::cout<<iTileW<<", "<<iTileH<<std::endl<<std::flush;
#endif

            int topLeftX = topX + (iTileW*tSize);
            int topLeftY = (iTileH*tSize);

            int64_t thisTileSizeX = std::min(tSize, lSizeW - topLeftX);
            int64_t thisTileSizeY = std::min(tSize, lSizeH - topLeftY);

            if (thisTileSizeX && thisTileSizeY) {
                Point ub(topLeftX, topLeftY);
                Point lb(topLeftX + tSize, topLeftY + tSize);
                bTiles.insert(bTiles.end(), BoundingBox(lb, ub));
            }
        }
    }

    return bTiles;
}
