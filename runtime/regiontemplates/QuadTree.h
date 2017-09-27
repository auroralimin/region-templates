#ifndef QUAD_TREE_H_
#define QUAD_TREE_H_

#include "BoundingBox.h"

class QuadTree {
    private:
        QuadTree **nodes;
        BoundingBox value;
        bool isUsed;

        void setValue(BoundingBox value);

    public:
        QuadTree(BoundingBox root);
        ~QuadTree();

        void subdivide();
        void merge();
        QuadTree** getNodes();
        BoundingBox getValue();
        QuadTree* getNW();
        QuadTree* getNE();
        QuadTree* getSW();
        QuadTree* getSE();
};

#endif

