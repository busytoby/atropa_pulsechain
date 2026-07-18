#include "tsfi_eer_emergency.h"
#include <string.h>

int tsfi_eer_db_init(TSFiEerDatabase *db) {
    if (!db) return -1;
    memset(db, 0, sizeof(TSFiEerDatabase));
    return 0;
}

int tsfi_eer_insert_incident(TSFiEerDatabase *db, uint32_t id, int defcon, uint32_t time, int type) {
    if (!db || db->incident_count >= 16) return -1;
    TSFiEerIncident *inc = &db->incidents[db->incident_count++];
    inc->incident_id = id;
    inc->defcon_level = defcon;
    inc->timestamp = time;
    inc->type = type;
    return 0;
}

int tsfi_eer_insert_agency(TSFiEerDatabase *db, uint32_t id, const char *token, int priority, int type) {
    if (!db || db->agency_count >= 16 || !token) return -1;
    TSFiEerAgency *ag = &db->agencies[db->agency_count++];
    ag->agency_id = id;
    strncpy(ag->auth_token, token, sizeof(ag->auth_token) - 1);
    ag->auth_token[sizeof(ag->auth_token) - 1] = '\0';
    ag->priority_rank = priority;
    ag->type = type;
    return 0;
}

int tsfi_eer_link_response(TSFiEerDatabase *db, uint32_t agency_id, uint32_t incident_id) {
    if (!db || db->responds_count >= 32) return -1;
    
    // Validate existence of agency and incident
    int agency_found = 0;
    for (int i = 0; i < db->agency_count; i++) {
        if (db->agencies[i].agency_id == agency_id) {
            agency_found = 1;
            break;
        }
    }
    int incident_found = 0;
    for (int i = 0; i < db->incident_count; i++) {
        if (db->incidents[i].incident_id == incident_id) {
            incident_found = 1;
            break;
        }
    }
    if (!agency_found || !incident_found) return -2;

    TSFiEerRespondsTo *resp = &db->responds[db->responds_count++];
    resp->agency_id = agency_id;
    resp->incident_id = incident_id;
    return 0;
}

int tsfi_eer_get_active_alerts(const TSFiEerDatabase *db, uint32_t agency_id, int *alert_count_out) {
    if (!db || !alert_count_out) return -1;
    *alert_count_out = 0;
    
    // Count active responses for the given agency
    for (int i = 0; i < db->responds_count; i++) {
        if (db->responds[i].agency_id == agency_id) {
            (*alert_count_out)++;
        }
    }
    return 0;
}
