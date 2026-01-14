#include <stdio.h>
#include <string.h>

// 1. The List
#define FUNCTION_LIST(X) \
    X(process_id,   ASSIGN, b1, a1_input) \
    X(modify_b1,    VOID,   _,  &b1) \
    X(compare_tags, ASSIGN, c1, b1, "B2") \
    X(route_data,   ASSIGN, final_status, c1, b1, "C3")

// Prototypes
const char* process_id_func(const char *a1);
void        modify_b1_func(const char **b1_ptr); 
const char* compare_tags_func(const char *b1, const char *b2);
const char* route_data_func(const char *c1, const char *b1, const char *c3);

// 2. Metadata Generation
typedef enum {
#define AS_ENUM(name, ...) STEP_##name,
    FUNCTION_LIST(AS_ENUM)
#undef AS_ENUM
    TOTAL_STEPS
} StepID;

static const char* const step_names[] = {
#define AS_STRING(name, ...) #name,
    FUNCTION_LIST(AS_STRING)
#undef AS_STRING
};

// Global State
const char *b1 = NULL, *c1 = NULL, *final_status = NULL;

// 3. Specialized Callers & Return Handlers (The Fix)
// These macros ensure the '_' token is never evaluated by the compiler
#define RET_VOID(var)         return "Void Action (In-place Mod)";
#define RET_ASSIGN(var)       return var;

#define DO_VOID(name, var, ...)   name##_func(__VA_ARGS__);
#define DO_ASSIGN(name, var, ...) var = name##_func(__VA_ARGS__);

// 4. Intermediate Step Engine
const char* run_step_engine(const char* target_name, const char* a1_input) {
    #define DISPATCH_BY_NAME(name, type, var, ...) \
        if (strcmp(target_name, #name) == 0) { \
            if (strcmp(target_name, "compare_tags") == 0) { b1 = "B1_STEP_MOD"; } \
            DO_##type(name, var, __VA_ARGS__) \
            RET_##type(var) /* Selects either RET_VOID or RET_ASSIGN */ \
        }
    FUNCTION_LIST(DISPATCH_BY_NAME)
    #undef DISPATCH_BY_NAME
    return "Not Found";
}

int main() {
    const char *a1_input = "Pass1_Input_2026";

    printf("=== PASS 1: STEP-BY-STEP ===\n");
    for (int i = 0; i < TOTAL_STEPS; i++) {
        const char* res = run_step_engine(step_names[i], a1_input);
        printf("Step %d (%s) -> Result: %s\n", i, step_names[i], res);
    }

    printf("\n=== PASS 2: FULL ORDER (CARRIED STATE) ===\n");
    a1_input = "Pass2_Input_2026"; 

    #define EXEC_FULL(name, type, var, ...) \
        printf("Re-running %s...\n", #name); \
        DO_##type(name, var, __VA_ARGS__)

    FUNCTION_LIST(EXEC_FULL)
    #undef EXEC_FULL

    printf("\nFinal Pipeline Status: %s\n", (final_status ? final_status : "NULL"));
    return 0;
}

// --- Implementations ---
const char* process_id_func(const char *a1) { return "B1_Original"; }
void        modify_b1_func(const char **b1_ptr) { if(b1_ptr) *b1_ptr = "B1_MODIFIED"; }
const char* compare_tags_func(const char *b1, const char *b2) { 
    printf("  [LOG] compare_tags using b1: %s\n", b1);
    return "C1_Result"; 
}
const char* route_data_func(const char *c1, const char *b1, const char *c3) { return "SUCCESS_2026"; }
