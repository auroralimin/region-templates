#include "QuadTree.h"

#include <cstdlib>

#include "HistologicalEntities.h"

QuadTree::QuadTree(std::string file, int32_t level) {
    nodes = NULL;
    osr = openslide_open(file.c_str());
    ratio = 1.0f;
    og = 0;
    this->level = level;
    isRoot = true;

    int64_t lSizeW = 0, lSizeH = 0;
    gth818n::getLevelSize(this->level, osr, lSizeW, lSizeH);
    Point ub(0, 0);
    Point lb(lSizeW, lSizeH);
    setBb(BoundingBox(lb, ub));
}

QuadTree::QuadTree(openslide_t *osr, int og, int32_t level) {
    nodes = NULL;
    this->osr = osr;
    ratio = 0.0f;
    this->og = og;
    this->level = level;
    isRoot = false;
    setBb(BoundingBox());
}

QuadTree::~QuadTree() {
    if (nodes == NULL) {
        return;
    }
    for (int i = 0; i < 4; i++) {
        delete nodes[i];
    }
    if (osr != NULL && isRoot)
        openslide_close(osr);
}

void QuadTree::subdivide() {
    if (nodes) {
        for (int i = 0; i < 4; i++) {
            if (nodes[i]) {
                nodes[i]->subdivide();
            }
        }
    } else {
        nodes = (QuadTree**) calloc(4, sizeof(QuadTree*));
        for (int i = 0; i < 4; i++) {
            nodes[i] = new QuadTree(osr, og + 1, level);
        }
        setNodesBb();
    }
}

void QuadTree::calculateRatios() {
    if (nodes == NULL) {
        int64_t sublevel = openslide_get_level_count(osr) - 1;
        double downsample = openslide_get_level_downsample(osr, sublevel);
        int64_t topLeftX = bb.getUb().getX();
        int64_t topLeftY = bb.getUb().getY();
        int64_t thisTileSizeX = (bb.getLb().getX() - bb.getUb().getX())/downsample;
        int64_t thisTileSizeY = (bb.getLb().getY() - bb.getUb().getY())/downsample;
        uint32_t* dest = new uint32_t[thisTileSizeX*thisTileSizeY];

        openslide_read_region(osr, dest, topLeftX, topLeftY, sublevel,
                              thisTileSizeX, thisTileSizeY);
        cv::Mat chunkData(thisTileSizeY, thisTileSizeX, CV_8UC3,
                          cv::Scalar(0, 0, 0));
        gth818n::osrRegionToCVMat(dest, chunkData);
        ratio = backgroundRatio(chunkData);
        delete[] dest;
    } else {
        float n = 0.0f;
        for (int i = 0; i < 4; i++) {
            if (nodes[i]) {
                n++;
                nodes[i]->calculateRatios();
                ratio += nodes[i]->getRatio();
            }
        }
        ratio /= n;
    }
}

void QuadTree::merge() {
    if (nodes == NULL) {
        return;
    }

    for (int i = 0; i < 4; i++) {
        if (nodes[i] != NULL) {
            nodes[i]->merge();
            delete nodes[i];
            nodes[i] = NULL;
        }
    }
    nodes = NULL;
}

void QuadTree::backgroundAwareMerge(int idealOg, float minSplit, float minRatio) {
    if (og < idealOg && nodes) {
        // Se for menor que o og ideal, nao faz merge mas pede para os filhos fazerem
        for (int i = 0; i < 4; i++) {
            if (nodes[i]) {
                nodes[i]->backgroundAwareMerge(idealOg, minSplit, minRatio);
            }
        }
        return;
    }

    // Se nao tem filhos o suficiente sendo background
    if (ratio > minSplit) {
        std::cout << bb << " (ratio = " << ratio << "): Merged!" << std::endl;
        merge();
    // Se tem
    } else {
        if (!nodes) {
            return;
        }
        for (int i = 0; i < 4; i++) {
            if (nodes[i]) {
                if (nodes[i]->getRatio() <= minRatio) {
                    std::ofstream out;
                    out.open("backgrounds.csv", std::ios_base::app);
                    out << "\"" << nodes[i]->getBb() << "\"" << std::endl;
                    out.close();
                    std::cout << nodes[i]->getBb() << " (ratio = " << ratio << "): Background ignored!" << std::endl;
                    //saveTiff(nodes[i]->getBb());
                    delete nodes[i];
                    nodes[i] = NULL;
               } else {
                   nodes[i]->backgroundAwareMerge(idealOg, minSplit, minRatio);
               }
            }
        }
    }
}

void QuadTree::recalculateBbs(int32_t level) {
    if (this->level == level) {
        return;
    }

    this->level = level;
    if (isRoot) {
        int64_t lSizeW = 0, lSizeH = 0;
        gth818n::getLevelSize(level, osr, lSizeW, lSizeH);
        Point ub(0, 0);
        Point lb(lSizeW, lSizeH);
        setBb(BoundingBox(lb, ub));
    }
    if (nodes) {
        setNodesBb();
        for (int i = 0; i < 4; i++) {
            if (nodes[i]) {
                nodes[i]->recalculateBbs(level);
            }
        }
    }
}

std::vector<QuadTree> QuadTree::treeToVec() {
    std::vector<QuadTree> qtVector, nVector;
    if (nodes == NULL) {
        qtVector.insert(qtVector.end(), *this);
        return qtVector;
    }

    for (int i = 0; i < 4; i++) {
        if (nodes[i]) {
            nVector = nodes[i]->treeToVec();
            qtVector.insert(qtVector.end(), nVector.begin(), nVector.end());
        }
    }

    return qtVector;
}

QuadTree** QuadTree::getNodes() {
    return nodes;
}

int QuadTree::getOg() {
    return og;
}

void QuadTree::setOg(int og) {
    this->og = og;
}

float QuadTree::getRatio() {
    return ratio;
}

BoundingBox QuadTree::getBb() {
    return bb;
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

void QuadTree::setBb(BoundingBox bb) {
    this->bb = bb;
}

void QuadTree::setNodesBb() {
    int x = bb.getUb().getX(); 
    int y = bb.getUb().getY(); 
    int w = bb.getLb().getX() - x, h = bb.getLb().getY() - y;

    if (nodes[0])
        nodes[0]->setBb(BoundingBox(Point(x + (w/2), y + (h/2)), bb.getUb()));
    if (nodes[1])
        nodes[1]->setBb(BoundingBox(Point(x + w, y + (h/2)), Point(x + (w/2), y)));
    if (nodes[2])
        nodes[2]->setBb(BoundingBox(Point(x + (w/2), y + h), Point(x, y + (h/2))));
    if (nodes[3])
        nodes[3]->setBb(BoundingBox(bb.getLb(), Point(x + (w/2), y + (h/2))));
}

float QuadTree::backgroundRatio(cv::Mat bgr) {
    unsigned char blue = 220, green = 220, red = 220;
    ::cciutils::SimpleCSVLogger *logger = NULL;
    ::cciutils::cv::IntermediateResultHandler *iresHandler = NULL;
    cv::Mat background = ::nscale::HistologicalEntities::getBackground(bgr,
            blue, green, red, logger, iresHandler);
    int bgArea = countNonZero(background);
    return (1.0f - ((float)bgArea / (float)(bgr.size().area())));
}

void QuadTree::saveTiff(BoundingBox ibb) {
    int64_t topLeftX = ibb.getUb().getX();
    int64_t topLeftY = ibb.getUb().getY();
    int64_t thisTileSizeX = ibb.getLb().getX() - ibb.getUb().getX();
    int64_t thisTileSizeY = ibb.getLb().getY() - ibb.getUb().getY();
    uint32_t* dest = new uint32_t[thisTileSizeX*thisTileSizeY];
    openslide_read_region(osr, dest, topLeftX, topLeftY, level,
            thisTileSizeX, thisTileSizeY);
    cv::Mat chunkData(thisTileSizeY, thisTileSizeX, CV_8UC3,
            cv::Scalar(0, 0, 0));
    gth818n::osrRegionToCVMat(dest, chunkData);
    std::ostringstream oss;
    oss <<  "temp/" << ibb << ".tiff";
    cv::imwrite(oss.str(), chunkData);
    delete[] dest;
}

