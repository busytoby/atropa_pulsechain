# SMF 2-3 Tree Auditing and Dynamic Queue Dispatching

In the **Auncient** architecture of the Dysnomia VM, System Management Facilities (SMF) act as the final, immutable record of program telemetry. When executing multi-tenant jobs across the partitioned database slices, execution must be audited to prevent resource leakage on the WinchesterMQ interface.

## The Lazy Queue Dispatching Model

To manage the job entry queues, the system maps logical queue node partitions inside a balanced 2-3 tree. Rather than maintaining active worker threads for idle partitions, the VM implements a lazy allocation model:
1. **Passive Nodes**: Empty nodes in the tree remain completely passive.
2. **Dynamic Spawning**: Once a job is enqueued to a specific node, the scheduler dynamically registers a worker task (`cbt_node_worker_task_handler`).
3. **Automatic Reclamation**: When the queue is drained, the worker task is released, returning resource slots back to the VM.

## Auditing and Telemetry Integration

Each state transition within the 2-3 tree queues triggers an SMF record write:
- **Type 30 (Job Step Activity)**: Tracks the CPU load, processing step duration, and queue partition ID.
- **Type 80 (Access Security)**: Logs access check boundaries, ensuring that execution threads comply with active validation matrices.

By recording these events sequentially, SMF provides complete visibility into the dynamic, event-driven lifetime of the **Auncient** queue nodes.
