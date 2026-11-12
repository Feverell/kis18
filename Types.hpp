#pragma once

namespace OldMap {

using NodeId = int;
using EdgeId = int;
using Currency = double;
using Probability = double;

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
    Probability probability;
    EdgeState state;
};

struct Node {
    NodeId id;
    Currency profit;
    bool is_visited;
};

}
