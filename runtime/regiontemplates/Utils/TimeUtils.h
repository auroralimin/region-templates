#ifndef TIME_UTILS_H_
#define TIME_UTILS_H_

#include <string>
#include <map>

class TimeUtils {
    public:
        TimeUtils(){};
        TimeUtils(std::string label);
        
        void markTimeUS(std::string label);
        void markDiffUS(std::string l1, std::string l2, std::string name);
        int getDiffUS(std::string name);
        void printDiff(std::string name);
        void printDiffs();
        void outCsv(std::string strOut);

    private:
        long long timestampInUS();

        std::map<std::string, long long> times, diffs;
};

#endif

