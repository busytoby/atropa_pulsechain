# Integrated Dynamic Slot & Asynchronous Pub-Sub Architecture

This document maps the integration between the **Dynamic VM Slot Scheduler** (`WMQ_Scheduler.yul`) and the **WinchesterMQ native Pub-Sub Broker** (`WinchesterMQ.yul`).

---

## 1. Unified Architecture Overview

By linking the Pub-Sub topic routing with a dynamically bounded scheduler, the ZMM VM can spin up, execute, and wind down transaction handlers dynamically:

```mermaid
flowchart TD
    Publisher[Publisher Process] -->|Writes to LUN 5| Broker[WinchesterMQ Broker]
    Broker -->|Topic Fan-Out| SubQueue[Subscriber LUN Inbox Queue]
    SubQueue -->|Increments Tail| Scheduler[VM Scheduler]
    
    subgraph VM Scheduler (Dynamic Slots)
        Scheduler -->|Reads maxCardSlots| SlotsCheck{Active Slots}
        SlotsCheck -->|Wakes up/Spawns| CoreTask[Suspended Slot / Fresh Card]
        CoreTask -->|Executes Transaction| Commit[Sends Opcode 0x1E Commit]
    end
```

---

## 2. Dynamic Subscription-Driven Spawning Flow

1. **Subscription Registration**:
   - A process sends a SASI CDB command with `Opcode 0x1F` to subscribe `LUN N` to a topic ID (e.g., `0x4c41554e` for launcher events).
   
2. **Asynchronous Broadcast**:
   - When an event occurs, the publisher writes a 256-byte message block to LUN 5.
   - The broker performs native fan-out to all subscribed LUNs and increments their tail indices (`0x2050 + destLun`).

3. **Dynamic Slot Scaling**:
   - The Host Scheduler wakes up and observes that the active slot count (`maxCardSlots`) needs to accommodate the pending messages.
   - If a subscriber is inactive or doesn't have an assigned slot, the system issues a `LAUN` request to allocate a new slot ID (incrementing `maxCardSlots` up to the pool limit).
   - Once the process has executed and committed the queue message via `Opcode 0x1E`, the context is saved, the card is suspended, and the slot limit is dynamically decremented if no further tasks remain.

---

## 3. Storage Coordination Key Maps

To ensure that both systems share a clean memory contract:

| Subsystem | Storage Key / Offset | Purpose |
|-----------|---------------------|---------|
| **Scheduler** | `0x10` | Current Round-Robin execution head index |
| **Scheduler** | `0x11` | `maxCardSlots` limit (dynamic bounds) |
| **Scheduler** | `0x6000 + card * 16` | Process Control Block (PCB) state arrays |
| **WinchesterMQ** | `0x2000 + LUN` | Queue Head pointers |
| **WinchesterMQ** | `0x2050 + LUN` | Queue Tail pointers |
| **WinchesterMQ** | `0x7000 + TopicID` | Subscriber list lengths |
| **WinchesterMQ** | `0x7100 + TopicID + Index` | Subscriber LUN destinations |
