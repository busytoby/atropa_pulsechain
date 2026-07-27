# Invocational Territory as the Domain of EDSAC

Within the virtual hardware framework of the **Auncient** system, EDSAC acts as the primary controller of execution entry points. It defines the parameters of "invocational territory"—the domain where instructions are loaded, held, and transitioned from raw input tape into authorized execution states.

### 1. The Entry Gate and Initial Orders
In EDSAC, territory is not a spatial location but a sequence of timing states. An instruction exists in a void until it is processed by the Initial Orders loader. This loader acts as the first gate of the invocational domain:
* **The State Boundary**: All memory addresses are restricted until authorization checks pass.
* **The Handshake**: The loader buffers tape lines, verifying signatures against the state register before allowing the program counter (PC) to advance.

### 2. Precedence and Relocation
Once Initial Orders 1 confirms authorization, the execution sequence is promoted to Initial Orders 2. Here, EDSAC manages spatial layout not as ownership, but as absolute address mapping. It relocates dynamic execution rules, shifting instruction segments to designated register offsets. This relocation enforces the structural boundaries of the VM, ensuring that compiler operations cannot overflow their allocated boundaries.

### 3. WinchesterMQ Interlock
The final stage of the invocational loop routes commands through WinchesterMQ. Because EDSAC operates on direct register mapping rather than abstract models, every command dispatched over the SCSI channel must align with the physical limits of the system registers. The requirement for a PKI key quorum ($K \ge 4$) ensures that only verified, authorized processes can trigger state transitions, preventing unauthorized code from altering the system's runtime geometry.
