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

#include "auxiliary.hpp"
#include "typedefs.hpp"
#include <fstream>
#include <filesystem>
#include <chrono>


namespace rs {


struct Stats {


bool TLE(){
  if( timelimit != -1  ){
	std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double> > (std::chrono::high_resolution_clock::now() - startTime);
	return duration.count()>=timelimit;
  }
  return false;
} 
  bool isOpt = false;
  int timelimit;
  std::chrono::high_resolution_clock::time_point startTime; //std::chrono::high_resolution_clock::now();  
  std::stringstream restartStream;
  std::stringstream decisionStream;
  std::stringstream timeStream;
  std::stringstream fixedStream;
  std::vector<int> solveCount;
  long long currentLowerBound = 0; // Mejor valor de la función objetivo encontrado
  long long currentUpperBound = 0; // Valor actual de la función objetivo

  long long NTRAILPOPS = 0, NWATCHLOOKUPS = 0, NWATCHLOOKUPSBJ = 0, NWATCHCHECKS = 0, NPROPCHECKS = 0,
            NADDEDLITERALS = 0;
  long long NCONFL = 0, NDECIDE = 0, NPROP = 0, NPROPCLAUSE = 0, NPROPCARD = 0, NPROPWATCH = 0, NPROPCOUNTING = 0,
            NRESOLVESTEPS = 0, NMAXCONSTRAINTS = 0;
  long long NWATCHED = 0, NCOUNTING = 0;
  int128 EXTERNLENGTHSUM = 0, LEARNEDLENGTHSUM = 0;
  bigint EXTERNDEGREESUM = 0, LEARNEDDEGREESUM = 0;
  long long NCLAUSESEXTERN = 0, NCARDINALITIESEXTERN = 0, NGENERALSEXTERN = 0;
  long long NCLAUSESLEARNED = 0, NCARDINALITIESLEARNED = 0, NGENERALSLEARNED = 0;
  long long NGCD = 0, NCARDDETECT = 0, NCORECARDINALITIES = 0, NCORES = 0, NSOLS = 0;
  long long NWEAKENEDNONIMPLYING = 0, NWEAKENEDNONIMPLIED = 0;
  long long NRESTARTS = 0, NCLEANUP = 0;
  double STARTTIME = 0;
  int LASTVSIDSDECISION = 0;
  long long NORIGVARS = 0, NAUXVARS = 0;
  long long NCONSFORMULA = 0, NCONSLEARNED = 0, NCONSBOUND = 0, NCONSCOREGUIDED = 0;
  long long NENCFORMULA = 0, NENCLEARNED = 0, NENCBOUND = 0,
            NENCCOREGUIDED;  // Number of times a reason constraint of this type was encountered.

  long long NLPADDEDROWS = 0, NLPDELETEDROWS = 0;
  long long NLPPIVOTSINTERNAL = 0, NLPPIVOTSROOT = 0, NLPNOPIVOT = 0, NLPRESETBASIS = 0;
  double LPSOLVETIME = 0, LPTOTALTIME = 0;
  long long NLPCALLS = 0, NLPOPTIMAL = 0, NLPINFEAS = 0, NLPFARKAS = 0;
  long long NLPCYCLING = 0, NLPNOPRIMAL = 0, NLPNOFARKAS = 0, NLPSINGULAR = 0, NLPOTHER = 0;
  long long NLPGOMORYCUTS = 0, NLPLEARNEDCUTS = 0, NLPLEARNEDFARKAS = 0, NLPDELETEDCUTS = 0;
  long long NLPENCGOMORY = 0, NLPENCFARKAS = 0,
            NLPENCLEARNEDFARKAS = 0;  // Number of times a reason constraint of this type was encountered.

  long long UNITCORES = 0, SINGLECORES = 0, REMOVEDBLOCKS = 0, FIRSTCOREBEST = 0, DECCOREBEST = 0, NOCOREBEST = 0,
            COREDEGSUM = 0, CORESLACKSUM = 0;
  double HEURISTICTIME = 0; double MAXHUERISTICTIME = 0;
  double HEURISTICAMOUNT = 0;
  double SOLVETIME = 0, SOLVETIMECG = 0, CATIME = 0, PROPTIME = 0;
  double RUNSTARTTIME = 0;

  inline double getTime() const { return aux::cpuTime() - STARTTIME; }
  inline double getRunTime() const { return aux::cpuTime() - RUNSTARTTIME; }
  inline double getSolveTime() const { return SOLVETIME + SOLVETIMECG; }

  inline long long getDetTime() const {
    return 1 + NADDEDLITERALS + NWATCHLOOKUPS + NWATCHLOOKUPSBJ + NWATCHCHECKS + NPROPCHECKS + NPROP + NTRAILPOPS +
           NDECIDE + NLPPIVOTSROOT + NLPPIVOTSINTERNAL;
  }

  template <typename LARGE>
  void printStatus(std::string formulaName, int isSat, const LARGE& bestObjVal) {
    // prints a csv like row
    std::cout << formulaName << ",";
    std::cout << std::to_string(HEURISTICTIME) << ",";
    std::cout << std::to_string(NMAXCONSTRAINTS) << ",";
    std::cout << std::to_string(MAXHUERISTICTIME) << ",";
    std::cout << std::to_string(getTime()) << ",";
    std::cout <<std::to_string(isSat) << ",";
    std::cout << std::to_string(HEURISTICAMOUNT) << ",";
    std::cout << std::to_string(NDECIDE) << ",";
    std::cout << std::to_string(NCONFL) << ",";
    std::cout << std::to_string(NORIGVARS) << ",";
    std::cout << std::to_string(NCONSFORMULA) << ",";
    std::cout <<  std::to_string(NRESTARTS) << ",";
    std::cout << std::to_string(NCONSLEARNED) << ",";
    std::cout << bestObjVal << ",";
    std::cout << (isOpt ? "OPT" : "DEC") << ",";
    std::cout << std::endl;
  }

  void printRestart(std::string formulaName){
    std::ofstream restartFile("amanda_results/" + formulaName + "_" + "restart.txt");
    // std::ofstream decisionFile("lukas_results/" + formulaName + "_" + "decision.txt");
    // std::ofstream timeFile("cata_results/" + formulaName + "_" + "times.txt");
    // std::ofstream fixedFile("irri_results/" + formulaName + "_" + "fixed.txt");
    if(!solveCount.empty()){
        for( Var v=1; v<=NORIGVARS; ++v){
            fixedStream << -v <<' '<<solveCount[v<<1] << '\n';

            fixedStream << v <<' '<< solveCount[(v<<1)+1] << '\n';
	    

        }
    }

    if (!restartFile) {
        std::cerr << "Cannot open the output file!" << std::endl;
        exit(1);
    }

    if (restartFile){
      restartFile << restartStream.str();

      if (restartFile.fail()) {
        std::cerr << "Failed to write to the file." << std::endl;
        exit(1);
      }
  
      restartFile.close();
      if (restartFile.fail()) {
        std::cerr << "Failed to close the file." << std::endl;
        exit(1);
      }
    }
    else {
      std::cout<< "Error\n" << std::endl;
    }
  }

  void printFile(std::string formulaName, int isSat, bigint bestObjVal) const {
    std::unordered_map<std::string, std::string> data;
    std::ofstream resultsFile;  
    resultsFile.open("results.csv", std::ios_base::app);
    long long nonLearneds = NCLAUSESEXTERN + NCARDINALITIESEXTERN + NGENERALSEXTERN;
    long long learneds = NCLAUSESLEARNED + NCARDINALITIESLEARNED + NGENERALSLEARNED;
    data["best_solution"] =  bestObjVal.str();
    data["total_pagerank_time"] = std::to_string(HEURISTICTIME);
    data["max_constraints_on_pagerank"] = std::to_string(NMAXCONSTRAINTS);
    data["max_pagerank_time"] = std::to_string(MAXHUERISTICTIME);
    data["name"] = formulaName;
    data["cpu_time"] = std::to_string(getTime());
    data["optimization_time"] = std::to_string(getRunTime() - getSolveTime());
    data["solve_time"] = std::to_string(getSolveTime());
    data["core-guided_solve_time"] = std::to_string(SOLVETIMECG);
    data["propagation_time"] = std::to_string(PROPTIME);
    data["conflict_analysis_time"] = std::to_string(CATIME);
    data["propagations"] = std::to_string(NPROP);
    data["resolve_steps"] = std::to_string(NRESOLVESTEPS);
    data["decisions"] = std::to_string(NDECIDE);
    data["conflicts"] = std::to_string(NCONFL);
    data["restarts"] = std::to_string(NRESTARTS);
    data["inprocessing_phases"] = std::to_string(NCLEANUP);
    data["input_clauses"] = std::to_string(NCLAUSESEXTERN);
    data["input_cardinalities"] = std::to_string(NCARDINALITIESEXTERN);
    data["intput_general_constraints"] = std::to_string(NGENERALSEXTERN);
    data["input_average_constraint_length"] = std::to_string((nonLearneds == 0 ? 0 : (double)EXTERNLENGTHSUM / nonLearneds));
    data["input_average_constraint_degree"] = std::to_string((nonLearneds == 0 ? 0 : (double)EXTERNDEGREESUM / nonLearneds));
    data["learned_clauses"] = std::to_string(NCLAUSESLEARNED);
    data["learned_cardinalities"] = std::to_string(NCARDINALITIESLEARNED);
    data["learned_general_constraints"] = std::to_string(NGENERALSLEARNED);
    data["learned_average_constraint_length"] = std::to_string((learneds == 0 ? 0 : (double)LEARNEDLENGTHSUM / learneds));
    data["learned_average_constraint_degree"] =  std::to_string((learneds == 0 ? 0 : (double)LEARNEDDEGREESUM / learneds));
    data["watched_constraints"] = std::to_string(NWATCHED);
    data["counting_constraints"] = std::to_string(NCOUNTING);
    data["gcd_simplifications"] = std::to_string(NGCD);
    data["detected_cardinalities"] = std::to_string(NCARDDETECT);
    data["weakened_non-implied_lits"] = std::to_string(NWEAKENEDNONIMPLIED);
    data["weakened_non-implying_lits"] = std::to_string(NWEAKENEDNONIMPLYING);
    data["original_variables"] = std::to_string(NORIGVARS);
    data["clausal_propagations"] = std::to_string(NPROPCLAUSE);
    data["cardinality_propagations"] = std::to_string(NPROPCARD);
    data["watched_propagations"] = std::to_string(NPROPWATCH);
    data["counting_propagations"] = std::to_string(NPROPCOUNTING);
    data["watch_lookups"] = std::to_string(NWATCHLOOKUPS);
    data["watch_backjump_lookups"] = std::to_string(NWATCHLOOKUPSBJ);
    data["watch_checks"] = std::to_string(NWATCHCHECKS);
    data["propagation_checks"] =  std::to_string(NPROPCHECKS);
    data["constraint_additions"] = std::to_string(NADDEDLITERALS);
    data["trail_pops"] = std::to_string(NTRAILPOPS);
    data["formula_constraints"] = std::to_string(NCONSFORMULA);
    data["learned_constraints"] = std::to_string(NCONSLEARNED);
    data["bound_constraints"] = std::to_string(NCONSBOUND);
    data["core_guided_constraints"] = std::to_string(NCONSCOREGUIDED);
    data["encountered_formula_constraints"] = std::to_string(NENCFORMULA);
    data["encountered_learned_constraints"] = std::to_string(NENCLEARNED);
    data["encountered_bound_constraints"] = std::to_string(NENCBOUND);
    data["encountered_core-guided_constraints"] = std::to_string(NENCCOREGUIDED);
    data["LP_total time"] = std::to_string(LPTOTALTIME);
    data["LP_solve time"] = std::to_string(LPSOLVETIME);
    data["LP_constraints_added"] = std::to_string(NLPADDEDROWS);
    data["LP_constraints_removed"] = std::to_string(NLPDELETEDROWS);
    data["LP_pivots_internal"] = std::to_string(NLPPIVOTSINTERNAL);
    data["LP_pivots_root"] = std::to_string(NLPPIVOTSROOT);
    data["LP_calls"] = std::to_string(NLPCALLS);
    data["LP_optimalities"] = std::to_string(NLPOPTIMAL);
    data["LP_no_pivot_count"] = std::to_string(NLPNOPIVOT);
    data["LP_infeasibilities"] = std::to_string(NLPINFEAS);
    data["LP_valid_Farkas_constraints"] = std::to_string(NLPFARKAS);
    data["LP_learned_Farkas_constraints"] = std::to_string(NLPLEARNEDFARKAS);
    data["LP_basis_resets"] = std::to_string(NLPRESETBASIS);
    data["LP_cycling_count"] = std::to_string(NLPCYCLING);
    data["LP_singular_count"] = std::to_string(NLPSINGULAR);
    data["LP_no_primal_count"] = std::to_string(NLPNOPRIMAL);
    data["LP_no_farkas_count"] = std::to_string(NLPNOFARKAS);
    data["LP_other_issue_count"] = std::to_string(NLPOTHER);
    data["LP_Gomory_cuts"] = std::to_string(NLPGOMORYCUTS);
    data["LP_learned_cuts"] = std::to_string(NLPLEARNEDCUTS);
    data["LP_deleted_cuts"] = std::to_string(NLPDELETEDCUTS);
    data["LP_encountered_Gomory_constraints"] = std::to_string(NLPENCGOMORY);
    data["LP_encountered_Farkas_constraints"] = std::to_string(NLPENCFARKAS);
    data["LP_encountered learned Farkas constraints"] = std::to_string(NLPENCLEARNEDFARKAS);
    data["CG_auxiliary_variables_introduced"] = std::to_string(NAUXVARS);
    data["CG_solutions_found"] = std::to_string(NSOLS);
    data["CG_cores_constructed"] = std::to_string(NCORES);
    data["CG_core_cardinality_constraints_returned"] = std::to_string(NCORECARDINALITIES);
    data["CG_unit_cores"] = std::to_string(UNITCORES);
    data["CG_single_cores"] = std::to_string(SINGLECORES);
    data["CG_blocks_emoved_during_cardinality_reduction"] = std::to_string(REMOVEDBLOCKS);
    data["CG_first_core_best"] = std::to_string(FIRSTCOREBEST);
    data["CG_decision_core_best"] = std::to_string(DECCOREBEST);
    data["CG_core_reduction_tie"] = std::to_string(NOCOREBEST);
    data["CG_core_degree_average"] = std::to_string((NCORES - UNITCORES) == 0 ? 0 : COREDEGSUM / (double)(NCORES - UNITCORES));
    data["CG_core_slack_average"] =  std::to_string((NCORES - UNITCORES) == 0 ? 0 : CORESLACKSUM / (double)(NCORES - UNITCORES)); 
    data["sat_state"] = std::to_string(isSat);
    data["pagerank_calls"] = std::to_string(HEURISTICAMOUNT);
    if (resultsFile.is_open()){
      // Write header only if file is empty
      resultsFile.seekp(0, std::ios::end);
      if(resultsFile.tellp() == 0) {
          for (const auto& pair : data) {
              resultsFile << pair.first << ",";
          }
          resultsFile << "\n";
      }
      for (const auto& pair : data) {
        resultsFile << pair.second << ",";
      }
      resultsFile << "\n";
      resultsFile.close(); // Remember to close the file
    } else {
        std::cout << "Unable to open file";
    }
  }

  void print() const {
    printf("c cpu time %g s\n", getTime());
    printf("c deterministic time %lld %.2e\n", getDetTime(), (double)getDetTime());
    printf("c optimization time %g s\n", getRunTime() - getSolveTime());
    printf("c total solve time %g s\n", getSolveTime());
    printf("c core-guided solve time %g s\n", SOLVETIMECG);
    printf("c propagation time %g s\n", PROPTIME);
    printf("c conflict analysis time %g s\n", CATIME);
    printf("c propagations %lld\n", NPROP);
    printf("c resolve steps %lld\n", NRESOLVESTEPS);
    printf("c decisions %lld\n", NDECIDE);
    printf("c conflicts %lld\n", NCONFL);
    printf("c restarts %lld\n", NRESTARTS);
    printf("c inprocessing phases %lld\n", NCLEANUP);
    printf("c input clauses %lld\n", NCLAUSESEXTERN);
    printf("c input cardinalities %lld\n", NCARDINALITIESEXTERN);
    printf("c input general constraints %lld\n", NGENERALSEXTERN);
    long long nonLearneds = NCLAUSESEXTERN + NCARDINALITIESEXTERN + NGENERALSEXTERN;
    printf("c input average constraint length %.2f\n", nonLearneds == 0 ? 0 : (double)EXTERNLENGTHSUM / nonLearneds);
    printf("c input average constraint degree %.2f\n", nonLearneds == 0 ? 0 : (double)EXTERNDEGREESUM / nonLearneds);
    printf("c learned clauses %lld\n", NCLAUSESLEARNED);
    printf("c learned cardinalities %lld\n", NCARDINALITIESLEARNED);
    printf("c learned general constraints %lld\n", NGENERALSLEARNED);
    long long learneds = NCLAUSESLEARNED + NCARDINALITIESLEARNED + NGENERALSLEARNED;
    printf("c learned average constraint length %.2f\n", learneds == 0 ? 0 : (double)LEARNEDLENGTHSUM / learneds);
    printf("c learned average constraint degree %.2f\n", learneds == 0 ? 0 : (double)LEARNEDDEGREESUM / learneds);
    printf("c watched constraints %lld\n", NWATCHED);
    printf("c counting constraints %lld\n", NCOUNTING);
    printf("c gcd simplifications %lld\n", NGCD);
    printf("c detected cardinalities %lld\n", NCARDDETECT);
    printf("c weakened non-implied lits %lld\n", NWEAKENEDNONIMPLIED);
    printf("c weakened non-implying lits %lld\n", NWEAKENEDNONIMPLYING);
    printf("c original variables %lld\n", NORIGVARS);
    printf("c clausal propagations %lld\n", NPROPCLAUSE);
    printf("c cardinality propagations %lld\n", NPROPCARD);
    printf("c watched propagations %lld\n", NPROPWATCH);
    printf("c counting propagations %lld\n", NPROPCOUNTING);
    printf("c watch lookups %lld\n", NWATCHLOOKUPS);
    printf("c watch backjump lookups %lld\n", NWATCHLOOKUPSBJ);
    printf("c watch checks %lld\n", NWATCHCHECKS);
    printf("c propagation checks %lld\n", NPROPCHECKS);
    printf("c constraint additions %lld\n", NADDEDLITERALS);
    printf("c trail pops %lld\n", NTRAILPOPS);
    printf("c formula constraints %lld\n", NCONSFORMULA);
    printf("c learned constraints %lld\n", NCONSLEARNED);
    printf("c bound constraints %lld\n", NCONSBOUND);
    printf("c core-guided constraints %lld\n", NCONSCOREGUIDED);
    printf("c encountered formula constraints %lld\n", NENCFORMULA);
    printf("c encountered learned constraints %lld\n", NENCLEARNED);
    printf("c encountered bound constraints %lld\n", NENCBOUND);
    printf("c encountered core-guided constraints %lld\n", NENCCOREGUIDED);
    printf("c LP total time %g s\n", LPTOTALTIME);
    printf("c LP solve time %g s\n", LPSOLVETIME);
    printf("c LP constraints added %lld\n", NLPADDEDROWS);
    printf("c LP constraints removed %lld\n", NLPDELETEDROWS);
    printf("c LP pivots internal %lld\n", NLPPIVOTSINTERNAL);
    printf("c LP pivots root %lld\n", NLPPIVOTSROOT);
    printf("c LP calls %lld\n", NLPCALLS);
    printf("c LP optimalities %lld\n", NLPOPTIMAL);
    printf("c LP no pivot count %lld\n", NLPNOPIVOT);
    printf("c LP infeasibilities %lld\n", NLPINFEAS);
    printf("c LP valid Farkas constraints %lld\n", NLPFARKAS);
    printf("c LP learned Farkas constraints %lld\n", NLPLEARNEDFARKAS);
    printf("c LP basis resets %lld\n", NLPRESETBASIS);
    printf("c LP cycling count %lld\n", NLPCYCLING);
    printf("c LP singular count %lld\n", NLPSINGULAR);
    printf("c LP no primal count %lld\n", NLPNOPRIMAL);
    printf("c LP no farkas count %lld\n", NLPNOFARKAS);
    printf("c LP other issue count %lld\n", NLPOTHER);
    printf("c LP Gomory cuts %lld\n", NLPGOMORYCUTS);
    printf("c LP learned cuts %lld\n", NLPLEARNEDCUTS);
    printf("c LP deleted cuts %lld\n", NLPDELETEDCUTS);
    printf("c LP encountered Gomory constraints %lld\n", NLPENCGOMORY);
    printf("c LP encountered Farkas constraints %lld\n", NLPENCFARKAS);
    printf("c LP encountered learned Farkas constraints %lld\n", NLPENCLEARNEDFARKAS);
    printf("c CG auxiliary variables introduced %lld\n", NAUXVARS);
    printf("c CG solutions found %lld\n", NSOLS);
    printf("c CG cores constructed %lld\n", NCORES);
    printf("c CG core cardinality constraints returned %lld\n", NCORECARDINALITIES);
    printf("c CG unit cores %lld\n", UNITCORES);
    printf("c CG single cores %lld\n", SINGLECORES);
    printf("c CG blocks removed during cardinality reduction %lld\n", REMOVEDBLOCKS);
    printf("c CG first core best %lld\n", FIRSTCOREBEST);
    printf("c CG decision core best %lld\n", DECCOREBEST);
    printf("c CG core reduction tie %lld\n", NOCOREBEST);
    printf("c CG core degree average %.2f\n",
           (NCORES - UNITCORES) == 0 ? 0 : COREDEGSUM / (double)(NCORES - UNITCORES));
    printf("c CG core slack average %.2f\n",
           (NCORES - UNITCORES) == 0 ? 0 : CORESLACKSUM / (double)(NCORES - UNITCORES));
  }
};

}  // namespace rs
