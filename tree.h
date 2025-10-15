#pragma once

#include "graphics.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <map>
#include <string>
#include <GL/glut.h>
#include <set>

#define M_PI 3.14159265358979323846

struct Color
{
    float r;
    float g;
    float b;
};

struct Wedge
{
    Point center;
    float radius;
    float start_angle;
    float end_angle;
};

enum class DrawState
{
    NORMAL,
    ANIMATING_FIND_CENTER
};

class R1Tree
{
public:
    R1Tree(int n) : num_vertices(n), adj(n), current_positions(n, {400, 300}), target_positions(n), widths(n, 0), depths(n, 0), max_depth(0), parent_map(n, -1), DELTA(50.0f) {}

    void addEdge(int u, int v)
    {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    const int getNumVertices() const { return num_vertices; }
    const std::vector<Point> &getCurrentPositions() const { return current_positions; }
    const std::vector<Point> &getTargetPositions() const { return target_positions; }
    const std::vector<int> &getDepths() const { return depths; }
    const std::vector<int> &getWidths() const { return widths; }
    const std::vector<std::vector<int>> &getPruningGenerations() const { return pruning_generations; }
    const std::vector<int> &getCenterNodes() const { return true_center_nodes; }
    float getDelta() const { return DELTA; }

    void setDelta(float newDelta)
    {
        DELTA = newDelta;
        if (center_nodes.size() == 1)
        {
            calculateLayoutFromRoot(center_nodes[0]);
        }
        else
        {
            calculateTrueCenterLayout();
        }
    }

    const std::vector<std::pair<int, int>> getEdges() const
    {
        std::vector<std::pair<int, int>> edges;
        for (int u = 0; u < num_vertices; u++)
        {
            for (int v : adj[u])
            {
                if (u < v)
                {
                    edges.emplace_back(u, v);
                }
            }
        }
        return edges;
    }

    void calculateTrueCenterLayout()
    {
        resetLayoutState();
        findCenter();
        true_center_nodes = center_nodes;
        if (center_nodes.size() == 1)
        {
            int root = center_nodes[0];
            computeWidthsAndDepths(root, -1, 0);
            target_positions[root] = {0.0f, 0.0f};
            drawSubTree(root, -1, 0.0f, 2 * M_PI);
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

            drawSubTree(u, v, 3 * M_PI / 2.0, M_PI / 2.0);
            drawSubTree(v, u, M_PI / 2.0, -M_PI / 2.0);
        }
        finalizeLayout();
    }

    void calculateLayoutFromRoot(int rootID)
    {
        resetLayoutState();
        center_nodes.clear();
        center_nodes.push_back(rootID);
        true_center_nodes = center_nodes;
        computeWidthsAndDepths(rootID, -1, 0);

        target_positions[rootID] = {0.0f, 0.0f};
        drawSubTree(rootID, -1, 0.0f, 2 * M_PI);
        finalizeLayout();
    }

    void prepareFindCenterAnimation()
    {
        pruning_generations.clear();
        std::vector<int> degree(num_vertices);
        std::vector<int> q;
        int remaining_nodes = num_vertices;
        if (num_vertices <= 2)
            return;
        for (int i = 0; i < num_vertices; ++i)
        {
            degree[i] = adj[i].size();
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
                for (int v : adj[u])
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

    void draw(int hoveredNodeID, bool showFramework, DrawState state, int animationStep)
    {
        if (showFramework)
        {
            Point screenCenter = {400.0f, 300.0f};
            for (float radius : framework_circles)
            {
                glColor3f(0.2f, 0.2f, 0.3f);
                Drawing::drawCircleOutline(screenCenter, radius);
            }
            for (const auto &wedge : framework_wedges)
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

        if (state == DrawState::ANIMATING_FIND_CENTER)
        {
            std::set<int> pruned_nodes;
            for (int i = 0; i < animationStep && i < pruning_generations.size(); ++i)
            {
                for (int node_id : pruning_generations[i])
                {
                    pruned_nodes.insert(node_id);
                }
            }

            for (int u = 0; u < num_vertices; ++u)
                for (int v : adj[u])
                    if (u < v)
                    {
                        glColor3f(0.6f, 0.6f, 0.6f);
                        Drawing::drawLine(current_positions[u], current_positions[v]);
                    }

            for (int i = 0; i < num_vertices; ++i)
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
            if (animationStep >= pruning_generations.size())
            {
                if (!pruning_generations.empty())
                {
                    for (int center_id : pruning_generations.back())
                    {
                        glColor3f(1.0f, 1.0f, 1.0f);
                        Drawing::drawFilledCircle(current_positions[center_id], 9);
                    }
                }
            }
        }
        else
        {
            for (int u = 0; u < num_vertices; ++u)
                for (int v : adj[u])
                    if (u < v)
                    {
                        glColor3f(0.6f, 0.6f, 0.6f);
                        Drawing::drawLine(current_positions[u], current_positions[v]);
                    }

            for (int i = 0; i < num_vertices; ++i)
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
                    glColor3f(1.0f, 1.0f, 1.0f);
                    Drawing::drawFilledCircle(current_positions[i], 9);
                }
                else
                {
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
            if (hoveredNodeID != -1)
            {
                highlightSubtree(hoveredNodeID);
            }
        }
    }

    std::vector<Point> current_positions;

    static R1Tree *generateRandomTree(int n)
    {
        std::cout << "Generating random tree with " << n << " nodes:" << std::endl;
        srand(static_cast<unsigned int>(time(0)));
        R1Tree *tree = new R1Tree(n);

        std::vector<int> prufer(n - 2);
        for (int i = 0; i < n - 2; i++)
        {
            prufer[i] = 1 + (rand() % n);
        }

        std::vector<int> nodes(n, 0);
        for (int p : prufer)
        {
            nodes[p - 1]++;
        }

        int j = 0;
        for (int i = 0; i < n - 2; i++)
        {
            for (j = 0; j < n; j++)
            {
                if (nodes[j] == 0)
                {
                    nodes[j] = -1;
                    std::cout << j << " " << prufer[i] - 1 << std::endl;
                    tree->addEdge(j, prufer[i] - 1);
                    nodes[prufer[i] - 1]--;
                    break;
                }
            }
        }
        int k = 0;
        j = 0;

        for (int i = 0; i < n; i++)
        {
            if (nodes[i] == 0)
            {
                if (k == 0)
                {
                    j = i;
                    k++;
                }
                else if (k == 1)
                {
                    std::cout << j << " " << i << std::endl;
                    tree->addEdge(j, i);
                    break;
                }
            }
        }
        std::cout << std::endl;
        return tree;
    }

    static R1Tree *loadTreeFromFile(const std::string &filename)
    {
        std::ifstream infile(filename);
        if (!infile)
        {
            std::cerr << "Error opening file: " << filename << std::endl;
            return nullptr;
        }
        int n;
        infile >> n;
        R1Tree *tree = new R1Tree(n);
        int u, v;
        while (infile >> u >> v)
        {
            tree->addEdge(u, v);
        }
        infile.close();
        return tree;
    }

private:
    float DELTA;
    int num_vertices;
    std::vector<std::vector<int>> adj;
    std::vector<Point> target_positions;
    std::vector<int> center_nodes, true_center_nodes;
    std::vector<int> widths, depths;
    int max_depth;
    std::vector<int> parent_map;
    std::vector<std::vector<int>> pruning_generations;
    std::set<float> framework_circles;
    std::vector<Wedge> framework_wedges;

    void resetLayoutState()
    {
        std::fill(parent_map.begin(), parent_map.end(), -1);
        max_depth = 0;
        framework_circles.clear();
        framework_wedges.clear();
    }

    void finalizeLayout()
    {
        for (int d : depths)
        {
            if (d > max_depth)
                max_depth = d;
        }
        for (int i = 0; i < num_vertices; ++i)
        {
            target_positions[i].x += 400;
            target_positions[i].y += 300;
        }
    }

    void highlightSubtree(int u)
    {
        glColor3f(1.0f, 0.5f, 0.0f);
        Drawing::drawFilledCircle(current_positions[u], 8);
        for (int v : adj[u])
        {
            if (parent_map[v] == u)
            {
                if (parent_map[u] == v)
                {
                    continue;
                }
                glColor3f(1.0f, 0.5f, 0.0f);
                Drawing::drawLine(current_positions[u], current_positions[v]);
                highlightSubtree(v);
            }
        }
    }

    void findCenter()
    {
        center_nodes.clear();
        std::vector<int> degree(num_vertices);
        std::vector<int> q;
        int remaining_nodes = num_vertices;

        if (num_vertices <= 2)
        {
            for (int i = 0; i < num_vertices; ++i)
                center_nodes.push_back(i);
            return;
        }

        for (int i = 0; i < num_vertices; ++i)
        {
            degree[i] = adj[i].size();
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
                for (int v : adj[u])
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

    void computeWidthsAndDepths(int u, int p, int d)
    {
        parent_map[u] = p;
        depths[u] = d;
        bool is_leaf = true;
        int leaf_count = 0;
        for (int v : adj[u])
        {
            if (v != p)
            {
                is_leaf = false;
                computeWidthsAndDepths(v, u, d + 1);
                leaf_count += widths[v];
            }
        }
        widths[u] = is_leaf ? 1 : leaf_count;
    }

    void drawSubTree(int u, int p, float alpha1, float alpha2)
    {
        float current_radius;
        if (center_nodes.size() == 1)
        {
            current_radius = static_cast<float>(depths[u] + 1.0f) * DELTA;
        }
        else
        {
            current_radius = (static_cast<float>(depths[u]) + 0.5f) * DELTA;
        }

        if (p != -1)
        {
            float angle = (alpha1 + alpha2) / 2.0f;
            target_positions[u] = {
                static_cast<float>(current_radius * cosf(angle)),
                static_cast<float>(current_radius * sinf(angle))};
        }

        framework_circles.insert(current_radius);

        float tau_rho = 0.0f;
        if (current_radius + DELTA > 0)
        {
            float acos_arg = std::min(1.0f, current_radius / (current_radius + DELTA));
            tau_rho = 2.0f * acosf(acos_arg);
        }

        float total_angle;
        float start_alpha;
        float angle_center = (alpha1 + alpha2) / 2.0f;

        if (alpha2 - alpha1 < 2 * M_PI && tau_rho < (alpha2 - alpha1))
        {
            total_angle = tau_rho;
            start_alpha = angle_center - (tau_rho / 2.0f);
        }
        else
        {
            total_angle = alpha2 - alpha1;
            start_alpha = alpha1;
        }

        float current_alpha = start_alpha;
        for (int v : adj[u])
        {
            if (v != p)
            {
                if (widths[u] == 0)
                    continue;
                float wedge_angle = (static_cast<float>(widths[v]) / widths[u]) * total_angle;

                Point parent_pos_local = target_positions[u];
                framework_wedges.push_back({parent_pos_local, current_radius + DELTA, current_alpha, current_alpha + wedge_angle});

                drawSubTree(v, u, current_alpha, current_alpha + wedge_angle);
                current_alpha += wedge_angle;
            }
        }
    }
};