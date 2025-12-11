# MetaPB: Solver Selection for Pseduo Boolean Optimization

MetaPB is a meta-solver designed to predict the best solver (e.g., RoundingSat, SCIP, NAPS, Mixed-Bag) for a given Pseudo-Boolean Optimization (PBO) instance.

The goal is to maximize the probability of solving an instance within a given time budget by selecting the solver most likely to succeed before execution.

## Repository Structure


1. `assets`: Contains all external resources, including feature extractors (static + RoundingSat) and trained ML models.
2. `meta_solver`:  Implements the core functionality of MetaPB: feature extraction, preprocessing, prediction logic, and helper utilities.
3. `config.py`: Parses command-line arguments and constructs the correct model paths based on the time limit.
4. `main.py`: Main entry point that loads extractors, processes the instance, and predicts the best solver.
5. `paths.conf`: Central configuration file defining default paths for extractors, models, and assets.


## Setup and Installation

### 1. Install Dependencies

```bash
pip install -r requirements.txt
```

## Compiling the C++ Feature Extractors

MetaPB uses two compiled C++ binaries for efficient feature extraction. After compiling, update their paths in paths.conf (STATIC_EXTRACTOR and ROUNDINGSAT_EXTRACTOR).

#### 1. Static features

```bash
cd assets/extractors/static_extractor
gcc static_features.cpp -lstdc++ -std=c++11 -o build/static_features
```

#### 2. CDCL-based features (Modified RoundingSat)

```bash
cd assets/extractors/roundingsat_extractor/build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## Running MetaPB

Execute the meta-solver with:

```bash
python main.py /path/to/instance.opb <time_limit_seconds>
```

The output is a prediction of the best solver to run given the time limit.

# Acknowledgment

## RoundingSAT

This project incorporates a modified version of RoundingSat. The original license is included in the third_party/ directory.

### Relevant Citations
1. Origin paper with a focus on cutting planes conflict analysis: [EN18] J. Elffers, J. Nordström. Divide and Conquer: Towards Faster Pseudo-Boolean Solving. IJCAI 2018

2. Integration with SoPlex: [DGN20] J. Devriendt, A. Gleixner, J. Nordström. Learn to Relax: Integrating 0-1 Integer Linear Programming with Pseudo-Boolean Conflict-Driven Search. CPAIOR 2020 / Constraints journal

3. Watched propagation: [D20] J. Devriendt. Watched Propagation for 0-1 Integer Linear Constraints. CP 2020

3. Core-guided optimization: [DGDNS21] J. Devriendt, S. Gocht, E. Demirović, J. Nordström, P. J. Stuckey. Cutting to the Core of Pseudo-Boolean Optimization: Combining Core-Guided Search with Cutting Planes Reasoning. AAAI 2021

## SCIP

MetaPB uses PySCIPOpt library 
```
@incollection{MaherMiltenbergerPedrosoRehfeldtSchwarzSerrano2016,
  author = {Stephen Maher and Matthias Miltenberger and Jo{\~{a}}o Pedro Pedroso and Daniel Rehfeldt and Robert Schwarz and Felipe Serrano},
  title = {{PySCIPOpt}: Mathematical Programming in Python with the {SCIP} Optimization Suite},
  booktitle = {Mathematical Software {\textendash} {ICMS} 2016},
  publisher = {Springer International Publishing},
  pages = {301--307},
  year = {2016},
  doi = {10.1007/978-3-319-42432-3_37},
}
```
