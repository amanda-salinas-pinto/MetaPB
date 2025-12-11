/***********************************************************************
Copyright (c) 2014-2020, Jan Elffers
Copyright (c) 2019-2021, Jo Devriendt
Copyright (c) 2020-2021, Stephan Gocht
Copyright (c) 2014-2021, Jakob Nordström

Parts of the code were copied or adapted from MiniSat.

MiniSat -- Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
           Copyright (c) 2007-2010  Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***********************************************************************/

#pragma once

#include <random>
#include <memory>
#include "Constr.hpp"
#include "IntSet.hpp"
#include "LpSolver.hpp"
#include "Options.hpp"
#include "typedefs.hpp"
#include "Graph.hpp"

namespace rs {

struct Logger;

enum class SolveState { SAT, UNSAT, INCONSISTENT, INPROCESSED, RESTARTED };

struct SolveAnswer {
  SolveState state;
  std::vector<CeSuper> cores;
  std::vector<Lit>& solution;
};

struct SummaryStats {
  double lbd_sum = 0;
  int ndecide_sum = 0;
  int nprop_sum = 0;
  int nconflict_total = 0;  // Also serves as conflict count
  int nclauses_learned_sum = 0;
  int ntrailpops_sum = 0;
  int nrestarts_sum = 0;
  int learned_length_sum = 0;
  int current_upper_bound_sum = 0;
  
  // Counters
  int conflict_count = 0;
  bool isOpt = false;
  

  void addConflict(double lbd, const Stats& stats) {
      lbd_sum += lbd;
      ndecide_sum += stats.NDECIDE;
      nprop_sum += stats.NPROP;
      nconflict_total += stats.NCONFL;
      nclauses_learned_sum += stats.NCLAUSESLEARNED;
      ntrailpops_sum += stats.NTRAILPOPS;
      nrestarts_sum += stats.NRESTARTS;
      learned_length_sum += (stats.NCLAUSESLEARNED > 0 ? stats.LEARNEDLENGTHSUM : 0);
      current_upper_bound_sum += stats.currentUpperBound;
      conflict_count++;
  }

  template <typename LARGE>
  void printSummary(std::ostream& out, int opt, const LARGE& bestObjVal) const {
    out << (opt == 1 ? "OPT" : (opt == 2 ? "UNSAT" : "UNK")) << ","
        << bestObjVal << ","
        << (conflict_count == 0 ? 0.0 : lbd_sum / conflict_count) << ","
        << (conflict_count == 0 ? 0.0 : static_cast<double>(ntrailpops_sum) / conflict_count) << ","
        << (conflict_count == 0 ? 0.0 : static_cast<double>(ndecide_sum) / nconflict_total) << ","
        << (conflict_count == 0 || nconflict_total == 0 ? 0.0 : static_cast<double>(nprop_sum) / nconflict_total) << ","
        << (conflict_count == 0 || ndecide_sum == 0 ? 0.0 : static_cast<double>(nprop_sum) / ndecide_sum) << ","
        << (conflict_count == 0 ? 0.0 : static_cast<double>(ndecide_sum) / conflict_count) << ", "
        << (conflict_count == 0 ? 0.0 : static_cast<double>(nprop_sum) / conflict_count) << ", "
        << (conflict_count == 0 ? 0.0 : static_cast<double>(nconflict_total) / conflict_count) << ", "
        << (conflict_count == 0 ? 0.0 : static_cast<double>(nclauses_learned_sum) / conflict_count) << ", "
        << (conflict_count == 0 ? 0.0 : static_cast<double>(nrestarts_sum) / conflict_count) << ", "
        << (conflict_count == 0 || nclauses_learned_sum == 0 ? 0.0 : static_cast<double>(learned_length_sum) / nclauses_learned_sum) << ", "
        // << bestObjVal << ","
        << stats.getTime() << std::endl;

              // << (opt == 1 ? "OPT" : (opt == 2 ? "UNSAT" : "UNK")) << ","
        // << (conflict_count == 0 ? 0.0 : static_cast<double>(current_upper_bound_sum) / conflict_count)

        
        
  }


};

class Solver {
  friend class LpSolver;
  friend struct Constr;
  friend struct Clause;
  friend struct Cardinality;
  template <typename CF, typename DG>
  friend struct Counting;
  template <typename CF, typename DG>
  friend struct Watched;
  template <typename CF, typename DG>
  friend struct CountingSafe;
  template <typename CF, typename DG>
  friend struct WatchedSafe;


  // ---------------------------------------------------------------------
  // Members

 public:
  std::shared_ptr<Logger> logger;
  ConstrExpPools cePools;
  std::vector<Lit> lastSol = {0};
  bool foundSolution() const { return lastSol.size() > 1; }
  int getGarbageCalls() const {return garbage_calls;}
  std::vector<CRef> constraints;
  ConstraintAllocator ca;
 private:
  SummaryStats summary_stats;
  int garbage_calls;
  int n;
  int orig_n;
  ID crefID = ID_Trivial;
  IntSet tmpSet;
  IntSet actSet;  // Set of literals that need their activity bumped after conflict analysis.
  OrderHeap order_heap;
  Graph graph;
  std::unordered_map<ID, CRef> external;
  std::vector<std::vector<Watch>> _adj = {{}};
  std::vector<std::vector<Watch>>::iterator adj;
  std::vector<int> _Level = {INF};
  IntVecIt Level;  // TODO: make Pos, Level, contiguous memory for better cache efficiency.
  std::vector<Lit> trail;
  std::vector<int> trail_lim, Pos;
  std::vector<CRef> Reason;
  int qhead = 0;  // for unit propagation

  std::vector<Lit> phase;
  std::vector<ActValV> activity;

  long long nconfl_to_reduce = 2000;
  long long nconfl_to_restart = 0;
  bool hasRestarted = false;
  ActValV v_vsids_inc = 1.0;
  ActValC c_vsids_inc = 1.0;

  bool firstRun = true;

  std::shared_ptr<LpSolver> lpSolver;
  std::vector<std::unique_ptr<ConstrSimpleSuper>> learnedStack;

  IntSet assumptions;

 public:
  Solver();


  template <typename LARGE>
  void printSummaryStats(std::ostream& out, int opt, const LARGE& bestObjVal) const {
    summary_stats.printSummary(out, opt, bestObjVal);
  }

  void init();  // call after having read options
  void initLP(const CeArb objective);

  int getNbVars() const { return n; }
  void setNbVars(long long nvars, bool orig = false);
  int getNbOrigVars() const { return orig_n; }

  const IntVecIt& getLevel() const { return Level; }
  const std::vector<int>& getPos() const { return Pos; }
  int decisionLevel() const { return trail_lim.size(); }

  std::pair<ID, ID> addConstraint(const CeSuper c, Origin orig);             // result: formula line id, processed id
  std::pair<ID, ID> addConstraint(const ConstrSimpleSuper& c, Origin orig);  // result: formula line id, processed id
  std::pair<ID, ID> addUnitConstraint(Lit l, Origin orig);                   // result: formula line id, processed id
  void dropExternal(ID id, bool erasable, bool forceDelete);
  int getNbConstraints() const { return constraints.size(); }
  CeSuper getIthConstraint(int i) { return ca[constraints[i]].toExpanded(cePools); }

  void setAssumptions(const IntSet& assumps);
  const IntSet& getAssumptions() { return assumptions; }

  /**
   * @return SolveAnswer.state:
   * 	UNSAT if root inconsistency detected
   * 	SAT if satisfying assignment found
   * 	INCONSISTENT if no solution extending assumptions exists
   * 	INPROCESSING if solver just finished a cleanup phase
   * @return SolveAnswer.cores:
   *    implied constraints C if INCONSISTENT
   *        if C is a tautology, negated assumptions at root level exist
   *        if C is not a tautology, it is falsified by the assumptions
   * @return SolveAnswer.solution:
   *    the satisfying assignment if SAT
   */
  // TODO: use a coroutine / yield instead of a SolveAnswer return value
  SolveAnswer solve();

 private:
  void presolve();
  void recalculatVarPriorityGlobally();
  // ---------------------------------------------------------------------
  // VSIDS

  void vDecayActivity();
  void vBumpActivity(Var v);
  void cDecayActivity();
  void cBumpActivity(Constr& c);
  // ---------------------------------------------------------------------
  // Trail manipulation

  void uncheckedEnqueue(Lit p, CRef from);
  void removeLastAssignment();
  void backjumpTo(int level);
  void decide(Lit l);
  void propagate(Lit l, CRef reason);
  /**
   * Unit propagation with watched literals.
   * @post: all constraints have been checked for propagation under trail[0..qhead[
   * @return: true if inconsistency is detected, false otherwise. The inconsistency is stored in confl->
   */
  CeSuper runPropagation(bool onlyUnitPropagation);
  WatchStatus checkForPropagation(CRef cr, int& idx, Lit p);

  // ---------------------------------------------------------------------
  // Conflict analysis

  void recomputeLBD(Constr& C);
  CeSuper prepareConflictConstraint(CeSuper conflict);
  /**
   * @brief Get initial set of literals that need their activity bumped after conflict analysis.
   *
   * @param confl Conflict side constraint.
   */
  void assignActiveSet(CeSuper confl);
  Constr& getReasonConstraint(Lit l);
  void trackReasonConstraintStats(Constr& reasonC);
  void bumpLiteralActivity();
  CeSuper analyze(CeSuper confl);
  std::vector<CeSuper> extractCore(CeSuper confl, const IntSet& assumptions, Lit l_assump = 0);

  // ---------------------------------------------------------------------
  // Constraint management

  CRef attachConstraint(CeSuper constraint, bool locked);
  void learnConstraint(const CeSuper c, Origin orig);
  CeSuper processLearnedStack();
  std::pair<ID, ID> addInputConstraint(CeSuper ce);
  void removeConstraint(Constr& C, bool override = false);

  // ---------------------------------------------------------------------
  // Garbage collection

  void garbage_collect();
  void reduceDB();

  // ---------------------------------------------------------------------
  // Solving

  double luby(double y, int i) const;
  bool checkSAT();
  void printWholeConstraints();
  void buildVariablePriority();
  long double** buildGraph();
  Lit pickBranchLit(bool lastSolPhase);
};

}  // namespace rs
