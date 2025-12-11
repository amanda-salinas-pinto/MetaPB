#pragma once

#include "typedefs.hpp"
#include "Constr.hpp"
#include <unordered_set>

namespace rs {


class Graph {
  /*
    Represents Formula as an uniderected graph.
  */
  std::unordered_map<int,  std::unordered_map<int, long double> > adjacency_list;
  int mode = 0;
  public:
    Graph();
    void init(ConstraintAllocator& ca, std::vector<CRef>& constraints, int nvars);
    /* helpers */
    /* centrality measures */
    void calculatePseudoBooleanCentrality(std::vector<ActValV>& activity, ConstraintAllocator& ca, std::vector<CRef>& constraints, int nvars);
    void calculateClosenessCentrality(std::vector<ActValV>& activity);
    void calculatePagerank(std::vector<ActValV>& activity);
    void calculateHITS(std::vector<ActValV>& activity);
    void calculateEigenvector(std::vector<ActValV>& activity);
    void calculateBetweennessCentrality(std::vector<ActValV>& activity);
    void calculateDegreeCentrality(std::vector<ActValV>& activity, bool weighted);
    /* memory cleanse */
    void cleanMem();
    void setMode(int newMode) {
      mode = newMode;
    }
  private:
    std::vector<std::vector<long double>> getShortestPaths();
    int nodes_amount;
    int n_var;
    int n_constraints;
};

}