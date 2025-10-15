#pragma once

#include <vector>

#include <string>

#include "graphics.h"

class Tree

{

public:
    Tree(int num_vertices);

    void addEdge(int u, int v);

    int getNumVertices() const;

    const std::vector<std::vector<int>> &getAdjacencyList() const;

    const std::vector<int> &getNeighbors(int u) const;

    std::vector<std::pair<int, int>> getEdges() const;

    static Tree loadFromFile(const std::string &filename);

    static Tree generateRandom(int n);

private:
    int num_vertices;

    std::vector<std::vector<int>> adj;
};