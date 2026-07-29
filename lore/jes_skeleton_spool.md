# Lore: JCL Skeleton Templates and JES/HASP Spooling Integration

In the architecture of the Auncient VM, the execution pathway for automated batch processing relies on the translation of static templates into dynamic spool entries. This translation bridges the declarative template definitions of the ISPF environment with the active scheduler queues of the Job Entry Subsystem (JES) and the HASP dispatcher.

## The Expansion Pathway

1. **JCL SKELETON Structure**: Skeletons serve as static blueprints containing step definitions, dataset allocations, and variable placeholders (prefixed with `&`). They reside in library partitions, awaiting parameter bindings.
2. **JES Translation Engine**: When triggered by a submission event, JES loads the SKELETON, evaluates active environment variables, performs substitution, and expands the template into a flat, 80-byte card-image JCL stream.
3. **HASP Spool Injection**: The expanded JCL stream is injected directly into the HASP reader queue. HASP assigns a job identifier, writes the record block sequence to the spool volumes, and updates scheduling priority registers, coordinating step initiation.
