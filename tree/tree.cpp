#include "include/tree.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <chrono>

Tree::Tree(int n) : num_vertices(n), adj(n) {}

void Tree::addEdge(int u, int v)
{
    // undirected graph
    adj[u].push_back(v);
    adj[v].push_back(u);
}

int Tree::getNumVertices() const { return num_vertices; }

const std::vector<std::vector<int>> &Tree::getAdjacencyList() const { return adj; }

const std::vector<int> &Tree::getNeighbors(int u) const { return adj[u]; }

std::vector<std::pair<int, int>> Tree::getEdges() const
{
    std::vector<std::pair<int, int>> edges;
    for (int u = 0; u < num_vertices; u++)
    {
        for (int v : adj[u])
        {
            // dont add the same edge twice
            if (u < v)
            {
                edges.emplace_back(u, v);
            }
        }
    }
    return edges;
}

// reads a tree structure from a text file
Tree Tree::loadFromFile(const std::string &filename)
{
    std::ifstream infile(filename);
    if (!infile)
    {
        std::cerr << "error opening file: " << filename << std::endl;
        return Tree(0);
    }

    // the file's first line should be the number of nodes
    int n;
    infile >> n;
    Tree tree(n);
    // then it just reads pairs of nodes to connect

    int u, v;
    while (infile >> u >> v)
    {
        tree.addEdge(u, v);
    }

    infile.close();
    return tree;
}

Tree Tree::generateRandom(int n)
{
    auto start = std::chrono::high_resolution_clock::now();
    // seed the random generator so we dont get the same tree every time
    srand(static_cast<unsigned int>(time(0)));
    Tree tree(n);
    if (n <= 1)
        return tree;

    // create a random prufer sequence
    std::vector<int> prufer(n - 2);
    for (int i = 0; i < n - 2; i++)
    {
        prufer[i] = rand() % n;
    }

    // now we figure out the degree of each node based on the sequence
    std::vector<int> degree(n, 1);
    for (int node : prufer)
    {
        degree[node]++;
    }

    // this loop connects the leaves to the nodes in the prufer sequence
    for (int p_node : prufer)
    {
        for (int leaf = 0; leaf < n; ++leaf)
        {
            if (degree[leaf] == 1)
            {
                tree.addEdge(p_node, leaf);
                degree[p_node]--;
                degree[leaf]--;
                break; // move to the next node in prufer seq
            }
        }
    }

    int u = -1;
    // finally, connect the last two nodes that are left
    for (int i = 0; i < n; ++i)
    {
        if (degree[i] == 1)
        {
            if (u == -1)
            {
                u = i;
            }
            else
            {
                tree.addEdge(u, i);
                break;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    double millis = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6;
    std::cout << "Random tree generation took " << millis << " ms" << std::endl;
    return tree;
}