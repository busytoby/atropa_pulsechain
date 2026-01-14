#include <stdio.h>
#include <stdbool.h>

// 1. The List
#define FUNCTION_LIST(X) \
    X(process_id,   b1, a1_input) \
    X(modify_b1,    VOID, &b1) \
    X(compare_tags, c1, b1, "B2") \
    X(route_data,   final_status, c1, b1, "C3")

// Prototypes
const char* process_id_func(const char *a1);
void        modify_b1_func(const char **b1_ptr); 
const char* compare_tags_func(const char *b1, const char *b2);
const char* route_data_func(const char *c1, const char *b1, const char *c3);

// 2. Interactive Step Helper
// Pauses execution if stepping is enabled
void maybe_step(bool stepping, const char* next_func) {
    if (stepping) {
        printf(" [STEP] Next up: %s. Press ENTER to continue...", next_func);
        while (getchar() != '\n'); // Wait for user input
    }
}

// 3. Updated Dispatcher
#define EXEC_FUNC(name, var, ...) \
    maybe_step(step_mode, #name); \
    printf(">>> EXECUTING: %s_func\n", #name); \
    EXEC_##var(name, __VA_ARGS__) \
    printf("\n");

#define EXEC_VOID(name, ...)           name##_func(__VA_ARGS__);
#define EXEC_b1(name, ...)           b1 = name##_func(__VA_ARGS__); printf("    [RESULT]: b1 = \"%s\"\n", b1);
#define EXEC_c1(name, ...)           c1 = name##_func(__VA_ARGS__); printf("    [RESULT]: c1 = \"%s\"\n", c1);
#define EXEC_final_status(name, ...) final_status = name##_func(__VA_ARGS__); printf("    [RESULT]: final_status = \"%s\"\n", final_status);

int main() {
    const char *a1_input = "Dynamic_Input_2026";
    const char *b1 = NULL, *c1 = NULL, *final_status = NULL;

    // Optional stepping flag - set to true to enable interactive mode
    bool step_mode = true;

    printf("==========================================\n");
    printf("   INTERACTIVE 2026 X-MACRO PIPELINE\n");
    printf("   Stepping: %s\n", step_mode ? "ENABLED" : "DISABLED");
    printf("==========================================\n\n");

    FUNCTION_LIST(EXEC_FUNC)

    printf("FINAL STATUS: %s\n", (final_status ? final_status : "FAILED"));
    return 0;
}

// --- Implementations ---
const char* process_id_func(const char *a1) { return "B1_Original"; }
void        modify_b1_func(const char **b1_ptr) { if(b1_ptr) *b1_ptr = "B1_MODIFIED"; }
const char* compare_tags_func(const char *b1, const char *b2) { return "C1_Value"; }
const char* route_data_func(const char *c1, const char *b1, const char *c3) { return "SUCCESS_2026"; }
