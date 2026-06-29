# Learning Proposal: Dynamic Contract ZMM Resolution Rule

## Rationale
During hypervisor validation and dashboard queries, contract resolution should always rely on exact contract addresses rather than names or string mutations of names to prevent incorrect mapping and align with EVM namespacing logic.

## Classification
- **Type**: Project-scoped Rule
- **Target File**: `/home/mariarahel/src/tsfi2/atropa_pulsechain/.agents/AGENTS.md`

## Proposed Changes

```diff
diff --git a/.agents/AGENTS.md b/.agents/AGENTS.md
--- a/.agents/AGENTS.md
+++ b/.agents/AGENTS.md
@@ -28,3 +28,6 @@
-## 8. Source File Size Constraints
-* **68KB File Limit:** Ensure all source files (`.c`, `.cpp`, `.h`, etc.) remain strictly under 68,000 bytes. Proactively split files into modular subcomponents as they approach this limit to preserve system design structure and modularity.
+## 8. Source File Size Constraints
+* **68KB File Limit:** Ensure all source files (`.c`, `.cpp`, `.h`, etc.) remain strictly under 68,000 bytes. Proactively split files into modular subcomponents as they approach this limit to preserve system design structure and modularity.
+
+## 9. Dynamic Contract ZMM Resolution
+* **Address-Based Resolution:** Always use contract addresses (e.g., `dynamic_<address>`) to resolve and query dynamic contract states on the ZMM VM, rather than relying on contract names or string transformations of names.
```
