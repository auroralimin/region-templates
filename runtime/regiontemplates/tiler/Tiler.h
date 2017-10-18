#ifndef TILER_H_
#define TILER_H_

#include <string>
#include <vector>
#include <algorithm>

#include "openslide.h"
#include "BoundingBox.h"

class Tiler {
    public:
	    Tiler();
	    virtual ~Tiler();
        
        std::vector<BoundingBox> defaultSplit(const char* imgName,
                                              int64_t tileSize,
                                              float threshold = 175.0f);
        std::vector<BoundingBox> approxSplit(const char* imgName,
                                             unsigned int pGpu,
                                             unsigned int pCpu,
                                             int64_t tSizes[3]);
        std::vector<BoundingBox> divSplit(const char* imgName, int pGpu,
                                          int pCpu, int oTiles[3]);
        void printTiles(std::vector<BoundingBox> bTiles);

    private:
        std::vector<BoundingBox> breakTiles(int topX = 0);
        std::vector<BoundingBox> recursiveBreak(int oValue[3], int nValue[3],
                                                int coordinates[4], int o = 0);
        int32_t lSizeLevel;
        int64_t nTileW, nTileH, tSize, lSizeW, lSizeH;
};

#endif

