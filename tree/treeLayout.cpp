#include "include/treeLayout.h"
#include <numeric>
#include <algorithm>
#include <cmath>
#include <iostream>

TreeLayout::TreeLayout(const Tree &tree) : tree_ref(tree), DELTA(50.0f)
{
    int n = tree.getNumVertices();
    target_positions.resize(n, {400, 300});
    widths.resize(n, 0);
    depths.resize(n, 0);
    parent_map.resize(n, -1);
    max_depth = 0;
}

// cleans up everything
void TreeLayout::resetLayoutState()
{
    int n = tree_ref.getNumVertices();
    parent_map.assign(n, -1);
    max_depth = 0;
    framework_circles.clear();
    framework_wedges.clear();
    pruning_generations.clear();
}

void TreeLayout::finalizeLayout()
{
    max_depth = 0;
    for (int d : depths)
    {
        if (d > max_depth)
            max_depth = d;
    }
    // shifts the whole tree to the center of the screen
    for (size_t i = 0; i < target_positions.size(); ++i)
    {
        target_positions[i].x += 400;
        target_positions[i].y += 300;
    }
}

void TreeLayout::calculateTrueCenterLayout()
{
    resetLayoutState();
    findCenter();
    true_center_nodes = center_nodes;
    if (center_nodes.size() == 1)
    {
        int root = center_nodes[0];
        computeWidthsAndDepths(root, -1, 0);
        target_positions[root] = {0.0f, 0.0f};
        layoutSubTree(root, -1, 0.0f, 2 * M_PI);
    }
    else if (center_nodes.size() == 2)
    {
        int u = center_nodes[0];
        int v = center_nodes[1];
        target_positions[u] = {-DELTA / 2.0f, 0.0f};
        target_positions[v] = {DELTA / 2.0f, 0.0f};
        parent_map[v] = u;
        parent_map[u] = v;
        computeWidthsAndDepths(u, v, 0);
        computeWidthsAndDepths(v, u, 0);
        layoutSubTree(u, v, 3 * M_PI / 2.0, M_PI / 2.0);
        layoutSubTree(v, u, M_PI / 2.0, -M_PI / 2.0);
    }
    finalizeLayout();
}

// lays out the tree starting from any node user chooses
void TreeLayout::calculateLayoutFromRoot(int rootID)
{
    resetLayoutState();

    // force center node
    center_nodes.clear();
    center_nodes.push_back(rootID);
    true_center_nodes = center_nodes;

    computeWidthsAndDepths(rootID, -1, 0);
    target_positions[rootID] = {0.0f, 0.0f};
    layoutSubTree(rootID, -1, 0.0f, 2 * M_PI);
    finalizeLayout();
}

void TreeLayout::setDelta(float newDelta) { DELTA = newDelta; }
float TreeLayout::getDelta() const { return DELTA; }
const std::vector<Point> &TreeLayout::getTargetPositions() const { return target_positions; }
const std::vector<int> &TreeLayout::getCenterNodes() const { return true_center_nodes; }
const std::vector<int> &TreeLayout::getDepths() const { return depths; }
const std::vector<int> &TreeLayout::getWidths() const { return widths; }
int TreeLayout::getMaxDepth() const { return max_depth; }
const std::vector<int> &TreeLayout::getParentMap() const { return parent_map; }
const std::vector<std::vector<int>> &TreeLayout::getPruningGenerations() const { return pruning_generations; }
const std::set<float> &TreeLayout::getFrameworkCircles() const { return framework_circles; }
const std::vector<Wedge> &TreeLayout::getFrameworkWedges() const { return framework_wedges; }

// gets all the steps for the leaf pruning animation
void TreeLayout::prepareFindCenterAnimation()
{
    pruning_generations.clear();
    int n = tree_ref.getNumVertices();
    std::vector<int> degree(n);
    std::vector<int> q;
    int remaining_nodes = n;
    if (n <= 2)
        return;
    for (int i = 0; i < n; ++i)
    {
        degree[i] = tree_ref.getNeighbors(i).size();
        if (degree[i] == 1)
            q.push_back(i);
    }
    while (remaining_nodes > 2)
    {
        if (q.empty())
            break;
        pruning_generations.push_back(q);
        int q_size = q.size();
        remaining_nodes -= q_size;
        std::vector<int> next_q;
        for (int u : q)
        {
            for (int v : tree_ref.getNeighbors(u))
            {
                degree[v]--;
                if (degree[v] == 1)
                    next_q.push_back(v);
            }
        }
        q = next_q;
    }
    pruning_generations.push_back(q);
}

// finds the center of the tree by trimming leaves layer by layer
void TreeLayout::findCenter()
{
    center_nodes.clear();
    int n = tree_ref.getNumVertices();
    std::vector<int> degree(n);
    std::vector<int> q;
    int remaining_nodes = n;

    // if the tree is tiny, all nodes are centers
    if (n <= 2)
    {
        for (int i = 0; i < n; ++i)
            center_nodes.push_back(i);
        return;
    }
    for (int i = 0; i < n; ++i)
    {
        degree[i] = tree_ref.getNeighbors(i).size();
        if (degree[i] == 1)
            q.push_back(i);
    }
    while (remaining_nodes > 2)
    {
        int q_size = q.size();
        if (q_size == 0)
            break;
        remaining_nodes -= q_size;
        std::vector<int> next_q;
        for (int u : q)
        {
            for (int v : tree_ref.getNeighbors(u))
            {
                degree[v]--;
                if (degree[v] == 1)
                    next_q.push_back(v);
            }
        }
        q = next_q;
    }
    center_nodes = q;
}

void TreeLayout::computeWidthsAndDepths(int u, int p, int d)
{
    parent_map[u] = p;
    depths[u] = d;
    bool is_leaf = true;
    int leaf_count = 0;
    for (int v : tree_ref.getNeighbors(u))
    {
        if (v != p)
        {
            is_leaf = false;
            computeWidthsAndDepths(v, u, d + 1);
            leaf_count += widths[v];
        }
    }
    // width of a node is how many leaves are in its subtree
    widths[u] = is_leaf ? 1 : leaf_count;
}

void TreeLayout::layoutSubTree(int u, int p, float alpha1, float alpha2)
{
    float current_radius;
    // radius depends on whether we have one or two centers
    if (center_nodes.size() == 1)
    {
        current_radius = static_cast<float>(depths[u]) * DELTA;
    }
    else
    {
        current_radius = (static_cast<float>(depths[u]) + 0.5f) * DELTA;
    }

    // places the current node based on its parent position and angle
    if (p != -1)
    {
        float angle = (alpha1 + alpha2) / 2.0f;
        Point parentPos = target_positions[p];
        target_positions[u].x = parentPos.x + static_cast<float>(DELTA * cosf(angle));
        target_positions[u].y = parentPos.y + static_cast<float>(DELTA * sinf(angle));
    }

    // store the circle for drawing the layout framework
    float layout_radius = sqrt(target_positions[u].x * target_positions[u].x + target_positions[u].y * target_positions[u].y);
    framework_circles.insert(layout_radius + DELTA);

    // find the angular wedge this node has for its children
    float tau_rho = 0.0f;
    if (layout_radius + DELTA > 0)
    {
        float acos_arg = std::min(1.0f, layout_radius / (layout_radius + DELTA));
        tau_rho = 2.0f * acosf(acos_arg);
    }
    float total_angle = std::abs(alpha2 - alpha1);
    float angle_center = atan2(target_positions[u].y, target_positions[u].x);

    float effective_angle = (total_angle < 2 * M_PI && tau_rho < total_angle) ? tau_rho : total_angle;
    float start_alpha = angle_center - (effective_angle / 2.0f);

    float current_alpha = start_alpha;
    // split the parent's wedge among the children based on their size
    for (int v : tree_ref.getNeighbors(u))
    {
        if (v != p)
        {
            if (widths[u] == 0)
                continue;
            float wedge_angle = (static_cast<float>(widths[v]) / widths[u]) * effective_angle;

            // store the wedge for drawing the layout framework
            Point parent_pos_local = target_positions[u];
            framework_wedges.push_back({parent_pos_local, layout_radius + DELTA, current_alpha, current_alpha + wedge_angle});

            layoutSubTree(v, u, current_alpha, current_alpha + wedge_angle);
            current_alpha += wedge_angle;
        }
    }
}