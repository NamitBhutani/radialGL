#pragma once
#include "tree.h"
#include "graphics.h"
#include <vector>
#include <set>

#define M_PI 3.14159265358979323846

struct Wedge
{
    Point center;
    float radius;
    float start_angle;
    float end_angle;
};

class TreeLayout
{
public:
    TreeLayout(const Tree &tree, const int halfwidth, const int halfheight);

    void calculateLayoutFromRoot(int rootID);
    void calculateTrueCenterLayout();
    void prepareFindCenterAnimation();

    void setDelta(float newDelta);
    float getDelta() const;

    float getHalfWidth() const;
    float getHalfHeight() const;

    const std::vector<Point> &getTargetPositions() const;
    const std::vector<int> &getCenterNodes() const;
    const std::vector<int> &getDepths() const;
    const std::vector<int> &getWidths() const;
    int getMaxDepth() const;
    const std::vector<int> &getParentMap() const;
    const std::vector<std::vector<int>> &getPruningGenerations() const;
    const std::set<float> &getFrameworkCircles() const;
    const std::vector<Wedge> &getFrameworkWedges() const;

private:
    void findCenter();
    void computeWidthsAndDepths(int u, int p, int d);
    void layoutSubTree(int u, int p, float alpha1, float alpha2);
    void resetLayoutState();
    void finalizeLayout();

    const Tree &tree_ref;
    float DELTA;
    int halfwidth, halfheight;

    std::vector<Point> target_positions;
    std::vector<int> center_nodes, true_center_nodes;
    std::vector<int> widths, depths;
    int max_depth;
    std::vector<int> parent_map;
    std::vector<std::vector<int>> pruning_generations;
    std::set<float> framework_circles;
    std::vector<Wedge> framework_wedges;
};