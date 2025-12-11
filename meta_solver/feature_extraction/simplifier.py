from pyscipopt import Model
import time
import math
import os
from scip_extractor import extract_scip_features
from constants import SOLVED_BY_SCIP, INFEASIBLE

def simplify_with_scip(instance_path: str, output_path: str, time_limit: int = 10) -> Model | str:
    print("Calling SCIP with presolver...\n")

    start_read = time.time()
    model = Model()
    model.readProblem(instance_path)
    read_time = time.time() - start_read

    # SCIP parameters
    model.setParam("limits/time", time_limit)
    model.hideOutput(quiet=True)
    model.setParam("display/verblevel", 0)

    start_opt = time.time()
    model.optimize()
    opt_time = time.time() - start_opt

    status = model.getStatus()
    print("[SCIP STATUS]", status)

    if status == "optimal":
        return SOLVED_BY_SCIP

    if status == "infeasible":
        return INFEASIBLE

    print("Writing modified instance...\n")
    start_write = time.time()

    n_vars = model.getNVars()
    n_cons = model.getNConss()
    constraint = ""

    # Add objective constraint if a solution was found
    try:
        n_sols = model.getNSolsFound()
        if n_sols > 0:
            best_obj = model.getObjVal(original=True)
            obj = model.getObjective()
            
            # Construct the objective constraint
            for var, coeff in obj.items():
                neg_coeff = int(round(-coeff))
                constraint += f"{'+' if neg_coeff >= 0 else ''}{neg_coeff} {var} "
                
            rhs = int(math.floor(-best_obj))  
            constraint += f">= {rhs} ;\n"
    except Exception as e:
        print(f"Warning: Failed to create objective constraint: {e}")
        
    # Write the simplified problem
    with open(output_path, 'w') as f:
        model.writeProblem(output_path)
        
    with open(output_path, 'r') as original:
        original_content = original.read()

    with open(output_path, 'w') as modified:
        modified.write(f"* #variable= {n_vars} #constraint= {n_cons} #equal= 0 intsize= 25\n") #TODO: Update intsize
        modified.write("*\n")
        modified.write(original_content)

    # Append the objective constraint
    if constraint != "":
        with open(output_path, 'a') as f:
            f.write(constraint)

    write_time = time.time() - start_write

    print(f"[READ TIME] {read_time:.4f} s")
    print(f"[OPTIMIZE TIME] {opt_time:.4f} s")
    print(f"[WRITE TIME] {write_time:.4f} s")

    return model