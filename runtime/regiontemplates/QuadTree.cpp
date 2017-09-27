#include "QuadTree.h"

#include<cstdlib>

QuadTree::QuadTree(BoundingBox root) {
    value = root;
    nodes = NULL;
    isUsed = false;
}

QuadTree::~QuadTree() {
    if (nodes == NULL) {
        return;
    }
    for (int i = 0; i < 4; i++) {
        delete nodes[i];
    }
}

void QuadTree::subdivide() {
    nodes = (QuadTree**) calloc(4, sizeof(QuadTree*));

    int x = value.getUb().getX(); 
    int y = value.getUb().getY(); 
    int w = value.getLb().getX() - x, h = value.getLb().getY() - y;

    nodes[0]->setValue(BoundingBox(Point(x + (w/2), y + (h/2)), value.getUb()));
    nodes[1]->setValue(BoundingBox(Point(x + w, y + (h/2)), Point(x + (w/2), y)));
    nodes[2]->setValue(BoundingBox(Point(x + (w/2), y + h), Point(x, y + (h/2))));
    nodes[1]->setValue(BoundingBox(value.getLb(), Point(x + (w/2), y + (h/2))));
}

void QuadTree::merge() {
    if (nodes == NULL) {
        return;
    }
    for (int i = 0; i < 4; i++) {
        delete nodes[i];
    }
}

QuadTree** QuadTree::getNodes() {
    return nodes;
}

BoundingBox QuadTree::getValue() {
    return value;
}

QuadTree* QuadTree::getNW() {
    return nodes != NULL ? nodes[0] : NULL;
}

QuadTree* QuadTree::getNE() {
    return nodes != NULL ? nodes[1] : NULL;
}

QuadTree* QuadTree::getSW() {
    return nodes != NULL ? nodes[2] : NULL;
}

QuadTree* QuadTree::getSE() {
    return nodes != NULL ? nodes[3] : NULL;
}

void QuadTree::setValue(BoundingBox value) {
    this->value = value;
}

