import subprocess
import time
import statistics
import re

profiles = {
    "Default (Baseline)": [50.0, 0.9999, 500000, 20, 50000, 0.25],
    "Aggressive Restart (Trillium Candidate 1)": [50.0, 0.999, 100000, 50, 20000, 0.25],
    "Deep Exploration (Local Candidate 1)": [100.0, 0.99999, 2000000, 5, 200000, 0.5],
    "Gentle Cooling (Local Candidate 2)": [50.0, 0.99995, 1000000, 10, 100000, 0.25],
    "High Temp Fast Decay (Trillium Candidate 2)": [100.0, 0.995, 50000, 100, 10000, 0.1],
}

n = 16 # Small enough to solve relatively quickly
runs_per_profile = 5

print(f"=== SA Parameter Tuning (n={n}, {runs_per_profile} runs per profile) ===")

best_time = float('inf')
best_profile = ""

results = {}

for name, params in profiles.items():
    print(f"\nTesting Profile: {name}")
    print(f"Params: init_temp={params[0]}, cooling={params[1]}, iters={params[2]}, restarts={params[3]}, reheat_thresh={params[4]}, reheat_ratio={params[5]}")
    
    times = []
    for i in range(runs_per_profile):
        seed_offset = i * 10 
        cmd = [
            "./wz_sa_tune", 
            str(n), 
            str(seed_offset),
            str(params[0]),
            str(params[1]),
            str(params[2]),
            str(params[3]),
            str(params[4]),
            str(params[5])
        ]
        
        start_time = time.time()
        # Run process and capture output
        try:
            # We timeout at 60 seconds per run just in case a profile is absolutely terrible
            process = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
            
            # Find time in output
            match = re.search(r"Time:\s*([\d\.]+)s", process.stdout)
            if match:
                run_time = float(match.group(1))
            else:
                # If we didn't find "Time:", maybe it hit the exact end. Just use wall clock.
                run_time = time.time() - start_time
        except subprocess.TimeoutExpired:
            print(f"  Run {i+1}: TIMEOUT (>30s)")
            run_time = 30.0
            
        print(f"  Run {i+1}: {run_time:.2f}s")
        times.append(run_time)
        
    avg_time = statistics.mean(times)
    print(f"-> Average Time: {avg_time:.2f}s")
    results[name] = avg_time
    
    if avg_time < best_time:
        best_time = avg_time
        best_profile = name

print("\n=== Tuning Summary ===")
for name, avg in sorted(results.items(), key=lambda item: item[1]):
    print(f"{name}: {avg:.2f}s")

print(f"\nRecommended optimal profile overall: {best_profile}")
