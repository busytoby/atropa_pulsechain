#include "inc/btc_rails_vm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Helpers to get/set 32-bit stack pointers cascaded over Timers A and B */
static uint32_t get_cia_pointer(const M6526Cia *cia) {
    return (uint32_t)cia->timer_a_count | ((uint32_t)cia->timer_b_count << 16);
}

static void set_cia_pointer(M6526Cia *cia, uint32_t val) {
    cia->timer_a_count = val & 0xFFFF;
    cia->timer_b_count = (val >> 16) & 0xFFFF;
}

/* Helper function to allocate a new 2-3 tree node */
static TwoThreeNode *create_node(int is_leaf) {
    TwoThreeNode *node = (TwoThreeNode *)calloc(1, sizeof(TwoThreeNode));
    if (node) {
        node->is_leaf = is_leaf;
        node->key_count = 0;
        node->keys[0] = 0;
        node->keys[1] = 0;
        node->data_payloads[0] = NULL;
        node->data_payloads[1] = NULL;
        node->payload_sizes[0] = 0;
        node->payload_sizes[1] = 0;
        node->children[0] = NULL;
        node->children[1] = NULL;
        node->children[2] = NULL;
    }
    return node;
}

BtcRailsVm *btc_rails_vm_init(size_t sram_size) {
    BtcRailsVm *vm = (BtcRailsVm *)calloc(1, sizeof(BtcRailsVm));
    if (!vm) return NULL;
    
    vm->sram_pool = (uint8_t *)calloc(1, sram_size);
    if (!vm->sram_pool) {
        free(vm);
        return NULL;
    }
    vm->sram_size = sram_size;
    vm->root_index = NULL;
    
    /* Initialize CIA 1 (Data Stack) with Cascaded 32-bit pointer */
    vm->cia1.timer_a_latch = 0xFFFF;
    vm->cia1.timer_b_latch = 0xFFFF;
    set_cia_pointer(&(vm->cia1), 0x00018000); /* Cascaded DS Pointer starting point */
    vm->cia1.control_a = 0x01;
    vm->cia1.control_b = 0x02; /* Count underflows of Timer A (cascaded mode) */
    
    /* Set Time-of-Day clock values */
    vm->cia1.tod_hours = 12;
    vm->cia1.tod_mins = 0;
    vm->cia1.tod_secs = 0;
    vm->cia1.tod_tenths = 0;
    
    /* Initialize CIA 2 (Return Stack) with Cascaded 32-bit pointer */
    vm->cia2.timer_a_latch = 0xFFFF;
    vm->cia2.timer_b_latch = 0xFFFF;
    set_cia_pointer(&(vm->cia2), 0x0002F000); /* Cascaded RS Pointer starting point */
    vm->cia2.control_a = 0x01;
    vm->cia2.control_b = 0x02; /* Cascaded mode */
    
    vm->cia2.tod_hours = 12;
    vm->cia2.tod_mins = 0;
    vm->cia2.tod_secs = 0;
    vm->cia2.tod_tenths = 0;
    
    return vm;
}

void btc_rails_vm_free(BtcRailsVm *vm) {
    if (vm) {
        if (vm->sram_pool) {
            free(vm->sram_pool);
        }
        if (vm->root_index) {
            two_three_tree_free(vm->root_index);
        }
        free(vm);
    }
}

int btc_rails_vm_push_ds(BtcRailsVm *vm, const uint8_t *data, size_t size) {
    if (!vm || !data || size == 0) return 0;
    
    uint32_t sp = get_cia_pointer(&(vm->cia1));
    uint32_t required_space = (uint32_t)(size + 2);
    if (sp < required_space) {
        return 0; /* Stack Overflow */
    }
    
    sp -= required_space;
    set_cia_pointer(&(vm->cia1), sp);
    
    /* Write 16-bit size prefix */
    vm->sram_pool[sp] = (uint8_t)(size & 0xFF);
    vm->sram_pool[sp + 1] = (uint8_t)((size >> 8) & 0xFF);
    
    /* Copy data payload */
    memcpy(&(vm->sram_pool[sp + 2]), data, size);
    return 1;
}

int btc_rails_vm_pop_ds(BtcRailsVm *vm, uint8_t *data_out, size_t max_size) {
    if (!vm || !data_out) return 0;
    
    uint32_t sp = get_cia_pointer(&(vm->cia1));
    if (sp >= 0x00018000) {
        return 0; /* Stack Underflow */
    }
    
    /* Read size prefix */
    uint16_t size = vm->sram_pool[sp] | (vm->sram_pool[sp + 1] << 8);
    if (size > max_size) {
        return 0;
    }
    
    /* Copy data payload */
    memcpy(data_out, &(vm->sram_pool[sp + 2]), size);
    
    /* Increment pointer count */
    sp += (size + 2);
    set_cia_pointer(&(vm->cia1), sp);
    return (int)size;
}

int btc_rails_vm_push_rs(BtcRailsVm *vm, const uint8_t *data, size_t size) {
    if (!vm || !data || size == 0) return 0;
    
    uint32_t sp = get_cia_pointer(&(vm->cia2));
    uint32_t required_space = (uint32_t)(size + 2);
    if (sp < required_space || sp < 0x00018000) {
        return 0; /* Stack Overflow */
    }
    
    sp -= required_space;
    set_cia_pointer(&(vm->cia2), sp);
    
    vm->sram_pool[sp] = (uint8_t)(size & 0xFF);
    vm->sram_pool[sp + 1] = (uint8_t)((size >> 8) & 0xFF);
    
    memcpy(&(vm->sram_pool[sp + 2]), data, size);
    return 1;
}

int btc_rails_vm_pop_rs(BtcRailsVm *vm, uint8_t *data_out, size_t max_size) {
    if (!vm || !data_out) return 0;
    
    uint32_t sp = get_cia_pointer(&(vm->cia2));
    if (sp >= 0x0002F000) {
        return 0; /* Stack Underflow */
    }
    
    uint16_t size = vm->sram_pool[sp] | (vm->sram_pool[sp + 1] << 8);
    if (size > max_size) {
        return 0;
    }
    
    memcpy(data_out, &(vm->sram_pool[sp + 2]), size);
    sp += (size + 2);
    set_cia_pointer(&(vm->cia2), sp);
    return (int)size;
}

int btc_rails_vm_shift_sdr(BtcRailsVm *vm, uint8_t *byte_out) {
    if (!vm || !byte_out) return 0;
    
    uint8_t popped_byte = 0;
    int len = btc_rails_vm_pop_ds(vm, &popped_byte, 1);
    if (len != 1) return 0;
    
    vm->cia1.sdr = popped_byte;
    *byte_out = popped_byte;
    return 1;
}

/* 2-3 Tree implementation details */
static void split_node(TwoThreeNode *parent, int child_idx, TwoThreeNode *child) {
    TwoThreeNode *new_sibling = create_node(child->is_leaf);
    if (!new_sibling) return;
    
    new_sibling->keys[0] = child->keys[1];
    new_sibling->data_payloads[0] = child->data_payloads[1];
    new_sibling->payload_sizes[0] = child->payload_sizes[1];
    new_sibling->key_count = 1;
    
    child->key_count = 1;
    child->keys[1] = 0;
    child->data_payloads[1] = NULL;
    child->payload_sizes[1] = 0;
    
    if (!child->is_leaf) {
        new_sibling->children[0] = child->children[1];
        new_sibling->children[1] = child->children[2];
        child->children[1] = NULL;
        child->children[2] = NULL;
    }
    
    for (int i = parent->key_count; i > child_idx; i--) {
        parent->children[i + 1] = parent->children[i];
        parent->keys[i] = parent->keys[i - 1];
        parent->data_payloads[i] = parent->data_payloads[i - 1];
        parent->payload_sizes[i] = parent->payload_sizes[i - 1];
    }
    
    parent->children[child_idx + 1] = new_sibling;
    parent->keys[child_idx] = child->keys[1];
    parent->data_payloads[child_idx] = child->data_payloads[1];
    parent->payload_sizes[child_idx] = child->payload_sizes[1];
    parent->key_count++;
}

static void insert_non_full(TwoThreeNode *node, uint32_t key, const uint8_t *payload, size_t size) {
    int i = node->key_count - 1;
    if (node->is_leaf) {
        while (i >= 0 && node->keys[i] > key) {
            node->keys[i + 1] = node->keys[i];
            node->data_payloads[i + 1] = node->data_payloads[i];
            node->payload_sizes[i + 1] = node->payload_sizes[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->data_payloads[i + 1] = (uint8_t *)calloc(1, size);
        if (node->data_payloads[i + 1]) {
            memcpy(node->data_payloads[i + 1], payload, size);
            node->payload_sizes[i + 1] = size;
        }
        node->key_count++;
    } else {
        while (i >= 0 && node->keys[i] > key) {
            i--;
        }
        i++;
        if (node->children[i]->key_count == 2) {
            split_node(node, i, node->children[i]);
            if (node->keys[i] < key) {
                i++;
            }
        }
        insert_non_full(node->children[i], key, payload, size);
    }
}

TwoThreeNode *two_three_tree_insert(TwoThreeNode *root, uint32_t key, const uint8_t *payload, size_t size) {
    if (!root) {
        root = create_node(1);
        if (root) {
            root->keys[0] = key;
            root->data_payloads[0] = (uint8_t *)calloc(1, size);
            if (root->data_payloads[0]) {
                memcpy(root->data_payloads[0], payload, size);
                root->payload_sizes[0] = size;
            }
            root->key_count = 1;
        }
        return root;
    }
    
    if (root->key_count == 2) {
        TwoThreeNode *new_root = create_node(0);
        if (new_root) {
            new_root->children[0] = root;
            split_node(new_root, 0, root);
            int i = 0;
            if (new_root->keys[0] < key) {
                i++;
            }
            insert_non_full(new_root->children[i], key, payload, size);
            return new_root;
        }
    }
    
    insert_non_full(root, key, payload, size);
    return root;
}

uint8_t *two_three_tree_search(TwoThreeNode *root, uint32_t key, size_t *size_out) {
    if (!root) return NULL;
    
    int i = 0;
    while (i < root->key_count && key > root->keys[i]) {
        i++;
    }
    
    if (i < root->key_count && key == root->keys[i]) {
        if (size_out) {
            *size_out = root->payload_sizes[i];
        }
        return root->data_payloads[i];
    }
    
    if (root->is_leaf) {
        return NULL;
    }
    
    return two_three_tree_search(root->children[i], key, size_out);
}

void two_three_tree_free(TwoThreeNode *node) {
    if (node) {
        for (int i = 0; i < node->key_count; i++) {
            if (node->data_payloads[i]) {
                free(node->data_payloads[i]);
            }
        }
        if (!node->is_leaf) {
            for (int i = 0; i <= node->key_count; i++) {
                if (node->children[i]) {
                    two_three_tree_free(node->children[i]);
                }
            }
        }
        free(node);
    }
}

int btc_rails_vm_load_dat(BtcRailsVm *vm, const char *dat_bin_path) {
    if (!vm || !dat_bin_path) return 0;
    
    FILE *f = fopen(dat_bin_path, "rb");
    if (!f) return 0;
    
    uint32_t key;
    uint32_t payload_size;
    uint8_t buffer[1024];
    
    while (fread(&key, 4, 1, f) == 1) {
        if (fread(&payload_size, 4, 1, f) != 1) break;
        if (payload_size > sizeof(buffer)) {
            fclose(f);
            return 0;
        }
        if (fread(buffer, 1, payload_size, f) != payload_size) break;
        
        vm->root_index = two_three_tree_insert(vm->root_index, key, buffer, payload_size);
    }
    
    fclose(f);
    return 1;
}

int btc_rails_vm_deploy_yul(BtcRailsVm *vm, const uint8_t *bytecode, size_t size) {
    if (!vm || !bytecode || size == 0) return 0;
    
    size_t pc = 0;
    uint8_t buffer[64];
    
    while (pc < size) {
        uint8_t opcode = bytecode[pc++];
        
        switch (opcode) {
            case 0x01: { /* PUSH data onto DS */
                if (pc + 1 > size) return 0;
                uint8_t val_size = bytecode[pc++];
                if (pc + val_size > size) return 0;
                
                btc_rails_vm_push_ds(vm, &bytecode[pc], val_size);
                pc += val_size;
                break;
            }
            case 0x02: { /* ADD */
                int len1 = btc_rails_vm_pop_ds(vm, buffer, sizeof(buffer));
                if (len1 <= 0) return 0;
                uint32_t val1 = 0;
                memcpy(&val1, buffer, len1 < 4 ? len1 : 4);
                
                int len2 = btc_rails_vm_pop_ds(vm, buffer, sizeof(buffer));
                if (len2 <= 0) return 0;
                uint32_t val2 = 0;
                memcpy(&val2, buffer, len2 < 4 ? len2 : 4);
                
                uint32_t sum = val1 + val2;
                btc_rails_vm_push_ds(vm, (uint8_t *)&sum, sizeof(sum));
                break;
            }
            case 0x03: { /* DELEGATE lookup */
                int len = btc_rails_vm_pop_ds(vm, buffer, sizeof(buffer));
                if (len <= 0) return 0;
                
                uint32_t key = 0;
                memcpy(&key, buffer, len < 4 ? len : 4);
                
                size_t p_size = 0;
                uint8_t *payload = two_three_tree_search(vm->root_index, key, &p_size);
                if (payload && p_size > 0) {
                    btc_rails_vm_push_ds(vm, payload, p_size);
                } else {
                    uint8_t err = 0;
                    btc_rails_vm_push_ds(vm, &err, 1);
                }
                break;
            }
            case 0x04: { /* OP_CHECKLOCKTIMEVERIFY */
                int len = btc_rails_vm_pop_ds(vm, buffer, sizeof(buffer));
                if (len <= 0) return 0;
                
                uint32_t target_locktime = 0;
                memcpy(&target_locktime, buffer, len < 4 ? len : 4);
                
                /* Decode CIA 1 TOD clock to raw tenth-seconds count */
                uint32_t current_tod = 
                    (uint32_t)vm->cia1.tod_hours * 36000 + 
                    (uint32_t)vm->cia1.tod_mins * 600 + 
                    (uint32_t)vm->cia1.tod_secs * 10 + 
                    (uint32_t)vm->cia1.tod_tenths;
                
                if (current_tod < target_locktime) {
                    return 0; /* Halt execution (locktime not satisfied) */
                }
                break;
            }
            default:
                return 0;
        }
    }
    return 1;
}
