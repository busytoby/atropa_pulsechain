# Proposed Plan: Instantiating the Simulation Federal Worker

This plan outlines the code changes required to instantiate the `AuncientFederalWorkerCharacter` as a first-class participant in the system registry, allowing the user to modify the integration rules.

---

## 1. Material-to-Cloth Bridge Additions
We will extend [src/auncient_cloth_material_bridge.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/src/auncient_cloth_material_bridge.c) to map the new worker character:

```diff
 void auncient_bridge_entity_to_ssa(const char *entity_name, char *ssn_out, char *site_out, int max_len) {
     if (!entity_name || !ssn_out || !site_out) return;
 
     if (strcmp(entity_name, "TeddyBearSkelCharacter") == 0) {
         strncpy(ssn_out, "006-12-3456", 12);
         strncpy(site_out, "Delaware", max_len - 1);
+    } else if (strcmp(entity_name, "AuncientFederalWorkerCharacter") == 0) {
+        strncpy(ssn_out, "009-12-3456", 12);
+        strncpy(site_out, "Vermont", max_len - 1);
     } else {
         strncpy(ssn_out, "000-00-0000", 12);
         strncpy(site_out, "Unknown", max_len - 1);
     }
 }
```

---

## 2. AUTODIN Startup Verification Extensions
We will update the startup checks in [src/auncient_timeline_autodin.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/src/auncient_timeline_autodin.c) to verify the new worker character at boot time:

```diff
 bool auncient_autodin_verify_system_start(const HoganAccount *accounts, int count) {
     bool has_bank = false;
     bool has_ssa = false;
     bool has_teddy_bear = false;
+    bool has_worker = false;
 
     for (int i = 0; i < count; i++) {
         if (accounts[i].account_id == 555) { // Singular Teddy Bear
             has_teddy_bear = true;
         }
+        if (accounts[i].account_id == 888) { // Simulation Federal Worker
+            has_worker = true;
+        }
         // Bank and SSA indicators
         if (accounts[i].account_id == 1) has_bank = true;
         if (accounts[i].account_id == 2) has_ssa = true;
     }
 
-    return (has_bank && has_ssa && has_teddy_bear);
+    return (has_bank && has_ssa && has_teddy_bear && has_worker);
 }
```

---

## 3. Unit Test Verification
We will add a validation case to `tests/test_auncient_federal_worker.c` to test the official registration of `AuncientFederalWorkerCharacter` to Hogan Bank:

```c
    // Verify singular entity registration for the Federal Worker
    char worker_ssn[16];
    char worker_site[32];
    auncient_bridge_entity_to_ssa("AuncientFederalWorkerCharacter", worker_ssn, worker_site, sizeof(worker_site));
    assert(strcmp(worker_ssn, "009-12-3456") == 0);
    assert(strcmp(worker_site, "Vermont") == 0);
    printf("   ✓ Singular Federal Worker Entity SSN and Site mapping verified.\n");
```
