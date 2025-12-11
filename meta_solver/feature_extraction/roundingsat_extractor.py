# meta_solver/feature_extraction/roundingsat_extractor.py
import subprocess
import os
from constants import SOLVED_BY_ROUNDINGSAT, INFEASIBLE

ROUNDINGSAT_EXTRACTOR_PATH = "" # Will be set in main.py

def run_roundingsat(instance_path: str, time_limit: int = 10):
    print("Extracting CDCL features...\n")
    features_keys = [
        "lbd_avg", "trailpop_per_conflict", "decisions_per_total_conflict",
        "propagations_per_total_conflict", "prop_per_decisions",
        "decisions_per_conflict", "propagations_per_conflict",
        "total_conflicts_per_conflict", "learned_clauses_per_conflict",
        "restarts_per_conflict", "learned_clause_length_avg", "time_taken" # time_taken is discarded later
    ]

    if not os.path.exists(ROUNDINGSAT_EXTRACTOR_PATH):
        raise FileNotFoundError(f"RoundingSat extractor not found at: {ROUNDINGSAT_EXTRACTOR_PATH}")
        
    cmd = [
        ROUNDINGSAT_EXTRACTOR_PATH, 
        f"--timeout={time_limit}",
        instance_path,
    ]
    
    result = subprocess.run(cmd, stdout=subprocess.PIPE, text=True, timeout=time_limit)
    output = result.stdout
    print("RS OUTPUT", output)
    
    lines = output.splitlines()
    float_features = []
    
    # Process the output line
    for line in lines:
        features = line.strip().split(",")
        if len(features) == 14:
            float_features = [float(x) for x in features[2:]]
            features_dict = dict(zip(features_keys, float_features))
            break

    if "OPT" in output:
        time = float(features[-1]) 
        obj = float(features[1])
        print("[ROUNDINGSAT TIME]", time)
        print("[ROUNDINGSAT OBJ]", obj)
        return SOLVED_BY_ROUNDINGSAT
        
    if "UNSAT" in output:
        return INFEASIBLE
        
    if not float_features:
        # Return list of zeros if features couldn't be extracted within the time limit
        return [0] * (len(features_keys) - 1) 


    return float_features[:-1] # Exclude the last element (time_taken)