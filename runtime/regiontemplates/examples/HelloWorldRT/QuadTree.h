#ifndef QUAD_TREE_H_
#define QUAD_TREE_H_

#include <vector>

#include "BoundingBox.h"
#include "utilitiesSvs.h"

class QuadTree {
   public:
        QuadTree(std::string file, int32_t level = 0);
        ~QuadTree();

        void subdivide();
        void calculateRatios();
        void merge();
        void backgroundAwareMerge(int idealOg,
                                  float minSplit = 0.1,
                                  float minRatio = 0.01);
        void recalculateBbs(int32_t level = 0);
        std::vector<QuadTree> treeToVec();
        QuadTree** getNodes();
        int getOg();
        void setOg(int og);
        float getRatio();
        BoundingBox getBb();
        QuadTree* getNW();
        QuadTree* getNE();
        QuadTree* getSW();
        QuadTree* getSE();

    private:
        QuadTree(openslide_t *osr, int og, int32_t level = 0);

        QuadTree **nodes;
        openslide_t *osr; 
        float ratio;
        int og;
        int32_t level;
        bool isRoot;
        BoundingBox bb;

        void setBb(BoundingBox bb);
        void setNodesBb();
        float backgroundRatio(cv::Mat bgr);
        void saveTiff(BoundingBox ibb);
};

#endif

