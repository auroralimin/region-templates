#include "TimeUtils.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <time.h>
#include <sys/time.h>

#include "OutUtils.h"

TimeUtils::TimeUtils(std::string label) {
    times[label] = timestampInUS(); 
}

void TimeUtils::markTimeUS(std::string label) {
    times[label] = timestampInUS(); 
}

void TimeUtils::markDiffUS(std::string l1, std::string l2, std::string name) {
    long long t1 = times[l1], t2 = times[l2];
    diffs[name] = llabs(t2 - t1); 
}

int TimeUtils::getDiffUS(std::string name) {
    return diffs[name];
}

void TimeUtils::printDiff(std::string name) {
    std::cout << COLOR(magenta) << "[TIME UTILS] " << OFF << name 
              << ": " << diffs[name] << " us" << std::endl;
}

void TimeUtils::printDiffs() {
    std::map<std::string, long long>::iterator d;
    for (d = diffs.begin(); d != diffs.end(); d++) {
        std::cout << COLOR(magenta) << "[TIME UTILS] " << OFF << d->first 
                  << ": " << d->second << " us" << std::endl;
    }
}

void TimeUtils::outCsv(std::string strOut) {
	std::ofstream out;
	out.open(strOut.c_str(), std::ios_base::app);
    
    std::map<std::string, long long>::iterator d;
    for (d = diffs.begin(); d != diffs.end(); d++) {
        out << "\"" << d->first << "\"" << "," << d->second << std::endl;
    }
	out.close();
}

long long TimeUtils::timestampInUS() {
    struct timeval ts;
    gettimeofday(&ts, NULL);
    return (ts.tv_sec * 1000000LL + (ts.tv_usec));
}

