import numpy as np
from simulate_fet_discharge import FETSoftBodyDischarge

class FETDischargeClassifier:
    """
    Analyzes physical state trajectories (node positions, resistance, temperature)
    from a simulated FET hardware structure and classifies the active state:
    - CHARGING: Electrostatic expansion phase (low channel resistance).
    - DISCHARGING: Active Verlet mass-spring decay and current flow (high resistance, hot junction).
    - EQUILIBRIUM: Discharged relaxation (high resistance, cooled junction).
    """
    def __init__(self, ambient_temp=293.15):
        self.ambient_temp = ambient_temp
        
    def classify_state(self, pos_history, temp_history, r_ds_history):
        if len(pos_history) < 2:
            return "UNKNOWN"
            
        current_r_ds = r_ds_history[-1]
        current_temp = temp_history[-1]
        
        # Classification criteria based on channel resistance and thermal state
        if current_r_ds < 3.0:
            return "CHARGING"
        elif current_temp > self.ambient_temp + 0.15:
            return "DISCHARGING"
        else:
            return "EQUILIBRIUM"

def run_fet_classification_test():
    print("[FET-CLASSIFIER] Initializing diagnostic class validation...")
    
    # Using standard stable damping (2.0)
    fet = FETSoftBodyDischarge(num_nodes=10, k_spring_base=100.0, damping_base=2.0)
    classifier = FETDischargeClassifier(ambient_temp=293.15)
    
    # Simulate a charging phase (gate_voltage = 3.3V)
    pos_hist = []
    temp_hist = []
    r_ds_hist = []
    
    for _ in range(10):
        pos, r_ds, temp = fet.tick(gate_voltage=3.3, source_voltage=0.0, drain_voltage=3.3)
        pos_hist.append(pos)
        temp_hist.append(temp)
        r_ds_hist.append(r_ds)
        
    state_charging = classifier.classify_state(pos_hist, temp_hist, r_ds_hist)
    print(f"Phase 1: Charging Gate (3.3V)  | Classified State: {state_charging}")
    
    # Simulate active discharge start (gate_voltage = 0V, current heats the junction)
    for _ in range(100):
        pos, r_ds, temp = fet.tick(gate_voltage=0.0, source_voltage=0.0, drain_voltage=3.3)
        pos_hist.append(pos)
        temp_hist.append(temp)
        r_ds_hist.append(r_ds)
        
    state_discharging = classifier.classify_state(pos_hist[-10:], temp_hist[-10:], r_ds_hist[-10:])
    print(f"Phase 2: Discharging Gate (0.0V) | Classified State: {state_discharging}")
    
    # Simulate long discharge to equilibrium (cooling past 3.77 time constants: 20000 ticks)
    for _ in range(20000):
        pos, r_ds, temp = fet.tick(gate_voltage=0.0, source_voltage=0.0, drain_voltage=0.0)
        pos_hist.append(pos)
        temp_hist.append(temp)
        r_ds_hist.append(r_ds)
        
    state_equilibrium = classifier.classify_state(pos_hist[-10:], temp_hist[-10:], r_ds_hist[-10:])
    print(f"Phase 3: Equilibrium Decay     | Classified State: {state_equilibrium}")
    
    # Verify classification logic matches expected transitions
    assert state_charging == "CHARGING"
    assert state_discharging == "DISCHARGING"
    assert state_equilibrium == "EQUILIBRIUM"
    print("[SUCCESS] FET Classifier diagnostic validation passed.")

if __name__ == "__main__":
    run_fet_classification_test()
