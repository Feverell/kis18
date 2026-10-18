#pragma once

using NodeId = int;
using EdgeId = int;

constexpr NodeId INVALID_NODE_ID = -1;
constexpr EdgeId INVALID_EDGE_ID = -1;

enum class EdgeState {
    UNKNOWN,
    OPEN,
    BROKEN
};

struct Edge {
    EdgeId id;
    NodeId u;
    NodeId v;
    double probability;
    EdgeState state;
};

struct Node {
    NodeId id;
    double profit;
    bool is_visited;
};
