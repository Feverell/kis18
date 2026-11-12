#pragma once

#include "Graph.hpp"

#include <optional>
#include <vector>
#include <queue>
#include <limits>

namespace OldMap {

class Solver {
public:
    explicit Solver(Currency day_cost) : day_cost_(day_cost) {}

    // Возвращает ID закрытого (BROKEN) ребра, которое выгоднее всего расчистить.
    // Если выгодных путей больше нет, возвращает std::nullopt.
    std::optional<EdgeId> getBestEdgeToClear(const Graph& graph, NodeId root) const {
        size_t n = graph.nodesCount();
        if (n == 0) return std::nullopt;

        // ====================================================================
        // ШАГ 1: Строим дерево кратчайших путей (минимизируем число CLEAR)
        // ====================================================================
        std::vector<Currency> dist(n, std::numeric_limits<Currency>::infinity());
        std::vector<EdgeId> parent_edge(n, INVALID_EDGE_ID);
        
        // Min-Heap для Дейкстры. Хранит пары {расстояние, ID вершины}
        using State = std::pair<Currency, NodeId>;
        std::priority_queue<State, std::vector<State>, std::greater<State>> pq;

        dist[root] = 0.0;
        pq.push({0.0, root});

        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();

            if (d > dist[u]) continue; // Пропускаем устаревшие записи

            for (EdgeId edge_id : graph.getIncidentEdges(u)) {
                const Edge& edge = graph.getEdge(edge_id);
                NodeId v = (edge.u == u) ? edge.v : edge.u;

                // Оцениваем "стоимость" прохода в виде ожидаемого числа CLEAR
                Currency weight = 0.0;
                if (edge.state == EdgeState::OPEN) {
                    weight = 0.0;
                } else if (edge.state == EdgeState::BROKEN) {
                    weight = 1.0;
                } else if (edge.state == EdgeState::UNKNOWN) {
                    weight = 1.0 - edge.probability;
                }

                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    parent_edge[v] = edge_id;
                    pq.push({dist[v], v});
                }
            }
        }

        // ====================================================================
        // ШАГ 2: Собираем древовидную структуру (списки детей) для DP
        // ====================================================================
        struct Child {
            NodeId node;
            EdgeId edge;
        };
        std::vector<std::vector<Child>> tree_children(n);
        
        for (NodeId v = 0; v < static_cast<NodeId>(n); ++v) {
            if (v != root && parent_edge[v] != INVALID_EDGE_ID) {
                NodeId u = graph.getOppositeNode(parent_edge[v], v); // Находим родителя
                tree_children[u].push_back({v, parent_edge[v]});
            }
        }

        // ====================================================================
        // ШАГ 3: Топологическая сортировка дерева (DFS) для обхода от листьев
        // ====================================================================
        std::vector<NodeId> topo;
        topo.reserve(n);
        std::vector<bool> visited_dfs(n, false);
        
        // Лямбда-функция для рекурсивного обхода (DFS)
        auto dfs = [&](auto& self, NodeId u) -> void {
            visited_dfs[u] = true;
            for (const auto& child : tree_children[u]) {
                if (!visited_dfs[child.node]) {
                    self(self, child.node);
                }
            }
            // Постфиксный порядок: сначала дети, потом родитель
            topo.push_back(u);
        };
        
        dfs(dfs, root);

        // ====================================================================
        // ШАГ 4: Динамическое программирование (оценка ожидаемого профита)
        // ====================================================================
        std::vector<Currency> dp(n, 0.0);
        
        for (NodeId u : topo) {
            const Node& node = graph.getNode(u);
            
            // Если мы уже были в этой комнате, лут собран (0). Иначе учитываем его.
            dp[u] = node.is_visited ? 0.0 : node.profit;
            
            for (const auto& child : tree_children[u]) {
                NodeId v = child.node;
                const Edge& e = graph.getEdge(child.edge);
                
                Currency child_val = dp[v];
                
                // Если поддерево в принципе не сулит выгоды, мы туда просто не пойдем
                if (child_val <= 0.0) continue;
                
                if (e.state == EdgeState::OPEN) {
                    // Путь открыт, забираем профит поддерева бесплатно
                    dp[u] += child_val;
                    
                } else if (e.state == EdgeState::BROKEN) {
                    // Путь завален, придется тратить день на расчистку
                    dp[u] += std::max(0.0, child_val - day_cost_);
                    
                } else if (e.state == EdgeState::UNKNOWN) {
                    // Мы не знаем состояние. 
                    // Если он цел (prob), мы получаем child_val.
                    // Если он сломан (1 - prob), мы решаем, окупается ли расчистка.
                    Currency expected = e.probability * child_val + 
                                      (1.0 - e.probability) * std::max(0.0, child_val - day_cost_);
                    dp[u] += expected;
                }
            }
        }

        // ====================================================================
        // ШАГ 5: Выбор лучшего ЗАВАЛЕННОГО ребра для расчистки
        // ====================================================================
        EdgeId best_edge = INVALID_EDGE_ID;
        Currency max_expected_profit = 0.0; // Ищем строго позитивный профит

        // Проходим по всем вершинам, чье родительское ребро сейчас сломано (BROKEN)
        for (NodeId v = 0; v < static_cast<NodeId>(n); ++v) {
            EdgeId e_id = parent_edge[v];
            if (e_id != INVALID_EDGE_ID) {
                const Edge& e = graph.getEdge(e_id);
                
                if (e.state == EdgeState::BROKEN) {
                    // Ожидаемый профит от расчистки этого конкретного ребра
                    Currency expected_profit = dp[v] - day_cost_;
                    
                    if (expected_profit > max_expected_profit) {
                        max_expected_profit = expected_profit;
                        best_edge = e_id;
                    }
                }
            }
        }

        if (best_edge != INVALID_EDGE_ID) {
            return best_edge;
        }
        
        // Ожидается, что профитных целей для расчистки больше нет
        return std::nullopt;
    }

private:
    Currency day_cost_;
};

}
