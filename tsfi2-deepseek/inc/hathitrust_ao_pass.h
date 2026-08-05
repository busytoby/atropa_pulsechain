#ifndef HATHITRUST_AO_PASS_H
#define HATHITRUST_AO_PASS_H

/*
 * Generates RenderMan RIB setup statements configuring an ambient occlusion render pass
 * to capture detailed shadows between page joints.
 * Returns:
 *   Allocated string containing RIB commands, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_generate_ao_pass_rib(void);

#endif /* HATHITRUST_AO_PASS_H */
