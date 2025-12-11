# config.py
import argparse
import os
import sys
import configparser
from typing import Dict, Any, List, Optional
import math 

CONF_FILE = 'paths.conf' 
VALID_TIME_LIMITS: List[int] = [60, 100, 300, 600, 3600] # Supported time limits for models

def load_config_file(conf_file: str) -> Dict[str, Dict[str, str]]:
    config = configparser.ConfigParser()
    if not os.path.exists(conf_file):
        print(f"[WARN] Configuration file '{conf_file}' not found. Using minimal defaults.")
        return {}
        
    config.read(conf_file)
    paths_by_section: Dict[str, Dict[str, str]] = {}
    
    current_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.join(current_dir, "")
    
    for section in config.sections():
        paths_by_section[section] = {}
        for key, value in config[section].items():
            if not os.path.isabs(value):
                paths_by_section[section][key.upper()] = os.path.join(project_root, value)
            else:
                 paths_by_section[section][key.upper()] = value
    
    return paths_by_section

def select_model_paths(general_paths: Dict[str, str], time_limit: int) -> Dict[str, str]:    
    if time_limit in VALID_TIME_LIMITS:
        effective_limit = time_limit
    else:
        supported_limits = [t for t in VALID_TIME_LIMITS if t >= time_limit]
        if not supported_limits:
            effective_limit = max(VALID_TIME_LIMITS)
        else:
            effective_limit = min(supported_limits)
        
        print(f"[CONFIG] Requested time limit T={time_limit}s not found. Using T={effective_limit}s model set.")

    models_base = general_paths.get('MODELS_PATH', '/tmp/models')
    scalers_base = general_paths.get('SCALERS_PATH', '/tmp/scalers')
    
    MODEL_PATHS = {
        'MODEL_PATH_MC': os.path.join(models_base, f"{effective_limit}_all_REGRESSION_RF_0.8_mc.joblib"),
        'MODEL_PATH_ML': os.path.join(models_base, f"{effective_limit}_all_REGRESSION_RF_0.8_ml.joblib"),
        'SCALER_PATH': os.path.join(scalers_base, f"{effective_limit}_all_REGRESSION_RF_0.8.joblib"),

        # Static-Only Feature Set
        'S_MODEL_PATH_MC': os.path.join(models_base, f"{effective_limit}_static_REGRESSION_RF_0.8_mc.joblib"),
        'S_MODEL_PATH_ML': os.path.join(models_base, f"{effective_limit}_static_REGRESSION_RF_0.8_ml.joblib"),
        'S_SCALER_PATH': os.path.join(scalers_base, f"{effective_limit}_static_REGRESSION_RF_0.8.joblib"), # Static scaler
    }
    
    return MODEL_PATHS


def parse_arguments_and_configure():
    
    file_paths_all = load_config_file(CONF_FILE)
    general_paths = file_paths_all.get('PATHS', {}) # Get paths from [PATHS] section
    
    parser = argparse.ArgumentParser(description="Select the best solver for a given instance using a meta-solver approach.")
    parser.add_argument("instance_path", type=str, help="Path to the instance file (.lp, .mps, etc.).")
    parser.add_argument("time_limit", type=int, help="Total time limit for the final solver run (in seconds). Used to select model set.")
    parser.add_argument("--time_scip", type=int, default=5, help="Time limit for SCIP presolve/probing (in seconds).")
    parser.add_argument("--time_rs", type=int, default=5, help="Time limit for RoundingSat probing (in seconds).")
    parser.add_argument("--alpha", type=float, default=0.8, help="Weighting factor (alpha) for small/medium instances.")
    parser.add_argument("--s_alpha", type=float, default=0.8, help="Weighting factor (s_alpha) for static-only/large instances.")    
  
    args, unknown = parser.parse_known_args()
    
    # Construct Model Paths based on time_limit
    model_paths_constructed = select_model_paths(general_paths, args.time_limit)

    parser_final = argparse.ArgumentParser(parents=[parser], add_help=False)

    args = parser_final.parse_args()
    
    config: Dict[str, Any] = {
        'instance_path': args.instance_path,
        'time_limit': args.time_limit,
        'time_scip': args.time_scip,
        'time_rs': args.time_rs,
        'alpha': args.alpha,
        's_alpha': args.s_alpha,
        'modified_dir': general_paths["MODIFIED_DIR"],

        'MODEL_PATHS': {
            'MODEL_PATH_MC': model_paths_constructed['MODEL_PATH_MC'],
            'MODEL_PATH_ML': model_paths_constructed['MODEL_PATH_ML'],
            'SCALER_PATH': model_paths_constructed['SCALER_PATH'],
            'S_MODEL_PATH_MC': model_paths_constructed['S_MODEL_PATH_MC'],
            'S_MODEL_PATH_ML': model_paths_constructed['S_MODEL_PATH_ML'],
            'S_SCALER_PATH': model_paths_constructed['S_SCALER_PATH'],
        },
        
        'STATIC_EXTRACTOR_PATH': general_paths['STATIC_EXTRACTOR'],
        'ROUNDINGSAT_EXTRACTOR_PATH': general_paths['ROUNDINGSAT_EXTRACTOR'],
    
    }
    
    return config


def validate_paths_and_setup(config):
    from meta_solver.feature_extraction import static_extractor, roundingsat_extractor
    from meta_solver.model_prediction import predictor

    static_extractor.STATIC_EXTRACTOR_PATH = config['STATIC_EXTRACTOR_PATH']
    roundingsat_extractor.ROUNDINGSAT_EXTRACTOR_PATH = config['ROUNDINGSAT_EXTRACTOR_PATH']
    predictor.MODEL_PATHS.update(config['MODEL_PATHS'])

    paths_to_check = {
        "Static Extractor": config['STATIC_EXTRACTOR_PATH'],
        "RoundingSat Extractor": config['ROUNDINGSAT_EXTRACTOR_PATH'],
        "Model MC": config['MODEL_PATHS']['MODEL_PATH_ML'],
        "Model ML": config['MODEL_PATHS']['MODEL_PATH_MC'],
        "Scaler": config['MODEL_PATHS']['SCALER_PATH'],
        "Static Model MC": config['MODEL_PATHS']['S_MODEL_PATH_ML'],
        "Static Model ML": config['MODEL_PATHS']['S_MODEL_PATH_MC'],
        "Static Scaler": config['MODEL_PATHS']['S_SCALER_PATH'],
    }
    
    for name, path in paths_to_check.items():
        if os.path.exists(path):
            print(f"[OK] {name}: {path}")
        else:
            print(f"[ERROR] {name} does NOT exist: {path}")
            sys.exit(f"Required file missing: {path}")
                 

    os.makedirs(config['modified_dir'], exist_ok=True)
    print(f"[OK] Modified Dir: {config['modified_dir']}")