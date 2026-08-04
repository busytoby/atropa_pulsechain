#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * RenderMan RunProgram Dynamic Procedural helper.
 * Parses character bounding coordinates from stdin and emits corresponding 
 * 3D polygons (RIB subdivision meshes) representing typographic geometry.
 */
int main(void) {
    char line[1024];
    
    // Read input lines from RenderMan renderer containing: HPOS VPOS WIDTH HEIGHT
    while (fgets(line, sizeof(line), stdin)) {
        int hpos = 0, vpos = 0, width = 0, height = 0;
        if (sscanf(line, "%d %d %d %d", &hpos, &vpos, &width, &height) != 4) {
            continue;
        }
        
        // Translate coordinates to normalized RIB 3D floating space
        double x1 = hpos / 1000.0;
        double y1 = vpos / 1000.0;
        double x2 = (hpos + width) / 1000.0;
        double y2 = (vpos + height) / 1000.0;
        
        // Output dynamic Catmull-Clark subdivision mesh representing the geometry block
        printf("AttributeBegin\n");
        printf("  SubdivisionMesh \"catmull-clark\" [4] [0 1 2 3] [\"interpolateboundary\"] [0 0] [] [] ");
        printf("\"P\" [%.4f %.4f 0  %.4f %.4f 0  %.4f %.4f 0  %.4f %.4f 0]\n",
               x1, -y1, x2, -y1, x2, -y2, x1, -y2);
        printf("AttributeEnd\n");
        fflush(stdout);
    }
    
    return 0;
}
