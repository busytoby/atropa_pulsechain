import json
import os

def main():
    scanned_path = "tmp/scanned_qings_0xd3a7a95012edd46ea115c693b74c5e524b3dda75.json"
    status_path = "config/nonukes_qings_status.json"

    if not os.path.exists(scanned_path):
        print(f"Scanned QINGs file not found: {scanned_path}")
        return

    if not os.path.exists(status_path):
        print(f"Status file not found: {status_path}")
        return

    with open(scanned_path, "r") as f:
        scanned_data = json.load(f)

    with open(status_path, "r") as f:
        status_data = json.load(f)

    # Build a lookup of integrative address -> qing address
    qing_lookup = {}
    for item in scanned_data:
        integrative = item["integrative"].lower()
        qing = item["qing"]
        qing_lookup[integrative] = qing

    # Update status data
    updated_count = 0
    for item in status_data:
        addr = item["address"].lower()
        if addr in qing_lookup:
            item["exists"] = True
            item["qing"] = qing_lookup[addr]
            updated_count += 1
        else:
            item["exists"] = False
            item["qing"] = None

    with open(status_path, "w") as f:
        json.dump(status_data, f, indent=2)

    print(f"Successfully matched and established {updated_count} QING accumulators in nonukes_qings_status.json.")

if __name__ == "__main__":
    main()
