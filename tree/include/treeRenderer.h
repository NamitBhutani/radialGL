#pragma once
#include "tree.h"
#include "treeLayout.h"
#include "graphics.h"
#include <GL/glut.h>
#include <vector>

struct Color
{
    float r, g, b;
};

enum class DrawState
{
    NORMAL,
    ANIMATING_FIND_CENTER
};

class TreeRenderer
{
public:
    TreeRenderer(const Tree &tree, const TreeLayout &layout);

    void draw(const std::vector<Point> &current_positions, int hoveredNodeID, bool showFramework, DrawState state, int animationStep);

private:
    void drawFramework(const std::vector<Point> &positions);
    void highlightSubtree(int u, const std::vector<Point> &positions);

    const Tree &tree_ref;
    const TreeLayout &layout_ref;
};