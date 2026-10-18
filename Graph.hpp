#pragma once

#include "Types.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>

class Graph {
public:
    Graph() = default;

    // Добавление вершины. Если вершина уже есть, просто возвращаем ее ID
    NodeId addNode(const std::string& name, double profit) {
        if (auto it = node_name_to_id_.find(name); it != node_name_to_id_.end()) {
            return it->second;
        }
        NodeId id = static_cast<NodeId>(nodes_.size());
        nodes_.push_back({id, profit, false});
        adj_list_.emplace_back();
        
        node_name_to_id_[name] = id;
        node_names_.push_back(name);
        return id;
    }

    // Добавление двустороннего ребра
    EdgeId addEdge(const std::string& name, const std::string& u_name, const std::string& v_name, double probability) {
        NodeId u = getNodeId(u_name);
        NodeId v = getNodeId(v_name);
        
        EdgeId id = static_cast<EdgeId>(edges_.size());
        edges_.push_back({id, u, v, probability, EdgeState::UNKNOWN});
        
        // Граф неориентированный, добавляем в списки смежности обеих вершин
        adj_list_[u].push_back(id);
        adj_list_[v].push_back(id);
        
        edge_name_to_id_[name] = id;
        edge_names_.push_back(name);
        return id;
    }

    NodeId getNodeId(const std::string& name) const {
        if (auto it = node_name_to_id_.find(name); it != node_name_to_id_.end()) {
            return it->second;
        }
        throw std::runtime_error("Unknown node name: " + name);
    }

    EdgeId getEdgeId(const std::string& name) const {
        if (auto it = edge_name_to_id_.find(name); it != edge_name_to_id_.end()) {
            return it->second;
        }
        throw std::runtime_error("Unknown edge name: " + name);
    }

    const std::string& getNodeName(NodeId id) const { return node_names_[id]; }
    const std::string& getEdgeName(EdgeId id) const { return edge_names_[id]; }

    Node& getNode(NodeId id) { return nodes_[id]; }
    const Node& getNode(NodeId id) const { return nodes_[id]; }
    
    Edge& getEdge(EdgeId id) { return edges_[id]; }
    const Edge& getEdge(EdgeId id) const { return edges_[id]; }

    const std::vector<EdgeId>& getIncidentEdges(NodeId id) const { return adj_list_[id]; }
    
    NodeId getOppositeNode(EdgeId edge_id, NodeId node_id) const {
        const Edge& edge = edges_[edge_id];
        return (edge.u == node_id) ? edge.v : edge.u;
    }

    void markNodeVisited(NodeId id) { nodes_[id].is_visited = true; }
    void updateEdgeState(EdgeId id, EdgeState state) { edges_[id].state = state; }

    size_t nodesCount() const { return nodes_.size(); }
    size_t edgesCount() const { return edges_.size(); }

private:
    std::vector<Node> nodes_;
    std::vector<Edge> edges_;
    std::vector<std::vector<EdgeId>> adj_list_; // adj_list_[u] = массив ID ребер

    // Словари для маппинга "строка <-> ID"
    std::unordered_map<std::string, NodeId> node_name_to_id_;
    std::unordered_map<std::string, EdgeId> edge_name_to_id_;
    
    // Обратный маппинг "ID -> строка" для команд LOOK, CLEAR, MOVE
    std::vector<std::string> node_names_;
    std::vector<std::string> edge_names_;
};
