import subprocess
import os

STATIC_EXTRACTOR_PATH = "" # Will be set in main.py

def extract_static_features(instance_path: str) -> list[float]:
    print("Extracting Static features...\n")
    
    static_features_keys = [
        "n_constraints", "n_variables", "pct_unary_constraints",
        "pct_binary_constraints", "pct_ternary_constraints", "pct_quaternary_or_more",
        "pct_terms_in_obj", "pct_terms_in_constraints",
        "pct_positive_terms_in_constraints", "pct_positive_terms_in_obj"
    ]
    
    if not os.path.exists(STATIC_EXTRACTOR_PATH):
        raise FileNotFoundError(f"Static extractor not found at: {STATIC_EXTRACTOR_PATH}")

    cmd = [
        STATIC_EXTRACTOR_PATH,
        instance_path
    ]
    
    result = subprocess.run(cmd, stdout=subprocess.PIPE, text=True, check=True)
    output = result.stdout
    
    features = output.strip().split(" ")
    float_features = [float(x) for x in features]
    
    return float_features