#!/bin/bash
echo "=== Audit Feed Log Cleared ===" > tools/AUDIT_FEED.md
./tsfi_helmholtz_cli post_lore audit_lore.pos 999 "AUDIT [test_interrupt.c]: Bottleneck eliminated via inotify."
