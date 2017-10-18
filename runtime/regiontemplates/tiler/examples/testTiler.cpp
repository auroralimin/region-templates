#include "Tiler.h"
#include <string>
#include <iostream>

int main(int argc, char **argv)
{
    if (argc < 8)
    {
        std::cerr << "Parameters:\
                     imgName outputPrefix Gpu\% Cpu\% oMin oAverage oMax\n";
        exit(-1);
    }

    const char* imgName = argv[1];
    std::string outPrefix(argv[2]);
    int pGpu = strtoul(argv[3], NULL, 0);
    int pCpu = strtoul(argv[4], NULL, 0);
    int oTiles[3] = {strtoll(argv[5], NULL, 10), strtoll(argv[6], NULL, 10),
                     strtoll(argv[7], NULL, 10)};
    int64_t tSizes[3] = {200, 300, 500};

    Tiler tiler;
    tiler.defaultSplit(imgName, outPrefix + "_default", 500);
    tiler.approxSplit(imgName, outPrefix + "_approx", pGpu, pCpu, tSizes);
    tiler.divSplit(imgName, outPrefix + "_div", pGpu, pCpu, oTiles);

    return 0;
}

