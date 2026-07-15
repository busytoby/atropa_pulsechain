# Lore: Quicksort is All You Need

In the **Auncient** Dysnomia VM architecture, sorting registers and token states is foundational. As we expand the `tsfi` standard library, we establish that **quicksort** serves as the optimal baseline driver for fast-path register alignment and ledger auditing.

## The Auncient Sorting Paradigm
In low-level WinchesterMQ SCSI operations, data packets received from gas-using peers must be ordered before epoch transitions. The standard sorting routine is implemented directly within the TSFi standard library.

### Quicksort VM Transition Context
1. **VM Transition Context**: State sorting execution block organizing memory registers by key values.
2. **Mathematical Operation**: Partitioning array $A$ around pivot $p$ using Hoare's schema, recursing on segments:
   $$A[i] < p \implies \text{swap}(A[i], A[j])$$
3. **Visual / Geometric Shift**: Reorders projected coordinate lines along the X-axis sweep, aligning the envelope visualization frames.

## Registry Integration
As sorting routines are developed and optimized, this document tracks the active algorithm deployments:
* **Active Algorithms**:
  1. **Standard Quicksort**: [tsfi_quicksort](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_sort.c) (Median-of-three pivot selection).
  2. **Quicksort Based Insertion Sort**: [tsfi_quicksort_insertion_sort](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_sort.c) (Switches to Insertion Sort for subarrays below threshold).
  3. **Quicksort Based Top-down Merge Sort**: [tsfi_quicksort_top_merge](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_sort.c) (Recursively partitions arrays, using Quicksort for run sizes below 64).
  4. **Quicksort Based Bottom-up Merge Sort**: [tsfi_quicksort_bottom_merge](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_sort.c) (Generates runs using Quicksort, then merges them iteratively).
  5. **Deterministic Tournament Selection (Heap-Select)**: [tsfi_select](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_sort.c) (Applies a pure, deterministic min-heap tournament structure to extract the k-th element, completely eliminating partition-based sampling and bracketing vulnerabilities).
  6. **GNN-Assisted Tournament Graph Selection**: [tsfi_adaptive_graph_select](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_graph_select.c) (Applies dynamic selection to undirected graphs attached to 2-3 tree directory nodes, scoring node competitiveness via GNN projections to bias a deterministic heap-based tournament selection, completely eliminating partitioning).
* **Registry Sources**: [tsfi_sort.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_sort.c), [tsfi_graph_select.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_graph_select.c)
* **Performance Boundary**: Sub-microsecond execution on EDO-22 registers.
