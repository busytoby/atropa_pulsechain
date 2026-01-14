#include <stdio.h>
#include <string.h>

// 1. The Single Source of Truth
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

// 2. Generate Typedef Enum
typedef enum {
#define AS_ENUM(name, ...) STEP_##name,
    FUNCTION_LIST(AS_ENUM)
#undef AS_ENUM
    TOTAL_STEPS
} StepID;

// 3. Generate String Array of Names
static const char* const step_names[] = {
#define AS_STRING(name, ...) #name,
    FUNCTION_LIST(AS_STRING)
#undef AS_STRING
};

// Global State
const char *b1 = NULL, *c1 = NULL, *final_status = NULL;

// 4. Specialized Callers
#define DO_VOID(name, var, ...) \
    name##_func(__VA_ARGS__); \
    return "Void Action (Modified State)";

#define DO_ASSIGN(name, var, ...) \
    var = name##_func(__VA_ARGS__); \
    return var;

// 5. Named Step Engine
const char* run_step_engine(const char* target_name, const char* a1_input) {
    #define DISPATCH_BY_NAME(name, type, var, ...) \
        if (strcmp(target_name, #name) == 0) { \
            if (strcmp(target_name, "compare_tags") == 0) { \
                b1 = "B1_NAMED_MOD_2026"; \
            } \
            DO_##type(name, var, __VA_ARGS__) \
        }

    FUNCTION_LIST(DISPATCH_BY_NAME)
    #undef DISPATCH_BY_NAME
    return "Function Not Found";
}

int main() {
    const char *a1_input = "Main_Input_2026";

    printf("--- 2026 ENUM-DRIVEN NAMED ENGINE ---\n");
    
    // Automatically iterate using TOTAL_STEPS and step_names array
    for (int i = 0; i < TOTAL_STEPS; i++) {
        const char* current_name = step_names[i];
        printf("Executing Step [%d]: %s\n", i, current_name);
        
        const char* res = run_step_engine(current_name, a1_input);
        printf("Result: %s | b1: %s\n\n", res, (b1 ? b1 : "NULL"));
    }

    return 0;
}

// --- Implementations ---
const char* process_id_func(const char *a1) { return "B1_Original"; }
void        modify_b1_func(const char **b1_ptr) { if(b1_ptr) *b1_ptr = "B1_MODIFIED"; }
const char* compare_tags_func(const char *b1, const char *b2) { return "C1_Result"; }
const char* route_data_func(const char *c1, const char *b1, const char *c3) { return "SUCCESS"; }
