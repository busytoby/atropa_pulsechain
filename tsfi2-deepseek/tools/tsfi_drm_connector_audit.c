#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

void print_topology(int fd) {
    drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
    drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);

    drmModeRes *res = drmModeGetResources(fd);
    if (!res) return;

    printf("\n--- Physical DRM Topology Audit ---\n");

    // 1. Audit CRTCs (Hardware Display Controllers)
    printf("\n[CRTCs: %d]\n", res->count_crtcs);
    for (int i = 0; i < res->count_crtcs; i++) {
        drmModeCrtc *crtc = drmModeGetCrtc(fd, res->crtcs[i]);
        if (crtc) {
            printf("  CRTC ID: %u | Active: %s | Mode: %dx%d @ %dHz | Buffer ID: %u\n",
                   crtc->crtc_id, crtc->mode_valid ? "YES" : "NO",
                   crtc->mode.hdisplay, crtc->mode.vdisplay, crtc->mode.vrefresh, crtc->buffer_id);
            drmModeFreeCrtc(crtc);
        }
    }

    // 2. Audit Connectors (Physical Monitor Ports)
    printf("\n[Connectors: %d]\n", res->count_connectors);
    for (int i = 0; i < res->count_connectors; i++) {
        drmModeConnector *conn = drmModeGetConnector(fd, res->connectors[i]);
        if (conn) {
            printf("  Connector ID: %u | Type: %u | Status: %s | Enc: %u\n",
                   conn->connector_id, conn->connector_type,
                   conn->connection == DRM_MODE_CONNECTED ? "CONNECTED" : "DISCONNECTED",
                   conn->encoder_id);
            
            // Check for explicit "Non-Desktop" property (Required by wlroots for leasing)
            drmModeObjectProperties *props = drmModeObjectGetProperties(fd, conn->connector_id, DRM_MODE_OBJECT_CONNECTOR);
            if (props) {
                for (uint32_t j = 0; j < props->count_props; j++) {
                    drmModePropertyRes *prop = drmModeGetProperty(fd, props->props[j]);
                    if (prop) {
                        if (strcmp(prop->name, "non-desktop") == 0) {
                            printf("    -> Property 'non-desktop': %llu (0 = Desktop, 1 = Leasable VR)\n", (unsigned long long)props->prop_values[j]);
                        }
                        drmModeFreeProperty(prop);
                    }
                }
                drmModeFreeObjectProperties(props);
            }
            drmModeFreeConnector(conn);
        }
    }

    // 3. Audit Planes (Hardware Memory Layers)
    drmModePlaneRes *plane_res = drmModeGetPlaneResources(fd);
    if (plane_res) {
        printf("\n[Planes: %d]\n", plane_res->count_planes);
        for (uint32_t i = 0; i < plane_res->count_planes; i++) {
            drmModePlane *plane = drmModeGetPlane(fd, plane_res->planes[i]);
            if (plane) {
                // If CRTC ID is 0, the plane is unassigned and technically free to lease
                printf("  Plane ID: %u | Bound to CRTC: %u | Allowed CRTCs: 0x%08x\n",
                       plane->plane_id, plane->crtc_id, plane->possible_crtcs);
                drmModeFreePlane(plane);
            }
        }
        drmModeFreePlaneResources(plane_res);
    }

    drmModeFreeResources(res);
}

int main() {
    printf("=== TSFi Raw Hardware Lease Diagnostic ===\n");

    int fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (fd < 0) fd = open("/dev/dri/card1", O_RDWR | O_CLOEXEC);
    
    if (fd < 0) {
        printf("[FAIL] Could not open DRM device.\n");
        return 1;
    }

    print_topology(fd);

    close(fd);
    return 0;
}
