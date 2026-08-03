#ifndef TSFI_DEPTHOFFIELD_H
#define TSFI_DEPTHOFFIELD_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    double focal_distance;
    double lens_radius;
    double target_z; // TARG focal coordinate boundary
    bool wavefront_coding_enabled; // Co-design EDoF toggle
    double wavefront_alpha; // Cubic phase plate parameter
} TSFiDepthOfField;

// Initialize the DepthOfField context
void tsfi_depthoffield_init(TSFiDepthOfField *dof, double focal_distance, double lens_radius, double target_z);

// Calculates the bokeh blur radius coefficient for a given geometry vertex Z depth
double tsfi_depthoffield_eval_blur(const TSFiDepthOfField *dof, double z_depth);

// Resolves target configuration from Z-machine virtual execution bounds
bool tsfi_depthoffield_resolve_zmachine(TSFiDepthOfField *dof, uint32_t zmachine_address);

// Updates focus configuration based on active shot index for cinematic sequences
void tsfi_depthoffield_set_shot(TSFiDepthOfField *dof, int shot_index);

// Fast 1D separable post-process Gaussian convolution replacing multi-sample lens bokeh
void tsfi_depthoffield_bokeh_replace_gaussian(const double *input_image, double *output_image, int width, int height, double coc_radius);

// Configures the camera parameters for 1.85:1 Super 8 widescreen aspect ratios
void tsfi_depthoffield_set_super8_vaesen(TSFiDepthOfField *dof, int width, int *height);

// Enables wavefront coding to achieve defocus-invariant blur
void tsfi_depthoffield_enable_wavefront_coding(TSFiDepthOfField *dof, bool enable, double alpha);

// Performs Wiener deconvolution approximation to restore wavefront-coded images
void tsfi_depthoffield_wiener_deconvolve(const double *input_image, double *output_image, int width, int height, double noise_signal_ratio);

// Performs channel-specific Wiener deconvolution to align and restore blurred color planes
void tsfi_depthoffield_wiener_deconvolve_chromatic(const double *input_image, double *output_image, int width, int height, double noise_signal_ratio, int channel);

// Runs an end-to-end joint optimization inner loop to minimize Mean Squared Error (MSE) on the restored image
double tsfi_depthoffield_optimize_joint_loop(TSFiDepthOfField *dof, const double *original_image, const double *blurred_image, int width, int height, int max_iterations);

// Runs a resource-constrained joint optimization loop balancing MSE against computation power and lens cost
double tsfi_depthoffield_optimize_constrained(TSFiDepthOfField *dof, const double *original_image, const double *blurred_image, int width, int height, double power_budget_mw, double *final_mse);

// Calculates chromatic defocus blur coefficients for specific wavelength channels (0=Red, 1=Green, 2=Blue)
double tsfi_depthoffield_eval_chromatic_blur(const TSFiDepthOfField *dof, double z_depth, int channel);




// Registers a dynamic camera in the thread-safe interop registry to avoid mocks
void tsfi_depthoffield_register_zmachine(uint32_t zmachine_address, double focal_distance, double lens_radius, double target_z);


#endif // TSFI_DEPTHOFFIELD_H

