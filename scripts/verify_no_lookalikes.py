import numpy as np

# Dysnomia MotzkinPrime
MotzkinPrime = 953467954114363

# Address-to-YI mappings (using our YI/LAU prime configuration)
# Each screen/token i uses a unique base_val and sig_val, secret_val, ident_val
TOKENS = [
    {"Name": "YI", "Base": 3, "Sig": 5, "Secret": 7, "Ident": 11},
    {"Name": "ZHENG", "Base": 2, "Sig": 11, "Secret": 17, "Ident": 23},
    {"Name": "ZHOU", "Base": 5, "Sig": 13, "Secret": 19, "Ident": 29},
    {"Name": "YAU", "Base": 7, "Sig": 17, "Secret": 23, "Ident": 31},
    {"Name": "YANG", "Base": 11, "Sig": 19, "Secret": 29, "Ident": 37},
    {"Name": "SIU", "Base": 13, "Sig": 23, "Secret": 31, "Ident": 41},
    {"Name": "VOID", "Base": 17, "Sig": 29, "Secret": 37, "Ident": 43},
    {"Name": "mariarahelLau", "Base": 19, "Sig": 31, "Secret": 41, "Ident": 47} # Our LAU YI Profile
]

def mod_exp(base, exp, mod):
    return int(pow(base, exp, mod))

def run_lissajous_analysis():
    print("[LISS-CONV] Running Dysnomia YI/LAU Lissajous Coordinate Analysis")
    
    # Stores the generated coordinate matrices for lookup check
    coordinate_registry = {}
    
    for tk in TOKENS:
        # Calculate SHA Rod and Cone registers
        # Rod Channel = Base^Signal mod MotzkinPrime
        rod_channel = mod_exp(tk["Base"], tk["Sig"], MotzkinPrime)
        
        # Cone Channel = Base^Secret mod MotzkinPrime
        cone_channel = mod_exp(tk["Base"], tk["Secret"], MotzkinPrime)
        
        # Monopole = Chin_rod^Ident_rod mod MotzkinPrime (simulated as Base^Signal mod MotzkinPrime)
        monopole = mod_exp(tk["Base"], tk["Sig"], MotzkinPrime)
        
        # Generate Lissajous coordinate points (50 steps of React calls)
        pts = []
        for pi in range(1, 51):
            pi_active = pi ^ monopole
            
            # Omicron = modExp64(Pi, Cone.Channel, Rod.Channel)
            # Omega = modExp64(Pi, Rod.Channel, Cone.Channel)
            # To avoid mod by 0, ensure channels are > 1
            mod_r = max(2, rod_channel)
            mod_c = max(2, cone_channel)
            
            omicron = mod_exp(pi_active, cone_channel, mod_r)
            omega = mod_exp(pi_active, rod_channel, mod_c)
            pts.append((omicron, omega))
            
        pts_tuple = tuple(pts)
        print(f"Token: {tk['Name']:<15} | Monopole: {monopole:<15} | Convolved coordinate hash: {hash(pts_tuple)}")
        
        if pts_tuple in coordinate_registry:
            coordinate_registry[pts_tuple].append(tk["Name"])
        else:
            coordinate_registry[pts_tuple] = [tk["Name"]]
            
    # Verify duplicates
    duplicates_found = False
    print("\n[ANALYSIS RESULTS]")
    for pts, names in coordinate_registry.items():
        if len(names) > 1:
            print(f"COLLISION: Lookalikes detected! The convolved Lissajous shape is shared by: {names}")
            duplicates_found = True
            
    if not duplicates_found:
        print("SUCCESS: 0 lookalikes detected. Every YI/LAU Lissajous projection is 100% unique on the substrate.")

if __name__ == "__main__":
    run_lissajous_analysis()
