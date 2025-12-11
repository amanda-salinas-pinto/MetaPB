#include "Graph.hpp"

namespace rs {
  Graph::Graph() {}
  
  void Graph::init(ConstraintAllocator& ca, std::vector<CRef>& constraints, int nvars) {
    n_var = nvars;
    nodes_amount = (n_var * 2) + constraints.size();
    n_constraints = constraints.size();
    int var_offset = constraints.size() - 1;
    int constraint_iter = 0;
    for (CRef cr: constraints) {
      const Constr& C = ca[cr];
      for (unsigned int var_iter = 0; var_iter < C.size(); ++var_iter) {
        Lit literal = C.lit(var_iter);
        int var_index = (literal > 0) ? literal: abs(literal) * 2;
        long double coef = static_cast<long double>(C.coef(var_iter));
        long double degree = static_cast<long double>(C.degree());
        adjacency_list[var_offset + var_index][constraint_iter] = coef/degree;
        adjacency_list[constraint_iter][var_offset + var_index] = coef/degree;
      }
      constraint_iter++;
    }
  }

  void Graph::calculatePseudoBooleanCentrality(std::vector<ActValV>& activity, ConstraintAllocator& ca, std::vector<CRef>& constraints, int nvars) {
    n_var = nvars;
    nodes_amount = (n_var * 2);
    std::vector<long double> ranks(nodes_amount, 0);
    for (CRef cr: constraints) {
      const Constr& C = ca[cr];
      long double degree = static_cast<long double>(C.degree());
      for (unsigned int var_iter = 0; var_iter < C.size(); ++var_iter) {
        Lit literal = C.lit(var_iter);
        int var_index = (literal > 0) ? literal: abs(literal) * 2;
        long double coef = static_cast<long double>(C.coef(var_iter));
        ranks[var_index] += (coef/degree) * (1.0 / C.size());
      }
    }
    for (int l = 0; l < n_var; l++) {
      activity[l + 1] = std::max(ranks[l], ranks[l + n_var]) * n_var * 2;
    }
  }

  void Graph::calculatePagerank(std::vector<ActValV>& activity) {
    long double damping_factor = 0.85;
    int max_iterations = 10;
    std::vector<long double> ranks(nodes_amount, 1.0 / nodes_amount);
    std::vector<long double> weights(nodes_amount);
    // normalization    
    for (const auto& [node, edges] : adjacency_list) {
        double sum = 0;
        for (const auto& [adj_node, adj_weight] : edges) {
            sum += 1;
        }
        weights[node] = sum;
    }
    // ranking
    long double damping_value = (1 - damping_factor) / nodes_amount;
    for (int iteration = 0; iteration < max_iterations; ++iteration) {
      std::vector<long double> new_ranks(nodes_amount, 0);
      for (const auto& [node, edges] : adjacency_list) {
        for (const auto& [adj_node, adj_weight] : edges) new_ranks[node] += (ranks[adj_node]* (adj_weight/weights[adj_node]));
        new_ranks[node] = damping_factor * new_ranks[node];
      }
      for(const auto &pair : adjacency_list) {
        new_ranks[pair.first] += damping_value;
      }
      ranks = new_ranks;
    }
    for (int l = 0; l < n_var; l++) {
      activity[l + 1] = std::max(ranks[l + n_constraints], ranks[l + n_constraints + n_var]) * n_var * 2;
    }
  }

  void Graph::calculateHITS(std::vector<ActValV>& activity) {
    /*
    * Uses authority as metric for variable picking.
    *
    */
    std::vector<long double> auth_index(nodes_amount, 1);
    std::vector<long double> hub_index(nodes_amount, 1);
    unsigned int max_iterations = 10;
    for (unsigned int iteration = 0; iteration < max_iterations; iteration++) {
      double norm = 0;
      for(const auto& [node, adjacent_nodes] : adjacency_list) {
        auth_index[node] = 0;
        for (const auto& [adjacent_node, edge_weight]: adjacent_nodes) {
            auth_index[node] += edge_weight * hub_index[adjacent_node];
        }
        norm += sqrt(auth_index[node]);
      }
      norm = sqrt(norm);
      for(const auto& [node, adjacent_nodes] : adjacency_list) {
        auth_index[node] /= norm;
      }
      norm = 0;
      for(const auto& [node, adjacent_nodes] : adjacency_list) {
        hub_index[node] = 0;
        for (const auto& [adjacent_node, edge_weight]: adjacent_nodes) {
            hub_index[node] += edge_weight * auth_index[adjacent_node];
        }
        norm += sqrt(hub_index[node]);
      }
      norm = sqrt(norm);
      for(const auto& [node, adjacent_nodes] : adjacency_list) {
        hub_index[node] /= norm;
      }
    }
    for (int l = 0; l < n_var; l++) {
      activity[l + 1] = std::max(auth_index[l + n_constraints], auth_index[l + n_constraints + n_var]) * n_var * 2;
    }
  }

  void Graph::calculateEigenvector(std::vector<ActValV>& activity) {
    /*
    * EigenVector centrality measure on variables
    */

    std::vector<long double> centrality(nodes_amount);
    unsigned int max_iterations = 10;

    for (unsigned int i = 0; i < max_iterations; i++) {
      long double sum = 0;
      std::vector<long double> new_centrality(nodes_amount, 0.0);
      for (const auto& [node, adjacent_nodes]: adjacency_list) {
        for(const auto& [adjacent_node, weight]: adjacent_nodes)
          new_centrality[node] += weight * centrality[adjacent_node];
        sum += new_centrality[node] * new_centrality[node];
      }
      long double norm = sqrt(sum);
      for (int i = 0; i < new_centrality.size(); i++) new_centrality[i] /= norm;
      centrality = new_centrality;
    }

    for (int l = 0; l < n_var; l++) {
      activity[l + 1] = std::max(centrality[l + n_constraints], centrality[l + n_constraints + n_var]) * n_var * 2;
    }
  }

  std::vector<std::vector<long double>> Graph::getShortestPaths() {
    /*
    * Floyd-Warshall algorithm for all-pairs shortest paths
    */
    std::cout << "c Floyd-Warshall" << std::endl;
    std::vector<std::vector<long double>> dist(nodes_amount, std::vector<long double>(nodes_amount, std::numeric_limits<long double>::max()));
    for (int i = 0; i < nodes_amount; i++) {
      dist[i][i] = 0;
    }
    for (const auto& [node, adjacent_nodes]: adjacency_list) {
      for (const auto& [adjacent_node, weight]: adjacent_nodes) {
        dist[node][adjacent_node] = weight;
      }
    }
    for (int k = 0; k < nodes_amount; k++) {
      std::cout << "c Floyd-Warshall iteration " << k << std::endl;
      for (int i = 0; i < nodes_amount; i++) {
        for (int j = 0; j < nodes_amount; j++) {
          if (dist[i][k] + dist[k][j] < dist[i][j]) {
            dist[i][j] = dist[i][k] + dist[k][j];
          }
        }
      }
    }
    return  dist;
  }

  void Graph::calculateClosenessCentrality(std::vector<ActValV>& activity) {
    /*
    * Closeness centrality measure on variables
    */
    std::vector<std::vector<long double>> dist = getShortestPaths();
    std::cout << "c Closeness centrality calculation" << std::endl;
    std::vector<long double> centrality(nodes_amount, 0);
    for (int i = n_constraints; i < nodes_amount; i++) {
      for (int j = 0; j < nodes_amount; j++) {
        if (i != j) {
          centrality[i] += dist[i][j];
        }
      }
    }
    for (int i = 0; i < nodes_amount; i++) {
      centrality[i] = (nodes_amount - 1) / centrality[i];
    }
    for (int l = 0; l < n_var; l++) {
      activity[l + 1] = std::max(centrality[l + n_constraints], centrality[l + n_constraints + n_var]) * n_var * 2;
    }
  }

  void Graph::calculateBetweennessCentrality(std::vector<ActValV>& activity) {
    /*
    * Betweenness centrality measure on variables
    */

    std::cout << "c Floyd-Warshall" << std::endl;
    std::vector<long double> centrality(nodes_amount, 1);
    std::vector<std::vector<long double>> dist(nodes_amount, std::vector<long double>(nodes_amount, std::numeric_limits<long double>::max()));
    for (int i = 0; i < nodes_amount; i++) {
      dist[i][i] = 0;
    }
    for (const auto& [node, adjacent_nodes]: adjacency_list) {
      for (const auto& [adjacent_node, weight]: adjacent_nodes) {
        dist[node][adjacent_node] = weight;
      }
    }
    for (int k = 0; k < nodes_amount; k++) {
      std::cout << "c Floyd-Warshall iteration " << k << std::endl;
      for (int i = 0; i < nodes_amount; i++) {
        for (int j = 0; j < nodes_amount; j++) {
          if (dist[i][k] + dist[k][j] < dist[i][j]) {
            dist[i][j] = dist[i][k] + dist[k][j];
            centrality[k] = 1;
          }
          else if (dist[i][k] + dist[k][j] == dist[i][j]) {
            centrality[k] += 1;
          }
        }
      }
    }
    std::cout << "c Betweenness centrality calculation" << std::endl;
    
    for (int l = 0; l < n_var; l++) {
      activity[l + 1] = std::max(centrality[l + n_constraints], centrality[l + n_constraints + n_var]) * n_var * 2;
    }
  }

  void Graph::calculateDegreeCentrality(std::vector<ActValV>& activity, bool weighted) {
    /*
    * Degree centrality measure on variables
    */
    std::cout << "c Degree centrality calculation" << std::endl;
    std::vector<long double> centrality(nodes_amount, 0);
    if (weighted)
      for (int i = 0; i < nodes_amount; i++) {
        for (const auto& [adjacent_node, weight]: adjacency_list[i]) {
          centrality[i] += weight;
        }
      }
    else
      for (int i = 0; i < nodes_amount; i++) {
        centrality[i] = adjacency_list[i].size();
      }
    for (int l = 0; l < n_var; l++) {
      activity[l + 1] = std::max(centrality[l + n_constraints], centrality[l + n_constraints + n_var]) * n_var * 2;
    }
  }

  void Graph::cleanMem() {
    adjacency_list.clear();
  }

}