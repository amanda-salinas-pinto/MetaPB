from pyscipopt import Model
import numpy as np
import math
from scipy import stats

def extract_scip_features(model: Model) -> list[float]:
    #Extracts features from an optimized SCIP model

    print("Extracting SCIP features... \n")
    features = {}

    try:
        features['num_vars'] = model.getNVars()
        features['num_constraints'] = model.getNConss()
        features['num_frac_vars'] = model.getNLPCols() - model.getNIntVars() - model.getNContVars()
        features['frac_var_ratio'] = features['num_frac_vars'] / features['num_vars'] if features['num_vars'] > 0 else 0
    except:
        features.update({'num_vars': 1e+20, 'num_constraints': 1e+20, 'num_frac_vars': 0, 'frac_var_ratio': 0})

    try:
        features['lp_obj_value'] = model.getObjVal()
        features['primal_bound'] = model.getPrimalbound()
        features['dual_bound'] = model.getDualbound()
        features['gap'] = model.getGap()
    except:
        features.update({'lp_obj_value': 1e+20, 'primal_bound': 1e+20, 'dual_bound': -1e+20, 'gap': 1e+20})

    try:
        features['feasible_leaves'] = model.getNFeasibleLeaves()
        features['lp_iterations'] = model.getNLPIterations()
        features['lp_solving_time'] = model.getSolvingTime()
        features['lp_iterations_per_var'] = features['lp_iterations'] / features['num_vars'] if features['num_vars'] > 0 else 0
        features['nsols'] = model.getNSolsFound()
    except:
        features.update({'feasible_leaves': 0, 'lp_iterations': 0, 'lp_solving_time': 5, 'lp_iterations_per_var': 0, 'nsols': 0})

    try:
        objs = np.array([var.getObj() for var in model.getVars()])
        if len(objs) > 0:
            features['obj_mean'] = np.mean(objs)
            features['obj_std'] = np.std(objs)
            features['obj_pos_ratio'] = np.sum(objs > 0) / len(objs)
            features['obj_neg_ratio'] = np.sum(objs < 0) / len(objs)
            features['obj_nonzero_ratio'] = np.count_nonzero(objs) / len(objs)
        else:
             features.update({'obj_mean': 0, 'obj_std': 0, 'obj_pos_ratio': 0, 'obj_neg_ratio': 0, 'obj_nonzero_ratio': 0})
    except:
        features.update({'obj_mean': 0, 'obj_std': 0, 'obj_pos_ratio': 0, 'obj_neg_ratio': 0, 'obj_nonzero_ratio': 0})

    try:
        cons = model.getConss()
        farkas_values = np.array([model.getDualfarkasLinear(con) for con in cons if con.isLinear()])
        if len(farkas_values) > 0:
            features['farkas_mean'] = np.mean(farkas_values)
            features['farkas_std'] = np.std(farkas_values)
            features['farkas_min'] = np.min(farkas_values)
        else:
             features.update({'farkas_mean': 0, 'farkas_std': 0, 'farkas_min': 0})
    except:
        features.update({'farkas_mean': 0, 'farkas_std': 0, 'farkas_min': 0})

    try:
        activities = np.array([model.getRowActivity(row) for row in model.getLPRowsData()])
        features['activity_mean'] = np.mean(activities) if len(activities) > 0 else 0
    except:
        features['activity_mean'] = 0

    selected_keys = [
        "lp_obj_value", "dual_bound", "obj_mean", "obj_std", "obj_pos_ratio",
        "obj_neg_ratio", "obj_nonzero_ratio", "gap", "feasible_leaves",
        "lp_iterations", "lp_iterations_per_var", "nsols", "activity_mean"
    ]

    return [features[k] for k in selected_keys]