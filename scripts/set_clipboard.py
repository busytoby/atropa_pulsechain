import sys
import os
import signal
import psutil

def kill_previous_instances():
    current_pid = os.getpid()
    for proc in psutil.process_iter(['pid', 'name', 'cmdline']):
        try:
            cmd = proc.info['cmdline']
            if cmd and any('set_clipboard.py' in part for part in cmd):
                pid = proc.info['pid']
                if pid != current_pid and pid != os.getppid():
                    os.kill(pid, signal.SIGTERM)
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            pass

if __name__ == "__main__":
    kill_previous_instances()
    text = sys.stdin.read()
    
    try:
        pid = os.fork()
        if pid > 0:
            print("[CLIPBOARD] Successfully synced text to host clipboard.")
            sys.exit(0)
    except OSError as e:
        print(f"[CLIPBOARD ERR] Fork failed: {e}", file=sys.stderr)
        sys.exit(1)
        
    os.setsid()
    
    # Properly redirect OS-level file descriptors 0, 1, 2 to /dev/null
    try:
        devnull = os.open(os.devnull, os.O_RDWR)
        os.dup2(devnull, 0)
        os.dup2(devnull, 1)
        os.dup2(devnull, 2)
        os.close(devnull)
    except Exception:
        pass
        
    # Import and initialize GTK only in the child process to avoid fork deadlocks
    import gi
    gi.require_version('Gtk', '3.0')
    from gi.repository import Gtk, Gdk
    
    try:
        clipboard = Gtk.Clipboard.get(Gdk.SELECTION_CLIPBOARD)
        clipboard.set_text(text, -1)
        
        primary = Gtk.Clipboard.get(Gdk.SELECTION_PRIMARY)
        primary.set_text(text, -1)
        
        Gtk.main()
    except Exception:
        pass
