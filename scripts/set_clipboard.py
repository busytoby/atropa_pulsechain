import sys
import tkinter as tk

def set_clipboard(text):
    try:
        r = tk.Tk()
        r.withdraw()
        r.clipboard_clear()
        r.clipboard_append(text)
        r.update()
        r.destroy()
        print("[CLIPBOARD] Successfully synced text to host clipboard.")
    except Exception as e:
        print(f"[CLIPBOARD ERR] Failed to set host clipboard: {e}", file=sys.stderr)

if __name__ == "__main__":
    text = sys.stdin.read()
    set_clipboard(text)
