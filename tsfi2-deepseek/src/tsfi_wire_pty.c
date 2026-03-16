#define _XOPEN_SOURCE 600
#include "tsfi_wire_pty.h"
#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_io.h"
#include "lau_audit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/mman.h>
#include <stdatomic.h>
#include <stdarg.h>

static void pty_step_read_stdout(LauWirePty *pty) {
    if (!pty->is_active || pty->master_fd < 0 || !pty->log_stdout.buffer || !pty->log_stdout.head || !pty->log_stdout.tail) return;
    
    char *buf = pty->log_stdout.buffer;
    _Atomic uint32_t *head_ptr = (_Atomic uint32_t *)pty->log_stdout.head;
    _Atomic uint32_t *tail_ptr = (_Atomic uint32_t *)pty->log_stdout.tail;

    uint32_t head = atomic_load_explicit(head_ptr, memory_order_relaxed);
    uint32_t tail = atomic_load_explicit(tail_ptr, memory_order_acquire);
    
    char read_buf[4096];
    ssize_t n = read(pty->master_fd, read_buf, sizeof(read_buf));
    if (n > 0) {
        for (ssize_t i = 0; i < n; i++) {
            uint32_t next = (head + 1) % 4096;
            if (next != tail) {
                buf[head] = read_buf[i];
                head = next;
            } else break;
        }
        atomic_store_explicit(head_ptr, head, memory_order_release);
    } else if (n < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) pty->is_active = false;
    } else {
        pty->is_active = false;
    }
}

static void pty_step_write_stdin(LauWirePty *pty) {
    if (!pty->is_active || pty->master_fd < 0 || !pty->log_stdin.buffer || !pty->log_stdin.head || !pty->log_stdin.tail) return;
    
    char *buf = pty->log_stdin.buffer;
    _Atomic uint32_t *head_ptr = (_Atomic uint32_t *)pty->log_stdin.head;
    _Atomic uint32_t *tail_ptr = (_Atomic uint32_t *)pty->log_stdin.tail;

    uint32_t head = atomic_load_explicit(head_ptr, memory_order_acquire);
    uint32_t tail = atomic_load_explicit(tail_ptr, memory_order_relaxed);
    
    if (tail != head) {
        if (head > tail) {
            ssize_t n = write(pty->master_fd, &buf[tail], head - tail);
            if (n > 0) tail = (tail + n) % 4096;
        } else {
            // Wrap case
            ssize_t n1 = write(pty->master_fd, &buf[tail], 4096 - tail);
            if (n1 > 0) {
                tail = (tail + n1) % 4096;
                if (tail == 0 && head > 0) {
                    ssize_t n2 = write(pty->master_fd, &buf[0], head);
                    if (n2 > 0) tail = n2;
                }
            }
        }
        atomic_store_explicit(tail_ptr, tail, memory_order_release);
    }
}

static void pty_step_check_child(LauWirePty *pty) {
    if (pty->child_pid > 0) {
        int status;
        pid_t wp = waitpid(pty->child_pid, &status, WNOHANG);
        if (wp > 0) {
            tsfi_io_printf(stdout, "[DEBUG] PTY Child %d exited with status %d\n", pty->child_pid, status);
            
            char lbuf[128];
            snprintf(lbuf, sizeof(lbuf), "PTY_EXIT: PID=%d STATUS=%d", pty->child_pid, status);
            lau_log_status(lbuf);

            pty->is_active = false; pty->child_pid = -1;
        } else if (wp < 0 && errno == ECHILD) {
            tsfi_io_printf(stdout, "[DEBUG] PTY Child %d lost (ECHILD)\n", pty->child_pid);
            
            char lbuf[128];
            snprintf(lbuf, sizeof(lbuf), "PTY_LOST: PID=%d", pty->child_pid);
            lau_log_status(lbuf);

            pty->is_active = false; pty->child_pid = -1;
        }
    }
}

void tsfi_wire_pty_step(LauWirePty *pty) {
    if (!pty) return;
    pty_step_read_stdout(pty);
    pty_step_write_stdin(pty);
    pty_step_check_child(pty);
}

static bool default_logic_state(void *obj) { return ((LauWirePty*)obj)->is_active; }
static void default_logic_epoch(void *obj) { tsfi_wire_pty_step((LauWirePty*)obj); }
static void default_logic_directive(void *obj, char *cmd) {
    LauWirePty *pty = (LauWirePty *)obj;
    if (strcmp(cmd, "STOP") == 0) { pty->is_active = false; if (pty->child_pid > 0) kill(pty->child_pid, SIGTERM); }
}

LauWirePty* tsfi_wire_pty_fork(const char *cmd) {
    LauWirePty *pty = (LauWirePty*)lau_malloc_wired(sizeof(LauWirePty));
    if (!pty) return NULL;
    memset(pty, 0, sizeof(LauWirePty));
    pty->is_active = true;
    int mfd = posix_openpt(O_RDWR | O_NOCTTY);
    if (mfd < 0 || grantpt(mfd) != 0 || unlockpt(mfd) != 0) { if(mfd>=0) close(mfd); lau_free(pty); return NULL; }
    char *sname = ptsname(mfd); if (!sname) { close(mfd); lau_free(pty); return NULL; }
    strncpy(pty->pts_name, sname, sizeof(pty->pts_name)-1); pty->master_fd = mfd;
    pid_t pid = fork();
    if (pid < 0) { close(mfd); lau_free(pty); return NULL; }
    else if (pid == 0) {
        close(mfd); setsid(); int sfd = open(sname, O_RDWR); if (sfd < 0) _exit(1);
        struct termios t; tcgetattr(sfd, &t); t.c_lflag |= (ECHO | ICANON | ISIG); tcsetattr(sfd, TCSANOW, &t);
        ioctl(sfd, TIOCSCTTY, 0); dup2(sfd, 0); dup2(sfd, 1); dup2(sfd, 2); if(sfd>2) close(sfd);
        setenv("TERM", "xterm", 1);
        setenv("LANG", "en_US.UTF-8", 1);
        setenv("NO_COLOR", "1", 1);
        setenv("HOME", "/home/mariarahel", 1);
        setenv("USER", "mariarahel", 1);
        setenv("PATH", "/usr/local/bin:/usr/bin:/bin", 1);
        setenv("TSFI_SESSION_ACTIVE", "1", 1);
        char *g_key = getenv("GEMINI_API_KEY");
        if (g_key) setenv("GOOGLE_API_KEY", g_key, 1);
        execl("/bin/sh", "sh", "-c", cmd, (char*)NULL); _exit(1);
    }
    pty->child_pid = pid; fcntl(mfd, F_SETFL, fcntl(mfd, F_GETFL) | O_NONBLOCK);

    char buf[256];
    snprintf(buf, sizeof(buf), "PTY_FORK: PID=%d CMD=\"%s\"", pid, cmd);
    lau_log_status(buf);

    LauSystemHeader *h = (LauSystemHeader *)((char *)pty - offsetof(LauSystemHeader, payload));
    h->logic_epoch = (void (*)(int*))default_logic_epoch; h->logic_state = (bool (*)(void*))default_logic_state;
    h->logic_directive = (void (*)(int*, char*))default_logic_directive;
    if (!h->proxy) h->proxy = ThunkProxy_create();
    if (h->proxy) {
        ThunkProxy *p = (ThunkProxy*)h->proxy;
        void* entry = ThunkProxy_emit_safety_chain(p, h->logic_state, (void(*)(void*))h->logic_epoch, pty);
        if (entry) { pty->step_safety_epoch = (void(*)(void))entry; pty->step_executor_directive = (void(*)(char*))ThunkProxy_emit_mixed(p, (void*)h->logic_directive, pty); }
        mprotect(p->thunk_pool, p->pool_size, PROT_READ | PROT_EXEC); __builtin___clear_cache((char*)p->thunk_pool, (char*)p->thunk_cursor);
    }
    return pty;
}

ssize_t tsfi_wire_pty_write(LauWirePty *pty, const char *buf, size_t n) { if (!pty || !pty->is_active || pty->master_fd < 0) return -1; return write(pty->master_fd, buf, n); }
int tsfi_printf(LauWirePty *pty, const char *format, ...) {
    char buf[4096]; va_list a; va_start(a, format); int n = vsnprintf(buf, sizeof(buf), format, a); va_end(a);
    if (n > 0) {
        ssize_t _nw = write((pty && pty->is_active && pty->master_fd >= 0) ? pty->master_fd : 1, buf, n);
        (void)_nw;
    }
    return n;
}
