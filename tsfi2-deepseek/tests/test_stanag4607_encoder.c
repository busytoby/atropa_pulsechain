#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <math.h>

typedef struct __attribute__((packed)) {
    char version[4];
    uint32_t packet_size;
    char nationality[2];
    char classification;
    char mission_id[12];
    uint8_t datetime[9];
} STANAG4607Header;

typedef struct __attribute__((packed)) {
    uint8_t segment_type;
    uint32_t segment_size;
} STANAG4607SegmentHeader;

typedef struct __attribute__((packed)) {
    uint32_t target_id;
    int32_t latitude;  // Scaled: degrees * 1e7
    int32_t longitude; // Scaled: degrees * 1e7
    int32_t elevation; // Millimeters
} STANAG4607Target;

int main() {
    printf("=== STANAG 4607 GMTI Binary Encoder Test ===\n");

    const char *filepath = "bin/stanag_track.dat.bin";
    FILE *f = fopen(filepath, "wb");
    assert(f != NULL);

    // 1. Initialize STANAG 4607 Header
    STANAG4607Header header;
    memcpy(header.version, "G2.0", 4);
    header.packet_size = sizeof(STANAG4607Header) + sizeof(STANAG4607SegmentHeader) + sizeof(STANAG4607Target);
    memcpy(header.nationality, "US", 2);
    header.classification = 'U'; // Unclassified
    memcpy(header.mission_id, "MISSION-0001", 12);
    memset(header.datetime, 0, 9); // Placeholder timestamp

    // 2. Initialize Segment Header (Type 0x01: Target Track Segment)
    STANAG4607SegmentHeader segment;
    segment.segment_type = 0x01;
    segment.segment_size = sizeof(STANAG4607SegmentHeader) + sizeof(STANAG4607Target);

    // 3. Initialize Target coordinates (from localized spy coordinates: 25.3, -39.9 km translated to geodetic)
    STANAG4607Target target;
    target.target_id = 4567;
    target.latitude = (int32_t)(38.8977 * 1e7);   // 38.8977 N
    target.longitude = (int32_t)(-77.0365 * 1e7); // 77.0365 W
    target.elevation = 44000;                     // 44 meters (44000 mm)

    // Write components to the binary file
    size_t written = fwrite(&header, 1, sizeof(header), f);
    assert(written == sizeof(header));
    written = fwrite(&segment, 1, sizeof(segment), f);
    assert(written == sizeof(segment));
    written = fwrite(&target, 1, sizeof(target), f);
    assert(written == sizeof(target));

    fclose(f);
    printf("[ENCODER] Successfully encoded target track to %s\n", filepath);

    // 4. Verification: Read back and assert data integrity
    FILE *f_in = fopen(filepath, "rb");
    assert(f_in != NULL);

    STANAG4607Header read_header;
    STANAG4607SegmentHeader read_segment;
    STANAG4607Target read_target;

    size_t read_bytes = fread(&read_header, 1, sizeof(read_header), f_in);
    assert(read_bytes == sizeof(read_header));
    read_bytes = fread(&read_segment, 1, sizeof(read_segment), f_in);
    assert(read_bytes == sizeof(read_segment));
    read_bytes = fread(&read_target, 1, sizeof(read_target), f_in);
    assert(read_bytes == sizeof(read_target));

    fclose(f_in);

    // Verify fields
    assert(strncmp(read_header.version, "G2.0", 4) == 0);
    assert(read_header.classification == 'U');
    assert(read_segment.segment_type == 0x01);
    assert(read_target.target_id == 4567);
    
    double restored_lat = (double)read_target.latitude / 1e7;
    double restored_lon = (double)read_target.longitude / 1e7;
    assert(fabs(restored_lat - 38.8977) < 1e-6);
    assert(fabs(restored_lon - (-77.0365)) < 1e-6);

    printf("[VERIFICATION] Latitude restored: %f\n", restored_lat);
    printf("[VERIFICATION] Longitude restored: %f\n", restored_lon);
    printf("[SUCCESS] STANAG 4607 binary encoder and decoder validated successfully!\n");

    return 0;
}
