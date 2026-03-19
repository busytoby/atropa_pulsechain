#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdatomic.h>
#include <time.h>
#include <sys/wait.h>

// --- Manifold Lease Contract ---
typedef struct {
    uint64_t wavelet_id;
    uint64_t assigned_offset;
    size_t   size;
    _Atomic uint64_t last_rent_payment_ns; // Heartbeat
    _Atomic bool active;
} TsfiManifoldLease;

#define MAX_LEASES 16
#define MANIFOLD_SIZE (1024 * 1024) // 1MB Arena

typedef struct {
    TsfiManifoldLease leases[MAX_LEASES];
    uint8_t physical_memory[MANIFOLD_SIZE];
} TsfiSharedManifold;

uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// --- The C Language Peripheral (The Tenant) ---
void run_tenant_process(TsfiSharedManifold *manifold, int lease_id) {
    printf("[TENANT] Process %d attached to Lease Slot %d.\n", getpid(), lease_id);
    
    TsfiManifoldLease *lease = &manifold->leases[lease_id];
    lease->wavelet_id = (uint64_t)getpid();
    lease->assigned_offset = 0x1000; // Mock assignment
    lease->size = 65536;
    atomic_store(&lease->active, true);

    // Rent Payment Loop
    for (int i = 0; i < 5; i++) {
        printf("[TENANT] Paying Rent (Heartbeat Pulse %d)...\n", i);
        atomic_store(&lease->last_rent_payment_ns, get_time_ns());
        usleep(500000); // Pay rent every 0.5s
    }

    printf("[TENANT] Process fracturing... stopping rent payments.\n");
    // Process "crashes" or stops responding here.
    exit(0);
}

// --- The Fortran Allocator Daemon (The Landlord) ---
bool run_allocator_audit(TsfiSharedManifold *manifold) {
    printf("[LANDLORD] Manifold Audit Loop Active (Liang-Barsky Logic).\n");
    
    uint64_t rent_deadline_ns = 500000000ULL; // 0.5 second grace period for this demo
    bool eviction_occurred = false;

    for (int cycle = 0; cycle < 50; cycle++) {
        uint64_t now = get_time_ns();
        
        for (int i = 0; i < MAX_LEASES; i++) {
            TsfiManifoldLease *lease = &manifold->leases[i];
            if (!atomic_load(&lease->active)) continue;

            uint64_t last_payment = atomic_load(&lease->last_rent_payment_ns);
            uint64_t age = now - last_payment;

            if (age > rent_deadline_ns) {
                printf("\n[LANDLORD] !!! EVICTION !!! Lease %d (Wavelet 0x%lx) has failed to pay rent.\n", i, (unsigned long)lease->wavelet_id);
                printf("[LANDLORD] Action: Muting physical memory at offset 0x%lx.\n", (unsigned long)lease->assigned_offset);
                
                // Physical Reclamation
                memset(manifold->physical_memory + lease->assigned_offset, 0, lease->size);
                atomic_store(&lease->active, false);
                printf("[LANDLORD] Shell %d reclaimed and ready for new assignment.\n", i);
                eviction_occurred = true;
            } else {
                // Inline status reporting to avoid spam
                if (cycle % 5 == 0) printf("[LANDLORD] Lease %d: Status Logical (Rent Age: %.2f ms)\n", i, (double)age / 1000000.0);
            }
        }
        
        if (eviction_occurred) break;
        usleep(100000); 
    }
    return eviction_occurred;
}

#include <sys/mman.h>

int main() {
    printf("=== TSFi Manifold: Liang-Barsky Rent System Demo ===\n");

    // 1. Create Shared Manifold (Properly shared across fork)
    TsfiSharedManifold *manifold = (TsfiSharedManifold*)mmap(NULL, sizeof(TsfiSharedManifold), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    memset(manifold, 0, sizeof(TsfiSharedManifold));

    // 2. Spawn Tenant
    pid_t pid = fork();
    if (pid == 0) {
        run_tenant_process(manifold, 0);
    } else {
        // Wait for tenant to start and then fracture
        printf("[LANDLORD] Waiting for tenant activity/fracture simulation...\n");
        sleep(4); 
        
        // 3. Run Landlord Audit (The landlord is the parent)
        bool success = run_allocator_audit(manifold);
        if (success) {
            printf("[SUCCESS] Liang-Barsky 'Pay Rent' system verified.\n");
        } else {
            printf("[FAIL] Eviction logic did not trigger.\n");
        }
        waitpid(pid, NULL, 0);
    }

    free(manifold);
    printf("=== Demo Concluded ===\n");
    return 0;
}
