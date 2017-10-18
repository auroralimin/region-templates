#ifndef TILER_H_
#define TILER_H_

#include <string>
#include <vector>
#include <algorithm>

#include "openslide.h"

class Tiler {
    public:
	    Tiler();
	    virtual ~Tiler();
        
        void defaultSplit(const char* imgName, std::string out,
                                         int64_t tileSize,
                                         float threshold = 175.0f);
        void approxSplit(const char* imgName, std::string out,
                                        unsigned int pGpu, unsigned int pCpu,
                                        int64_t tSizes[3]);
        void divSplit(const char* imgName, std::string out,
                      int pGpu, int pCpu, int oTiles[3]);

    private:
        void breakTiles(std::string id,
                        std::string rest = "rest", int64_t topX = 0);
        void recursiveBreak(int oValue[3], int nValue[3], int coordinates[4],
                            int o = 0);

        openslide_t *osr;
        int64_t nTileW, nTileH, tSize, lSizeW, lSizeH;
        int32_t lSizeLevel;
        int mag;
        std::string out;
};

#endif

