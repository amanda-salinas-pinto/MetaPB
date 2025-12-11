# main.py
import os
import time
import math
import sys

from config import parse_arguments_and_configure, validate_paths_and_setup

# Import modules
from constants import SOLVED_BY_SCIP, SOLVED_BY_ROUNDINGSAT, INFEASIBLE
from meta_solver.feature_extraction.static_extractor import extract_static_features
from meta_solver.feature_extraction.roundingsat_extractor import run_roundingsat
from meta_solver.feature_extraction.scip_extractor import extract_scip_features
from meta_solver.feature_extraction.simplifier import simplify_with_scip
from meta_solver.model_prediction.predictor import predict_best_solver


def process_instance(config: dict, solver_time_limit: float = 3600.0) -> str:
    start_total = time.time()
    
    # Configuration extraction
    original_path = config['instance_path']
    modified_dir = config['modified_dir']
    time_scip = config['time_scip']
    time_rs = config['time_rs']
    alpha = config['alpha']
    s_alpha = config['s_alpha']
    
    instance_name = os.path.basename(original_path)
    modified_path = os.path.join(modified_dir, instance_name)
    
    # Static Feature Extraction
    start = time.time()
    static_features = extract_static_features(original_path)
    n_const = static_features[0] # Number of constraints
    print(f"[TIME] Static feature extraction took {time.time() - start:.2f} seconds")
    
    
    if n_const >= 100000:
        print("\n Static Only")
        all_features = static_features
        current_alpha = s_alpha
        best_solver = predict_best_solver(all_features, current_alpha, n_const)
    else:
        print("\n all features")
        current_alpha = alpha
        
        start_scip = time.time()
        scip_result = simplify_with_scip(original_path, modified_path, time_limit=time_scip)
        print(f"[TIME] SCIP simplification took {time.time() - start_scip:.2f} seconds")
        

        scip_model = scip_result
        start_extract_scip = time.time()
        scip_features = extract_scip_features(scip_model)
        print(f"[TIME] SCIP feature extraction took {time.time() - start_extract_scip:.2f} seconds")

        start_rs = time.time()
        rs_features = run_roundingsat(modified_path, time_limit=time_rs) 
        print(f"[TIME] RoundingSat feature extraction took {time.time() - start_rs:.2f} seconds")
        
        all_features = static_features + scip_features + rs_features
        
        all_features = [0 if (math.isinf(x) or math.isnan(x)) else x for x in all_features]
        start_predict = time.time()
        best_solver = predict_best_solver(all_features, current_alpha, n_const)
        print(f"[TIME] Prediction took {time.time() - start_predict:.2f} seconds")
    
    return best_solver


def main():
    if not os.path.exists(os.path.join(os.path.dirname(__file__), 'config.py')):
        print("[SETUP ERROR] Please ensure 'config.py' exists.")
        sys.exit(1)

    config = parse_arguments_and_configure()
    
    validate_paths_and_setup(config)

    try:
        best_solver = process_instance(config)
        print(best_solver)
    except Exception as e:
        print(f"\n[ERROR] Failed during instance processing: {e}", file=sys.stderr)

if __name__ == "__main__":
    main()