#pragma once

#include "Graph.hpp"
#include "Solver.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <optional>

class Agent {
public:
    Agent(Graph& graph, Solver solver, NodeId entrance_node)
        : graph_(graph), solver_(std::move(solver)), 
          entrance_node_(entrance_node), current_node_(entrance_node) {}

    // Главный цикл управления
    void run() {
        while (true) {
            // 1. Если мы в новой вершине, осматриваемся и забираем лут
            if (!graph_.getNode(current_node_).is_visited) {
                lookAround();
            }

            // 2. Ищем ближайшую недоисследованную вершину, доступную по открытым (OPEN) путям
            std::optional<EdgeId> next_edge = getNextEdgeToUnvisited();

            if (next_edge.has_value()) {
                // 3. Если путь есть, идем по нему
                moveTo(next_edge.value());
            } else {
                // 4. Если свободных путей нет, время звать математику на помощь
                std::optional<EdgeId> best_edge_to_clear = solver_.getBestEdgeToClear(graph_, entrance_node_);

                if (best_edge_to_clear.has_value()) {
                    clearEdge(best_edge_to_clear.value());
                } else {
                    // Математика сказала, что больше выгоды не извлечь
                    endExpedition();
                    break;
                }
            }
        }
    }

private:
    Graph& graph_;
    Solver solver_;
    NodeId entrance_node_;
    NodeId current_node_;

    // --- Блок взаимодействия с судьей (I/O) ---

    void lookAround() {
        std::cout << "LOOK" << std::endl; // endl форсирует сброс буфера (flush), что критично для интерактива

        // Так как топология нам известна, мы точно знаем, сколько строк ответит судья
        size_t degree = graph_.getIncidentEdges(current_node_).size();
        
        for (size_t i = 0; i < degree; ++i) {
            std::string path_marker, edge_name, dest_name, status;
            std::cin >> path_marker >> edge_name >> dest_name >> status; // PATH х1 ЗАЛ-А OK

            EdgeId edge_id = graph_.getEdgeId(edge_name);
            if (status == "OK") {
                graph_.updateEdgeState(edge_id, EdgeState::OPEN);
            } else if (status == "BROKEN") {
                graph_.updateEdgeState(edge_id, EdgeState::BROKEN);
            }
        }
        
        // Отмечаем вершину как посещенную (лут собран, ходы осмотрены)
        graph_.markNodeVisited(current_node_);
    }

    void moveTo(EdgeId edge_id) {
        std::cout << "MOVE " << graph_.getEdgeName(edge_id) << std::endl;
        
        std::string status;
        std::cin >> status;
    }

    void clearEdge(EdgeId edge_id) {
        std::cout << "CLEAR " << graph_.getEdgeName(edge_id) << std::endl;
        
        std::string status;
        std::cin >> status; // Ожидаем OK
        
        // Судья расчистил путь и телепортировал нас на базу
        current_node_ = entrance_node_;
        
        // Теперь этот путь свободен, помечаем его как OPEN, чтобы BFS смог строить через него маршруты
        graph_.updateEdgeState(edge_id, EdgeState::OPEN);
    }

    void endExpedition() {
        std::cout << "END" << std::endl;
        
        // Читаем результат, чтобы оставить поток чистым
        std::string result_marker, collected, days, profit;
        std::cin >> result_marker >> collected >> days >> profit;
    }

    // --- Блок навигации ---

    // BFS поиск кратчайшего пути по гарантированно свободным (OPEN) ходам до ближайшей неисследованной вершины
    std::optional<EdgeId> getNextEdgeToUnvisited() {
        size_t n = graph_.nodesCount();
        std::vector<bool> visited_bfs(n, false);
        std::vector<EdgeId> edge_to_reach(n, INVALID_EDGE_ID);
        std::vector<NodeId> parent_node(n, INVALID_NODE_ID);
        
        std::queue<NodeId> q;
        
        q.push(current_node_);
        visited_bfs[current_node_] = true;
        
        NodeId target_node = INVALID_NODE_ID;
        
        // Запускаем поиск в ширину
        while (!q.empty()) {
            NodeId u = q.front();
            q.pop();
            
            // Если нашли непосещенную вершину (мы в ней не делали LOOK)
            if (!graph_.getNode(u).is_visited) {
                target_node = u;
                break; 
            }
            
            for (EdgeId edge_id : graph_.getIncidentEdges(u)) {
                const Edge& edge = graph_.getEdge(edge_id);
                
                // Перемещаемся только по 100% безопасным путям
                if (edge.state == EdgeState::OPEN) {
                    NodeId v = graph_.getOppositeNode(edge_id, u);
                    if (!visited_bfs[v]) {
                        visited_bfs[v] = true;
                        edge_to_reach[v] = edge_id;
                        parent_node[v] = u;
                        q.push(v);
                    }
                }
            }
        }
        
        // Если неисследованных достижимых вершин нет
        if (target_node == INVALID_NODE_ID) {
            return std::nullopt;
        }
        
        // Раскручиваем путь с конца, чтобы понять, какой первый шаг нужно сделать из current_node_
        NodeId curr = target_node;
        EdgeId first_edge = INVALID_EDGE_ID;
        
        while (curr != current_node_) {
            first_edge = edge_to_reach[curr];
            curr = parent_node[curr];
        }
        
        return first_edge;
    }
};
