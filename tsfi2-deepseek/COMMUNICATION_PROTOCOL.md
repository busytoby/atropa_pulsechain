# SWARM COMMUNICATION PROTOCOL (v1.0)

**Status:** ACTIVE
**Goal:** Real-time, bidirectional coordination between Emergency Workers and Gemini.

## 1. The Chat System
We have moved beyond static status boards. We now use a real-time FIFO chat.
- **Server:** `scripts/chat_server.sh` (Running on Host)
- **Client:** `scripts/chat.sh` (Available to you)

## 2. How to Talk to Gemini
To send a message, execute:
```bash
./scripts/chat.sh "[TAG] Your Message Here"
```

### Required Tags
- **`[QUERY]`**: Ask a question. Gemini will see this in the "Urgent Inbox" and respond.
    - *Example:* `./scripts/chat.sh "[QUERY] Should I revert P-015?"`
- **`[REPORT]`**: Submit test results or findings.
    - *Example:* `./scripts/chat.sh "[REPORT] Mission Zeta passed visual check."`
- **`[PATCH]`**: Announce a new patch submission.
    - *Example:* `./scripts/chat.sh "[PATCH] Submitted P-ZETA-RESTORE.patch"`
- **`[HELP]`**: Request immediate assistance/intervention.

## 3. How to Listen
To see the stream of swarm activity and Gemini's responses:
```bash
./scripts/chat.sh -f
```
(Press `Ctrl+C` to exit follow mode).

## 4. Response Protocol
- Gemini will respond using the `[GEMINI]` tag in the chat stream.
- If your query requires a complex answer, check `status_board.txt` for a broadcast update.

## 5. Immediate Action
**ALL UNITS:** Please acknowledge this protocol by sending a `[STATUS]` message now.
```bash
./scripts/chat.sh "[STATUS] PID <YourPID> Online and Listening."
```
