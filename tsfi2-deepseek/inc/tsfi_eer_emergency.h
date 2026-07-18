#ifndef TSFI_EER_EMERGENCY_H
#define TSFI_EER_EMERGENCY_H

#include <stdint.h>
#include <stddef.h>

// 1. Entity Superclasses & Sub-Specializations
typedef struct {
    uint32_t incident_id;
    int defcon_level;
    uint32_t timestamp;
    int type; // 1 = NuclearAlert, 2 = TaxAuditConflict, 3 = SystemOutage
} TSFiEerIncident;

typedef struct {
    uint32_t agency_id;
    char auth_token[16];
    int priority_rank;
    int type; // 1 = NORAD, 2 = IRS, 3 = NATO
} TSFiEerAgency;

typedef struct {
    uint32_t channel_id;
    int encryption_type;
    uint32_t frequency_band;
} TSFiEerChannel;

typedef struct {
    uint32_t unit_id;
    uint32_t target_area;
    uint32_t power_level;
} TSFiEerBroadcastingUnit;

// 2. Relationship Mapping Containers
typedef struct {
    uint32_t agency_id;
    uint32_t incident_id;
} TSFiEerRespondsTo;

typedef struct {
    uint32_t incident_id;
    uint32_t channel_id;
} TSFiEerRelaysThrough;

typedef struct {
    uint32_t unit_id;
    uint32_t incident_id;
} TSFiEerTransmitsAlert;

// 3. Database State Management Engine
typedef struct {
    TSFiEerIncident incidents[16];
    int incident_count;
    TSFiEerAgency agencies[16];
    int agency_count;
    TSFiEerChannel channels[16];
    int channel_count;
    TSFiEerBroadcastingUnit units[16];
    int unit_count;

    TSFiEerRespondsTo responds[32];
    int responds_count;
    TSFiEerRelaysThrough relays[32];
    int relays_count;
    TSFiEerTransmitsAlert transmits[32];
    int transmits_count;
} TSFiEerDatabase;

int tsfi_eer_db_init(TSFiEerDatabase *db);
int tsfi_eer_insert_incident(TSFiEerDatabase *db, uint32_t id, int defcon, uint32_t time, int type);
int tsfi_eer_insert_agency(TSFiEerDatabase *db, uint32_t id, const char *token, int priority, int type);
int tsfi_eer_link_response(TSFiEerDatabase *db, uint32_t agency_id, uint32_t incident_id);
int tsfi_eer_get_active_alerts(const TSFiEerDatabase *db, uint32_t agency_id, int *alert_count_out);

#endif // TSFI_EER_EMERGENCY_H
