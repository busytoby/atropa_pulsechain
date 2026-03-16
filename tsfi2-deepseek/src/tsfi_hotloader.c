#include "tsfi_hotloader.h"
#include "lau_thunk.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <sys/wait.h>
#include <unistd.h>

static void *g_current_handle = NULL;

int tsfi_compile_compute_shader(const char *source_path, const char *output_spv) {
    tsfi_io_printf(stderr, "[HOTLOADER] Compiling Shader: %s -> %s\n", source_path, output_spv);
    
    pid_t pid = fork();
    if (pid == 0) {
        execlp("glslangValidator", "glslangValidator", "-V", source_path, "-o", output_spv, (char*)NULL);
        perror("[HOTLOADER] execlp glslangValidator failed");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        return (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? 0 : -1;
    }
}

int tsfi_compile_plugin(const char *source_path, const char *output_so) {
    tsfi_io_printf(stderr, "[HOTLOADER] Compiling: %s -> %s\n", source_path, output_so); tsfi_io_flush(stderr);

    pid_t pid = fork();
    if (pid == -1) {
        perror("[HOTLOADER] fork failed");
        return -1;
    } else if (pid == 0) {
        // Child Process
        const char *compiler = getenv("CC");
        if (!compiler) compiler = "gcc";

        #ifdef TSFI_DEBUG
        const char *debug_flags[] = {"-g", "-fsanitize=undefined", "-fno-sanitize-recover=all"};
        int num_debug_flags = 3;
        #else
        const char **debug_flags = NULL;
        int num_debug_flags = 0;
        #endif

        char *args[64];
        int i = 0;
        args[i++] = (char*)compiler;
        for(int j=0; j<num_debug_flags; j++) args[i++] = (char*)debug_flags[j];

        args[i++] = "-shared";
        args[i++] = "-fPIC";
        args[i++] = "-D_POSIX_C_SOURCE=200809L";
        args[i++] = "-Werror";
        args[i++] = "-Wall";
        args[i++] = "-Wextra";
        args[i++] = "-mavx512f";
        args[i++] = "-mavx512bw";
        args[i++] = "-Iinc";
        args[i++] = "-Iplugins";
        args[i++] = "-Iplugins/vulkan";
        args[i++] = "-Iplugins/window_inc";
        args[i++] = "-I.";
        args[i++] = "-o";
        args[i++] = (char*)output_so;

        if (strstr(source_path, "tsfi_vulkan") != NULL && strstr(source_path, "display") == NULL) {
            if (strcmp(source_path, "plugins/tsfi_vulkan.c") == 0) {
                args[i++] = "plugins/tsfi_vulkan.c";
            } else {
                args[i++] = "plugins/vulkan/vulkan_main.c";
                args[i++] = "plugins/vulkan/vulkan_logic.c";
                args[i++] = "plugins/vulkan/vulkan_render.c";
                args[i++] = "plugins/vulkan/vulkan_xdg.c";
                args[i++] = "plugins/vulkan/vulkan_input.c";
                args[i++] = "plugins/vulkan/vulkan_dmabuf.c";
                args[i++] = "plugins/tsfi_staging.c";
                args[i++] = "plugins/vulkan_init.c";
            }
            args[i++] = "plugins/linux-dmabuf-v1-protocol.c";
            args[i++] = "plugins/linux-explicit-synchronization-unstable-v1-protocol.c";
            args[i++] = "plugins/xdg-shell-protocol.c";
            args[i++] = "plugins/window_src/tsfi_input.c";
            args[i++] = "src/tsfi_vision.c";
            args[i++] = "src/tsfi_wiring.c";
            args[i++] = "src/tsfi_wire_firmware.c";
            args[i++] = "src/firmware/LauWireFirmware_rtl.c";
            args[i++] = "src/lau_thunk.c";
            args[i++] = "-lwayland-client";
            args[i++] = "-lvulkan";
            args[i++] = "-ldl";
        } else if (strstr(source_path, "display") != NULL || strstr(source_path, "vulkan_display") != NULL) {
            args[i++] = "plugins/vulkan/backend_display.c";
            args[i++] = "plugins/vulkan/input_vty.c";
            args[i++] = "plugins/vulkan/vulkan_logic.c";
            args[i++] = "plugins/vulkan/vulkan_render.c";
            args[i++] = "plugins/tsfi_staging.c";
            args[i++] = "plugins/vulkan_init.c";
            args[i++] = "plugins/window_src/tsfi_input.c";
            args[i++] = "src/tsfi_vision.c";
            args[i++] = "src/tsfi_wiring.c";
            args[i++] = "src/tsfi_wire_firmware.c";
            args[i++] = "src/firmware/LauWireFirmware_rtl.c";
            args[i++] = "src/lau_thunk.c";
            args[i++] = "-lvulkan";
            args[i++] = "-ldl";
        } else {
            args[i++] = (char*)source_path;
        }
        
        args[i++] = NULL;
        execvp(compiler, args);
        perror("[HOTLOADER] execvp failed");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            tsfi_io_printf(stderr, "[HOTLOADER] Successfully compiled: %s\n", source_path); tsfi_io_flush(stderr);
            return 0;
        } else {
            tsfi_io_printf(stderr, "[HOTLOADER] Compilation failed (Exit: %d)\n", WEXITSTATUS(status));
            return -1;
        }
    }
}



#include <pthread.h>
#include "tsfi_zhao.h"

// Context container for the Fourier synthesis bridging
typedef struct {
    char thunk_path[256];
    char symbol_name[256];
    _Atomic(void*)* target_ptr;
} HotloadFourierTask;

// The External Hotloader Background Task (Mu timeline)
static void* background_compiler_thread(void* arg) {
    HotloadFourierTask* task = (HotloadFourierTask*)arg;
    
    struct stat last_stat;
    if (stat(task->thunk_path, &last_stat) != 0) {
        last_stat.st_mtime = 0; // File might not exist yet
    }
    time_t last_mtime = last_stat.st_mtime;

    // Do an initial compile if it exists
    if (last_mtime > 0) {
        void* compiled_func = tsfi_hotload_thunk(task->thunk_path, task->symbol_name);
        if (compiled_func) {
            struct SwapEnv {
                _Atomic(void*)* target;
                void* new_func;
            };
            struct SwapEnv* env = malloc(sizeof(struct SwapEnv));
            env->target = task->target_ptr;
            env->new_func = compiled_func;
            
            void lockfree_swap(void* ctx) {
                struct SwapEnv* e = (struct SwapEnv*)ctx;
                atomic_store_explicit(e->target, e->new_func, memory_order_release);
                free(e);
            }
            tsfi_zhao_submit(lockfree_swap, env);
        }
    }

    // Enter persistent hot-reload loop
    while(1) {
        sleep(1); // Poll interval
        
        struct stat current_stat;
        if (stat(task->thunk_path, &current_stat) == 0) {
            if (current_stat.st_mtime > last_mtime) {
                last_mtime = current_stat.st_mtime;
                
                tsfi_io_printf(stderr, "[HOTLOADER] Modification detected on %s, hot-recompiling...\n", task->thunk_path);
                
                void* compiled_func = tsfi_hotload_thunk(task->thunk_path, task->symbol_name);
                
                if (compiled_func) {
                    struct SwapEnv {
                        _Atomic(void*)* target;
                        void* new_func;
                    };
                    struct SwapEnv* env = malloc(sizeof(struct SwapEnv));
                    env->target = task->target_ptr;
                    env->new_func = compiled_func;
                    
                    void lockfree_swap(void* ctx) {
                        struct SwapEnv* e = (struct SwapEnv*)ctx;
                        atomic_store_explicit(e->target, e->new_func, memory_order_release);
                        free(e);
                    }
                    
                    tsfi_zhao_submit(lockfree_swap, env);
                }
            }
        }
    }
    
    free(task);
    return NULL;
}

void tsfi_hotreload_poll_async(const char* thunk_path, const char* symbol_name, _Atomic(void*)* target_ptr) {
    HotloadFourierTask* task = malloc(sizeof(HotloadFourierTask));
    strncpy(task->thunk_path, thunk_path, 255);
    strncpy(task->symbol_name, symbol_name, 255);
    task->target_ptr = target_ptr;
    
    pthread_t thread_id;
    // Spawns a fully isolated POSIX thread to handle the OS latency
    if (pthread_create(&thread_id, NULL, background_compiler_thread, task) != 0) {
        tsfi_io_printf(stderr, "[HOTLOADER] Failed to spawn background compiler geometry.\n");
        free(task);
        return;
    }
    pthread_detach(thread_id); // Allow autonomous execution and termination
}

void* tsfi_hotload_thunk(const char* thunk_path, const char* symbol_name) {
    char so_path[2048];
    
    static _Atomic int generation_counter = 0;
    int current_gen = atomic_fetch_add(&generation_counter, 1);
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        snprintf(so_path, sizeof(so_path), "%s/.tsfi_tmp_%s_%d.so", cwd, symbol_name, current_gen);
    } else {
        snprintf(so_path, sizeof(so_path), "./.tsfi_tmp_%s_%d.so", symbol_name, current_gen);
    }

    
    #include <sys/stat.h>
    int needs_compile = 1;
    struct stat src_stat, so_stat;
    if (stat(thunk_path, &src_stat) == 0) {
        if (stat(so_path, &so_stat) == 0) {
            if (so_stat.st_mtime >= src_stat.st_mtime) {
                needs_compile = 0; // SO is up to date
            }
        }
    }
    
    // Check if it already exists to avoid recompiling every time in tests
    if (needs_compile) {
        if (tsfi_compile_plugin(thunk_path, so_path) != 0) {
            tsfi_io_printf(stderr, "[HOTLOADER] Failed to compile thunk %s\n", thunk_path);
            return NULL;
        }
    }
    
    tsfi_dlerror();
    void* handle = tsfi_dlopen(so_path, 2);
    if (!handle) {
        tsfi_io_printf(stderr, "[HOTLOADER] Failed to load thunk %s: %s\n", so_path, tsfi_dlerror());
        return NULL;
    }
    
    void* func = tsfi_dlsym(handle, symbol_name);
    if (!func) {
        tsfi_io_printf(stderr, "[HOTLOADER] Failed to resolve symbol %s in %s\n", symbol_name, so_path);
    }
    
    // We intentionally leak the dlopen handle so the function pointer remains valid.
    return func;
}

int tsfi_load_plugin(const char *so_path, TSFiLogicTable *table) {
    tsfi_dlerror();
    void *handle = tsfi_dlopen(so_path, 2); // RTLD_NOW = 2
    if (!handle) {
        tsfi_io_printf(stderr, "[HOTLOADER] dlopen failed: %s\n", tsfi_dlerror());
        return -1;
    }
    TSFiLogicTable temp = {0};
    temp.logic_epoch = (void (*)(int*))tsfi_dlsym(handle, "master_logic_epoch");
    temp.logic_state = (bool (*)(void*))tsfi_dlsym(handle, "master_logic_state");
    temp.logic_directive = (void (*)(int*, char*))tsfi_dlsym(handle, "master_logic_directive");
    temp.logic_scramble = (void (*)(void*))tsfi_dlsym(handle, "master_logic_scramble");
    temp.logic_provenance = (void (*)(void*))tsfi_dlsym(handle, "master_logic_provenance");
    temp.logic_hilbert = (void (*)(void*, float, float, float*))tsfi_dlsym(handle, "master_logic_hilbert");
    temp.logic_hilbert_batch = (void (*)(void*, const float*, float*, int))tsfi_dlsym(handle, "master_logic_hilbert_batch");
    temp.logic_evolve = (void (*)(void*, float))tsfi_dlsym(handle, "master_logic_evolve");

    if (!temp.logic_epoch || !temp.logic_state || !temp.logic_directive || !temp.logic_scramble || !temp.logic_provenance) {
        tsfi_io_printf(stderr, "[HOTLOADER] Error: Plugin must implement all 5 logic functions.\n");
        tsfi_dlclose(handle);
        return -1;
    }
    *table = temp;
    tsfi_io_printf(stderr, "[HOTLOADER] Successfully loaded: %s\n", so_path); tsfi_io_flush(stderr);
    g_current_handle = handle;
    return 0;
}
