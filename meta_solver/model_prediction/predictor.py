from joblib import load
import numpy as np
import time

MODEL_PATHS = {}

def predict_best_solver(features: list[float], alpha: float, n_const: int) -> str:
    print("Predicting best solver...\n")
    
    # Select Model/Scaler Paths based on n_const threshold (100,000)
    if n_const >= 100000:
        model_set = "static"
        PATH_MC = MODEL_PATHS['S_MODEL_PATH_MC']
        PATH_ML = MODEL_PATHS['S_MODEL_PATH_ML']
        S_PATH = MODEL_PATHS['S_SCALER_PATH']
    else:
        model_set = "full"
        PATH_MC = MODEL_PATHS['MODEL_PATH_MC']
        PATH_ML = MODEL_PATHS['MODEL_PATH_ML']
        S_PATH = MODEL_PATHS['SCALER_PATH']
        
    print(f"Using {model_set} models (n_const={n_const}).")
    solvers = ["naps", "roundingsat", "rs_default", "scip", "mixed-bag"]
    
    total_start = time.time()
    features_array = np.array(features).reshape(1, -1)

    # Load Scaler and Scale Features
    try:
        scaler = load(S_PATH)
        features_scaled = scaler.transform(features_array)
        print(f"Scaling features: {time.time() - total_start:.4f} seconds")
    except Exception as e:
        print(f"Error scaling features: {e}")
        
    # Load Models
    try:
        model_mc = load(PATH_MC)
        model_ml = load(PATH_ML)
        print(f"Loading models: {time.time() - total_start:.4f} seconds")
    except Exception as e:
        print(f"Error loading models: {e}")

    proba_preds = model_mc.predict_proba(features_scaled)
    y_pred_opt = np.array([proba[:, 1] for proba in proba_preds]).T 
    y_pred_obj = model_ml.predict(features_scaled)

    # Min-Max Normalization
    norm_obj = (y_pred_obj - y_pred_obj.min()) / (y_pred_obj.max() - y_pred_obj.min())
    norm_opt = (y_pred_opt - y_pred_opt.min()) / (y_pred_opt.max() - y_pred_opt.min())

    # Combined score (minimize objective, maximize probability of optimality)
    combined_score = alpha * norm_obj - (1-alpha) * norm_opt
    
    id_solver = np.argmin(combined_score)
    best_solver = solvers[id_solver]

    print(f"\nTotal prediction time: {time.time() - total_start:.4f} seconds")
    
    return best_solver