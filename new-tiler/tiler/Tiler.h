#ifndef TILER_H_
#define TILER_H_

#include <string>
#include <algorithm>

void defaultSplit(const char* imgName, std::string outStr, int64_t tileSize, float threshold = 175.0f);
void approxSplit(const char* imgName, std::string outStr, unsigned int pGpu, unsigned int pCpu,
                    int64_t tMin, int64_t tMax, int64_t tAve);
void divSplit(const char* imgName, std::string outStr, int pGpu, int pCpu, int oTiles[3]);
#endif

