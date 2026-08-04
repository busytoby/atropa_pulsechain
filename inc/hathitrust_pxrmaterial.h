#ifndef HATHITRUST_PXRMATERIAL_H
#define HATHITRUST_PXRMATERIAL_H

/*
 * Compiles specular, roughness, and diffuse maps into unified RenderMan material definitions (PxrDisney bindings).
 * Parameters:
 *   material_id: Target ID of the material block.
 *   diffuse_tex: Diffuse color texture map name.
 *   specular_tex: Specular intensity texture map name.
 *   roughness_tex: Roughness texture map name.
 * Returns:
 *   Allocated string containing the material RIB description, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_generate_pxrmaterial_rib(
    const char *material_id,
    const char *diffuse_tex,
    const char *specular_tex,
    const char *roughness_tex
);

#endif /* HATHITRUST_PXRMATERIAL_H */
