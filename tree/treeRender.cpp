#include "include/treeRenderer.h"
#include <set>
#include <algorithm>

TreeRenderer::TreeRenderer(const Tree &tree, const TreeLayout &layout)
    : tree_ref(tree), layout_ref(layout) {}

void TreeRenderer::drawFramework(const std::vector<Point> &positions)
{
    Point screenCenter = {400.0f, 300.0f};
    // first draw all the concentric circles
    for (float radius : layout_ref.getFrameworkCircles())
    {
        glColor3f(0.2f, 0.2f, 0.3f);
        Drawing::drawCircleOutline(screenCenter, radius);
    }
    // then draw the wedge lines
    for (const auto &wedge : layout_ref.getFrameworkWedges())
    {
        Point start = {
            screenCenter.x + (wedge.radius * cosf(wedge.start_angle)),
            screenCenter.y + (wedge.radius * sinf(wedge.start_angle))};
        Point end = {
            screenCenter.x + (wedge.radius * cosf(wedge.end_angle)),
            screenCenter.y + (wedge.radius * sinf(wedge.end_angle))};
        Point center = {screenCenter.x + wedge.center.x, screenCenter.y + wedge.center.y};
        glColor3f(0.2f, 0.4f, 0.4f);
        Drawing::drawLine(center, start);
        Drawing::drawLine(center, end);
    }
}

void TreeRenderer::highlightSubtree(int u, const std::vector<Point> &positions)
{
    glColor3f(1.0f, 0.5f, 0.0f);
    Drawing::drawFilledCircle(positions[u], 8);
    const auto &parent_map = layout_ref.getParentMap();

    for (int v : tree_ref.getNeighbors(u))
    {
        //  check to avoid getting stuck in a loop with two center nodes
        if (parent_map.at(v) == u && parent_map.at(u) != v)
        {
            glColor3f(1.0f, 0.5f, 0.0f);
            Drawing::drawLine(positions[u], positions[v]);
            highlightSubtree(v, positions);
        }
    }
}

void TreeRenderer::draw(const std::vector<Point> &current_positions, int hoveredNodeID, bool showFramework, DrawState state, int animationStep)
{
    if (showFramework)
    {
        drawFramework(current_positions);
    }

    if (state == DrawState::ANIMATING_FIND_CENTER)
    {
        const auto &pruning_generations = layout_ref.getPruningGenerations();
        std::set<int> pruned_nodes;
        // figure out which nodes have been pruned so far based
        for (int i = 0; i < animationStep && i < pruning_generations.size(); ++i)
        {
            for (int node_id : pruning_generations[i])
            {
                pruned_nodes.insert(node_id);
            }
        }

        // draw all the edges first
        for (const auto &edge : tree_ref.getEdges())
        {
            glColor3f(0.6f, 0.6f, 0.6f);
            Drawing::drawLine(current_positions[edge.first], current_positions[edge.second]);
        }

        // draw the nodes, graying out the pruned ones
        for (int i = 0; i < tree_ref.getNumVertices(); ++i)
        {
            if (pruned_nodes.count(i))
            {
                glColor4f(0.5f, 0.5f, 0.5f, 0.2f);
            }
            else
            {
                glColor3f(1.0f, 1.0f, 0.0f);
            }
            Drawing::drawFilledCircle(current_positions[i], 6);
        }
        // highlight the final center nodes
        if (animationStep >= pruning_generations.size() && !pruning_generations.empty())
        {
            for (int center_id : pruning_generations.back())
            {
                glColor3f(1.0f, 1.0f, 1.0f);
                Drawing::drawFilledCircle(current_positions[center_id], 9);
            }
        }
    }
    else
    {
        for (const auto &edge : tree_ref.getEdges())
        {
            glColor3f(0.6f, 0.6f, 0.6f);
            Drawing::drawLine(current_positions[edge.first], current_positions[edge.second]);
        }

        const auto &depths = layout_ref.getDepths();
        const auto &true_center_nodes = layout_ref.getCenterNodes();
        int max_depth = layout_ref.getMaxDepth();

        for (int i = 0; i < tree_ref.getNumVertices(); ++i)
        {
            bool isTrueCentralNode = false;
            for (int central_id : true_center_nodes)
            {
                if (i == central_id)
                {
                    isTrueCentralNode = true;
                    break;
                }
            }
            if (isTrueCentralNode)
            {
                // highlight the true center nodes in white
                glColor3f(1.0f, 1.0f, 1.0f);
                Drawing::drawFilledCircle(current_positions[i], 9);
            }
            else
            {
                // we color the nodes based on their depth, making a nice gradient
                float t = (max_depth > 1) ? static_cast<float>(std::max(0, depths[i] - 1)) / (max_depth - 1) : (depths[i] > 0 ? 1.0f : 0.0f);
                Color start_color = {1.0f, 1.0f, 0.0f}, end_color = {0.1f, 0.4f, 1.0f};
                Color node_color;
                node_color.r = start_color.r * (1.0f - t) + end_color.r * t;
                node_color.g = start_color.g * (1.0f - t) + end_color.g * t;
                node_color.b = start_color.b * (1.0f - t) + end_color.b * t;
                glColor3f(node_color.r, node_color.g, node_color.b);
                Drawing::drawFilledCircle(current_positions[i], 7);
            }
        }
        // highlight hovered node's subtree
        if (hoveredNodeID != -1)
        {
            highlightSubtree(hoveredNodeID, current_positions);
        }
    }
}