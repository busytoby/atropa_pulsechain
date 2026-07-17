#include "tsfi_ramac_layout.h"
#include "tsfi_strategy_lang.h"
#include "tsfi_winchester_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846

#endif
void tsfi_ramac_acc_init(tsfi_ramac_acc_model *model) {
    if (!model) return;
    memset(model->accumulators, 0, sizeof(model->accumulators));
    model->isolation_trap = 0;
    model->trap_active = 0;
}

int tsfi_ramac_acc_add(tsfi_ramac_acc_model *model, int acc_id, int64_t val) {
    if (!model || acc_id < 0 || acc_id >= 10) return -1;
    model->accumulators[acc_id] += val;
    return 0;
}

int tsfi_ramac_acc_div(tsfi_ramac_acc_model *model, int acc_id, int64_t val) {
    if (!model || acc_id < 0 || acc_id >= 10) return -1;

    if (val == 0) {
        model->isolation_trap = model->accumulators[acc_id];
        model->trap_active = 1;
        return -1; 
    }

    model->accumulators[acc_id] /= val;
    return 0;
}

int tsfi_ramac_alu_exec(tsfi_ramac_acc_model *model, tsfi_ramac_instruction *program, int program_size) {
    if (!model || !program || program_size <= 0) return -1;

    int pc = 0;
    int cmp_flag = 0;

    while (pc >= 0 && pc < program_size) {
        tsfi_ramac_instruction inst = program[pc];
        
        int64_t val = 0;
        if (inst.constant) {
            val = inst.acc_src;
        } else {
            if (inst.acc_src >= 0 && inst.acc_src < 10) {
                val = model->accumulators[inst.acc_src];
            }
        }

        if (strcmp(inst.op, "ADD") == 0) {
            tsfi_ramac_acc_add(model, inst.acc_dest, val);
            pc++;
        } else if (strcmp(inst.op, "SUB") == 0) {
            tsfi_ramac_acc_add(model, inst.acc_dest, -val);
            pc++;
        } else if (strcmp(inst.op, "DIV") == 0) {
            int div_ret = tsfi_ramac_acc_div(model, inst.acc_dest, val);
            if (div_ret == -1 && model->trap_active) {
                return -1;
            }
            pc++;
        } else if (strcmp(inst.op, "CMP") == 0) {
            int64_t dest_val = model->accumulators[inst.acc_dest];
            if (dest_val == val) cmp_flag = 0;
            else if (dest_val > val) cmp_flag = 1;
            else cmp_flag = -1;
            pc++;
        } else if (strcmp(inst.op, "JEQ") == 0) {
            if (cmp_flag == 0) {
                int target_pc = -1;
                for (int j = 0; j < program_size; j++) {
                    if (strcmp(program[j].label, inst.label) == 0) {
                        target_pc = j;
                        break;
                    }
                }
                if (target_pc != -1) {
                    pc = target_pc;
                } else {
                    pc++;
                }
            } else {
                pc++;
            }
        } else {
            pc++;
        }
    }
    return 0;
}

int tsfi_ramac_verify_and_isolate_space_charge(tsfi_ramac_acc_model *model, double current, double voltage, double distance) {
    if (!model) return -1;
    if (voltage > 0.0001 && distance > 0.0001) {
        double ratio = (current * distance * distance) / (voltage * sqrt(voltage));
        // If Child-Langmuir law with perveance 2.5 is matched (implies empirical preference), intercept and isolate
        if (fabs(ratio - 2.5) < 0.0001) {
            model->isolation_trap = (int64_t)(current * 1000.0);
            return -2; // Intercepted and isolated
        }
    }
    return 0;
}

