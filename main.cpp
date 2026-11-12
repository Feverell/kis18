#include "Types.hpp"
#include "Graph.hpp"
#include "Solver.hpp"
#include "Agent.hpp"

#include <iostream>
#include <string>

int main() {

    OldMap::Graph graph;
    OldMap::Currency day_cost = 0.0;
    std::string entrance_name;
    std::string token;



    // Парсим карту
    while (std::cin >> token) {
        if (token == "MAP") {
            continue;
        } else if (token == "day_cost") {
            std::cin >> day_cost;
        } else if (token == "entrance") {
            std::cin >> entrance_name;
        } else if (token == "vertex") {
            std::string name;
            OldMap::Currency profit;
            std::cin >> name >> profit;
            graph.addNode(name, profit);
        } else if (token == "edge") {
            std::string name, u_name, v_name;
            OldMap::Probability prob;
            std::cin >> name >> u_name >> v_name >> prob;
            graph.addEdge(name, u_name, v_name, prob);
        } else if (token == "END") {
            break;
        }
    }
    if (entrance_name.empty() || graph.nodesCount() == 0) {
        std::cerr << "Error: Invalid or empty map description.\n";
        return 1;
    }

    OldMap::NodeId start_node = graph.getNodeId(entrance_name);
    
    OldMap::Solver solver(day_cost);
    OldMap::Agent agent(graph, solver, start_node);
    
    agent.run();

    return 0;
}
