#include "Tiler.h"
#include <string>
#include <iostream>

int main(int argc, char **argv)
{
    if (argc < 8)
    {
        std::cerr << "Parameters: imgName outputPrefix Gpu\% Cpu\% oMin oMax oAverage\n";
        exit(-1);
    }

    const char* imgName = argv[1];
    std::string outPrefix(argv[2]);
    int pGpu = strtoul(argv[3], NULL, 0);
    int pCpu = strtoul(argv[4], NULL, 0);
    int oTiles[3] = {strtoll(argv[5], NULL, 10), strtoll(argv[6], NULL, 10), strtoll(argv[7], NULL, 10)};

    divSplit(imgName, outPrefix, pGpu, pCpu, oTiles);

    return 0;
}

