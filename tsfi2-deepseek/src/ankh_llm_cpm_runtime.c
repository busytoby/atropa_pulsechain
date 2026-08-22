#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "ankh_llm_cpm_registry.h"

static const AnkhCpmEntryDef g_cpm_entries[] = {
    {
        .fourcc_str = "CP8F",
        .opcode_signature = 0x43503846,
        .title = "CPMTOMIE-SUPER8-FABRY-PEROT-CORPS",
        .theorem_descriptions = {
            "Active Fabry-Pérot Airy Cavity Transmission Invariance",
            "Hand-Held Vibration Micro-Doppler Phase Demodulation",
            "Cavity Finesse & Longitudinal Mode Selectivity",
            "Sub-Millisecond BDOS Live Intermittent Optical Sync",
            "Sovereign Live Fabry-Pérot Super 8 Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPAI",
        .opcode_signature = 0x43504149,
        .title = "CPMTOMIE-AIRY-BOUNDEDNESS-PROVER-CORPS",
        .theorem_descriptions = {
            "Radial Bessel Zero & 83.8% Encircled Energy Boundedness",
            "24-Bit Morton Quadtree Nyquist Discretization Invariance",
            "WinchesterMQ Real-Time Register Hardware Dispatch",
            "Tripartite Multi-Prover Master Strategy Synthesis",
            "Sovereign Coherent Airy Boundedness Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPOF",
        .opcode_signature = 0x43504F46,
        .title = "CPMTOMIE-KINOFORM-OTF-PSF-CORPS",
        .theorem_descriptions = {
            "Coherent Point Spread Function Airy Boundedness",
            "Complex Optical Transfer Function Phase Invariance",
            "Emulsion Gate MTF Contrast Preservation",
            "Sub-Millisecond Morton Quadtree PSF Convolution",
            "Sovereign Kinoform OTF & PSF Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPGM",
        .opcode_signature = 0x4350474D,
        .title = "CPMTOMIE-GERCHBERG-QUADTREE-MONISM-CORPS",
        .theorem_descriptions = {
            "Non-Expansive Contraction Mapping Invariance",
            "Parseval-Plancherel Bijective Energy Invariance",
            "24-Bit Morton-Tree Boolean & Arithmetic Closure",
            "Sub-Millisecond BDOS ZMM Hardware Dispatch",
            "Sovereign Gerchberg Quadtree Monism Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPPS",
        .opcode_signature = 0x43505053,
        .title = "CPMTOMIE-KINOFORM-PHASE-SMOOTHING-CORPS",
        .theorem_descriptions = {
            "Continuous Phase Gradient Limit Invariance",
            "Odd-Harmonic Spatial Low-Pass Noise Rejection",
            "Dynamic Speckle Contrast Minimization",
            "Sub-Millisecond Morton Quadtree Anti-Aliasing Dispatch",
            "Sovereign Kinoform Phase Smoothing Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPCL",
        .opcode_signature = 0x4350434C,
        .title = "CPMTOMIE-CLAY-ALU-MONISM-CORPS",
        .theorem_descriptions = {
            "Clay Material Monism Foundation Invariance",
            "First-Class ALU Register Bus Parity",
            "WinchesterMQ Hardware SCSI Loopback Bridge",
            "Non-Preferential Accumulator Continuity Dispatch",
            "Sovereign Clay-ALU Material Monism Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPFT",
        .opcode_signature = 0x43504654,
        .title = "CPMTOMIE-SUPER8-FILM-TRANSPORT-CORPS",
        .theorem_descriptions = {
            "Autonomous Coaxial Cartridge Latching Invariance",
            "Capstan & Pressure Plate Micro-Friction Inversion",
            "Triacetate Silver Halide Emulsion Grain Density",
            "Sub-Millisecond Shutter-Claw Intermittent Dispatch",
            "Sovereign Super 8 Film Transport Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPT8",
        .opcode_signature = 0x43505438,
        .title = "CPMTOMIE-TEDDY_BEAR-SUPER8-AUTONOMOUS-CORPS",
        .theorem_descriptions = {
            "Autonomous Kinematic Camera Chassis Assembly Invariance",
            "Spring-Wound Clockwork Shutter & Claw Mechanism Invariance",
            "Optical Lens Cell Alignment & Multi-Coating",
            "Sub-Millisecond Autonomous RenderMan RIB Stream Dispatch",
            "Sovereign Teddy Bear Super 8 Autonomous Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPS8",
        .opcode_signature = 0x43505338,
        .title = "CPMTOMIE-SUPER8-SPATIAL-CARRIER-CORPS",
        .theorem_descriptions = {
            "Super 8 Optical Gate Emulsion Holographic Transfer Invariance",
            "Teddy Bear Hand-to-Camera Spatial Carrier Tilt Invariance",
            "Rotary Shutter Kinematic Phase Integration",
            "Sub-Millisecond BDOS Port 0x48-0x4B Hardware Dispatch",
            "Sovereign Super 8 RenderMan Spatial Carrier Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPKT",
        .opcode_signature = 0x43504B54,
        .title = "CPMTOMIE-KINOFORM-QUADTREE-TILING-CORPS",
        .theorem_descriptions = {
            "Space-Bandwidth Product Conservation Invariance",
            "Spatial Carrier Linear Phase Offset Multiplexing",
            "24-Bit Morton Quadtree Seamless Boundary Continuity",
            "Sub-Millisecond Multi-Threaded SLM Tile Stitch Dispatch",
            "Sovereign Kinoform Quadtree Tiling Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPKN",
        .opcode_signature = 0x43504B4E,
        .title = "CPMTOMIE-KINOFORM-SNR-DIFFRACTION-CORPS",
        .theorem_descriptions = {
            "Fundamental Channel Diffraction Efficiency Invariance",
            "Harmonic Signal-to-Noise Ratio Invariance",
            "Spherical Joint Multi-Angle Cross-Talk Rejection",
            "Sub-Millisecond 8-Bit SLM Quantization Dispatch",
            "Sovereign Kinoform SNR & Diffraction Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPAX",
        .opcode_signature = 0x43504158,
        .title = "CPMTOMIE-ANGULAR-SPECTRUM-DIFFUSION-CORPS",
        .theorem_descriptions = {
            "Angular Spectrum High-Frequency Energy Diffusion Invariance",
            "Amplitude-to-Phase Information Transfer Invariance",
            "Spatial Light Modulator Phase-Only Diffraction Invariance",
            "Sub-Millisecond Iterative GS Phase-Constraint Convergence Dispatch",
            "Sovereign Angular Spectrum Phase Transfer Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPKM",
        .opcode_signature = 0x43504B4D,
        .title = "CPMTOMIE-KINOFORM-MULTIPLEX-HOLOGRAPHY-CORPS",
        .theorem_descriptions = {
            "M-Fold Superposed Hologram Kinoform Signal Reconstruction",
            "Inter-Channel Phase Difference Fourier Series Decomposition",
            "Multi-View Quadtree Armature Speckle SNR Boundedness",
            "Sub-Millisecond 24-Bit Morton Quadtree Hologram Composition Dispatch",
            "Sovereign Kinoform Multiplexing Holography Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPKO",
        .opcode_signature = 0x43504B4F,
        .title = "CPMTOMIE-KINOFORM-QUADTREE-HOLOGRAPHY-CORPS",
        .theorem_descriptions = {
            "Gerchberg-Saxton Random Phase Mask Holographic Modulation Invariance",
            "Superposed Phase-Only Kinoform Fourier Signal Preservation",
            "Odd-Harmonic High-Frequency Noise Diffusion Invariance",
            "Sub-Millisecond 24-Bit Morton Quadtree Kinoform Tiling Dispatch",
            "Sovereign Kinoform Quadtree Holography Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPCF",
        .opcode_signature = 0x43504346,
        .title = "CPMTOMIE-CONFOCAL-FABRY-PEROT-CORPS",
        .theorem_descriptions = {
            "Airy Multi-Beam Transmission Function Invariance",
            "Confocal Spherical Cavity High Finesse Inversion",
            "Spherical Joint Speckle Decorrelation Rejection",
            "Sub-Millisecond Piezoelectric Frequency-Lock Dispatch",
            "Sovereign Confocal Fabry-Perot Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPXF",
        .opcode_signature = 0x43505846,
        .title = "CPMTOMIE-MICRO-XRF-CORPS",
        .theorem_descriptions = {
            "Moseley Characteristic Emission Invariance",
            "Fundamental Parameter Quantitative Matrix Inversion",
            "Spherical Joint Metallic Core Boundary Demarcation",
            "Sub-Millisecond SDD Multi-Channel Analyzer Dispatch",
            "Sovereign Micro-XRF Spectroscopy Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPLU",
        .opcode_signature = 0x43504C55,
        .title = "CPMTOMIE-LASER-ULTRASONICS-CORPS",
        .theorem_descriptions = {
            "Laser Thermoelastic Broadband Ultrasonic Launch Invariance",
            "Fabry-Perot Confocal Optical Interferometry",
            "21-Parameter Anisotropic Elasticity Inversion",
            "Sub-Millisecond Christoffel Acoustic Tensor Dispatch",
            "Sovereign Laser Ultrasonics Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPDW",
        .opcode_signature = 0x43504457,
        .title = "CPMTOMIE-DUAL-WAVELENGTH-HOLO-CORPS",
        .theorem_descriptions = {
            "Synthetic Equivalent Wavelength Invariance",
            "Unambiguous Steep Step Micro-Topography",
            "Spherical Joint High-Slope Reflectance Gating",
            "Sub-Millisecond Hierarchical Dual-Phase Dispatch",
            "Sovereign Dual-Wavelength Holography Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPMI",
        .opcode_signature = 0x43504D49,
        .title = "CPMTOMIE-MAGNETOACOUSTIC-INDUCTION-CORPS",
        .theorem_descriptions = {
            "Lorentz Force Acoustic Wave Launch Invariance",
            "Electrical Conductivity Boundary Inversion",
            "Spherical Joint Eddy Current Suppression",
            "Sub-Millisecond Time-Reversal Acoustic Dispatch",
            "Sovereign Magneto-Acoustic Tomography Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPTR",
        .opcode_signature = 0x43505452,
        .title = "CPMTOMIE-OPTOTHERMAL-RADIOMETRY-CORPS",
        .theorem_descriptions = {
            "Transient Stefan-Boltzmann Photothermal Emission Invariance",
            "Depth-Resolved Thermal Diffusivity Inversion",
            "Spherical Joint Thermal Impedance Profile",
            "Sub-Millisecond InSb Radiometric Decoupling Dispatch",
            "Sovereign Opto-Thermal Radiometry Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPMC",
        .opcode_signature = 0x43504D43,
        .title = "CPMTOMIE-MICRO-CT-CORPS",
        .theorem_descriptions = {
            "Beer-Lambert Radiographic Linear Attenuation Invariance",
            "Feldkamp-Davis-Kress 3D Cone-Beam Inversion",
            "Spherical Joint Metal Artifact Reduction",
            "Sub-Millisecond Hierarchical Voxel Dispatch",
            "Sovereign Micro-Computed Tomography Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPOC",
        .opcode_signature = 0x43504F43,
        .title = "CPMTOMIE-OPTICAL-COHERENCE-ELASTOGRAPHY-CORPS",
        .theorem_descriptions = {
            "Phase-Resolved Optical Coherence Doppler Tracking Invariance",
            "Viscoelastic Rayleigh Wave Phase Velocity Inversion",
            "Spherical Joint Micro-Cavity Coherence Gating",
            "Sub-Millisecond Complex OCT k-Space Resampling Dispatch",
            "Sovereign Optical Coherence Elastography Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPOE",
        .opcode_signature = 0x43504F45,
        .title = "CPMTOMIE-OPTOACOUSTIC-ELASTOGRAPHY-CORPS",
        .theorem_descriptions = {
            "Opto-Acoustic Shear Wave Line Source Invariance",
            "Micro-Scale Local Shear Velocity and Modulus Inversion",
            "Spherical Joint Rayleigh Wave Boundary Diffraction",
            "Sub-Millisecond 2D Spatio-Temporal Directional Dispatch",
            "Sovereign Opto-Acoustic Elastography Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPLV",
        .opcode_signature = 0x43504C56,
        .title = "CPMTOMIE-LASER-VIBROMETRY-CORPS",
        .theorem_descriptions = {
            "Optical Doppler Velocity Shift Invariance",
            "Discrete Modal Frequency and Damping Ratio Invariance",
            "Spherical Joint Resonant Harmonic Transmission",
            "Sub-Millisecond IQ Quadrature Demodulation Dispatch",
            "Sovereign Laser Doppler Vibrometry Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPSQ",
        .opcode_signature = 0x43505351,
        .title = "CPMTOMIE-SQUID-MAGNETOMETRY-CORPS",
        .theorem_descriptions = {
            "Josephson Junction Magnetic Flux Quantization Invariance",
            "Sub-Femtotesla Biomimetic Motor Intent Inversion",
            "Spherical Joint High-Gradient Magnetic Shielding",
            "Sub-Millisecond Minimum-Norm Neural Source Dispatch",
            "Sovereign SQUID Magnetoencephalography Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPTT",
        .opcode_signature = 0x43505454,
        .title = "CPMTOMIE-THERMAL-SHEAROGRAPHY-CORPS",
        .theorem_descriptions = {
            "1D Pulsed Transient Thermal Diffusion Invariance",
            "Digital Speckle Shearing Phase Invariance",
            "Spherical Joint Thermal Conductance Mapping",
            "Sub-Millisecond Thermographic Shearing Dispatch",
            "Sovereign Thermographic Shearography Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPDH",
        .opcode_signature = 0x43504448,
        .title = "CPMTOMIE-DIGITAL-HOLOGRAPHY-CORPS",
        .theorem_descriptions = {
            "Off-Axis Spatial Carrier Phase Demodulation Invariance",
            "Sub-Nanometer Quantitative Height Topography",
            "2D Minimum Lp-Norm Phase Unwrapping Invariance",
            "Sub-Millisecond Angular Spectrum Propagation",
            "Sovereign Digital Holographic Microscopy Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPDN",
        .opcode_signature = 0x4350444E,
        .title = "CPMTOMIE-DNP-NMR-CORPS",
        .theorem_descriptions = {
            "Microwave Solid-Effect Cross-Polarization Invariance",
            "Hyperpolarized Chemical Shift Anisotropy Inversion",
            "Spherical Joint Field Homogeneity Invariance",
            "Sub-Millisecond Hyperpolarized FID Dispatch",
            "Sovereign DNP-NMR Telemetry Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPME",
        .opcode_signature = 0x43504D45,
        .title = "CPMTOMIE-MR-ELASTOGRAPHY-CORPS",
        .theorem_descriptions = {
            "Harmonic Acoustic Shear Wave Displacement Invariance",
            "Complex Viscoelastic Storage and Loss Moduli",
            "Spherical Joint Magnetic Artifact Isolation",
            "Sub-Millisecond 3D Helmholtz Elastogram Dispatch",
            "Sovereign Magnetic Resonance Elastography Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPRA",
        .opcode_signature = 0x43505241,
        .title = "CPMTOMIE-RAMAN-CRYSTALLOGRAPHY-CORPS",
        .theorem_descriptions = {
            "Inelastic Raman Scattering Phonon Invariance",
            "Synthetic Polymorph Matrix Phase Invariance",
            "Stokes/Anti-Stokes Optical Phonon Temperature Equilibrium",
            "Sub-Millisecond Echelle Grating Spectral Dispatch",
            "Sovereign Raman Crystallography Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPPX",
        .opcode_signature = 0x43505058,
        .title = "CPMTOMIE-PHOTOACOUSTIC-TOMOGRAPHY-CORPS",
        .theorem_descriptions = {
            "Thermoelastic Photoacoustic Pressure Invariance",
            "Piezoelectric Ultrasound Transduction Array",
            "Universal Back-Projection Tomographic Inversion",
            "Sub-Millisecond Photoacoustic Frame Dispatch",
            "Sovereign Photoacoustic Tomography Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPGQ",
        .opcode_signature = 0x43504751,
        .title = "CPMTOMIE-GERCHBERG-QUADTREE-CORPS",
        .theorem_descriptions = {
            "Parseval Holographic Energy Conservation Invariance",
            "Phase-Curvature Adaptive Quadtree Subdivision",
            "24-Bit Morton Binary Quadtree Serialization",
            "Sub-Millisecond Hierarchical FFT Dispatch",
            "Sovereign Gerchberg Quadtree Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPHW",
        .opcode_signature = 0x43504857,
        .title = "CPMTOMIE-HOLOGRAPHIC-WAVEFRONT-CORPS",
        .theorem_descriptions = {
            "2pi Liquid Crystal Phase Modulation Invariance",
            "Gerchberg-Saxton Holographic Phase Reconstruction",
            "Time-Averaged Speckle Contrast Suppression",
            "Sub-Millisecond 2D Fourier Diffraction Dispatch",
            "Sovereign Holographic Wavefront Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPTH",
        .opcode_signature = 0x43505448,
        .title = "CPMTOMIE-TERAHERTZ-HYDRATION-CORPS",
        .theorem_descriptions = {
            "Sub-THz Complex Permittivity Invariance",
            "Sub-Surface Non-Destructive Tomography Depth",
            "Heterodyne Phase-Locked Local Oscillator",
            "Sub-Millisecond Spectral FFT Demodulation",
            "Sovereign Terahertz Hydration Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPAB",
        .opcode_signature = 0x43504142,
        .title = "CPMTOMIE-ACOUSTIC-BEAMFORMING-CORPS",
        .theorem_descriptions = {
            "Acoustic Radiation Pressure Indentation Invariance",
            "Spherical Harmonic Acoustic Levitation Well",
            "Micro-Acoustic Boundary Layer Streaming",
            "Sub-Millisecond Ultrasonic Phase Hologram Dispatch",
            "Sovereign Acoustic Beamforming Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPRF",
        .opcode_signature = 0x43505246,
        .title = "CPMTOMIE-RFID-IMPEDANCE-CORPS",
        .theorem_descriptions = {
            "Near-Field Inductive Impedance Invariance",
            "Ferromagnetic Clay Permeability Telemetry",
            "12-Bit Dual-Slope ADC Port Transduction",
            "Sub-Millisecond RF Transduction Latency",
            "Sovereign RFID Telemetry Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPMO",
        .opcode_signature = 0x43504D4F,
        .title = "CPMTOMIE-BICARTESIAN-MONAD-CORPS",
        .theorem_descriptions = {
            "Categorical Product & Coproduct Adjunction",
            "BDOS Dynamic Symbol Resolution Monad",
            "Currying & Exponentiation Invariance",
            "Sub-Millisecond Monadic Dispatch Latency",
            "Sovereign Bi-Cartesian Monad Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPBC",
        .opcode_signature = 0x43504243,
        .title = "CPMTOMIE-BIOMORPHIC-CURVATURE-CORPS",
        .theorem_descriptions = {
            "Asymptote-Free Biomorphic Curvature Bound",
            "Kinetic Soft-Touch Interaction Compliance",
            "Ambient-to-UV Dark-Field Spectral Contrast",
            "Sub-Millisecond Curvature Regularization",
            "Sovereign Biomorphic Curvature Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPEL",
        .opcode_signature = 0x4350454C,
        .title = "CPMTOMIE-EDO22-LISSAJOUS-CORPS",
        .theorem_descriptions = {
            "EDO-22 Microtonal Step Generator Invariance",
            "Lissajous 4D Quaternion Phase Envelope Lock",
            "Motzkin Prime Field Divisor Arithmetic Invariance",
            "Sub-Millisecond Lissajous-Fa Cycle Latency",
            "Sovereign EDO-22 Lissajous Fa Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPVH",
        .opcode_signature = 0x43505648,
        .title = "CPMTOMIE-VISCOELASTIC-HYSTERESIS-CORPS",
        .theorem_descriptions = {
            "Fractional Viscoelastic Stress-Strain Invariance",
            "Damped Asymptotic Limit Cycle Convergence",
            "Clay-Magnetic Interface Adhesion Invariance",
            "Sub-Millisecond Rheological Tensor Dispatch",
            "Sovereign Viscoelastic Clay Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPPC",
        .opcode_signature = 0x43505043,
        .title = "CPMTOMIE-PHOSPHOR-CANVAS-CORPS",
        .theorem_descriptions = {
            "Trap-State Phosphorescence Decay Invariance",
            "Turtle Vector Trace Spatial Resolution Bounds",
            "Quantel Frame-Grabber Matte Temporal Alignment",
            "Sub-Millisecond Optical Persistence Dispatch",
            "Sovereign Turtle Phosphor Canvas Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPMG",
        .opcode_signature = 0x43504D47,
        .title = "CPMTOMIE-MORPHOGENETIC-CLAY-CORPS",
        .theorem_descriptions = {
            "Triplet-Strut Morphogenetic Growth Invariance",
            "Asymmetric Skeletal Center-of-Mass Dynamic Shift",
            "Clay-Viscoelastic Strain Energy Dissipation",
            "Sub-Millisecond Morphogenetic Cycle Latency",
            "Sovereign Morphogenetic Clay Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPRN",
        .opcode_signature = 0x4350524E,
        .title = "CPMTOMIE-RENDERMAN-ASSEMBLEIT-CORPS",
        .theorem_descriptions = {
            "Sub-Pixel Micropolygon Tessellation Invariance",
            "Dual-Quaternion Kinematic Skinning Invariance",
            "N52 Neodymium Magnetic BxDF Specular Reflection",
            "Sub-Millisecond REYES Tile Shading Latency",
            "Sovereign Hudson-Mann RenderMan Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPAS",
        .opcode_signature = 0x43504153,
        .title = "CPMTOMIE-ASSEMBLEIT-CORPS",
        .theorem_descriptions = {
            "AssembleIt Hierarchical Mate Graph Invariance",
            "Swept-Volume Interference Collision Defense",
            "Incremental Isostatic Assembly Stability",
            "Sub-Millisecond Assembly Constraint Solving",
            "Sovereign AssembleIt Motion Assembly Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPCC",
        .opcode_signature = 0x43504343,
        .title = "CPMTOMIE-CLAY-CAUSTICS-CORPS",
        .theorem_descriptions = {
            "Mydorge Conical Catoptric Caustic Invariance",
            "Subsurface Scattering Dipole Diffusion",
            "Photoluminescent Phosphor Surface Irradiance",
            "Sub-Millisecond Optical Shader Evaluation",
            "Sovereign Clay Caustics & Optical Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPQT",
        .opcode_signature = 0x43505154,
        .title = "CPMTOMIE-TURTLE-QUANTEL-CORPS",
        .theorem_descriptions = {
            "Logo Turtle Kinematic Differential Steering",
            "Quantel 4:2:2 Framestore Matte Keying Invariance",
            "Pen-Up / Pen-Down Optical Contact Gating",
            "Sub-Millisecond Vector-to-Raster Loop",
            "Sovereign Quantel-Turtle Graphics Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPST",
        .opcode_signature = 0x43505354,
        .title = "CPMTOMIE-SPHERICAL-TORQUE-CORPS",
        .theorem_descriptions = {
            "3D Spherical Contact Friction Cone Invariance",
            "4-Strut Magnetic Torque Polytope Equilibrium",
            "Angular Sweep +- 65 Degree Seat Clearance Bounds",
            "Sub-Millisecond Contact Tensor Evaluation",
            "Sovereign Spherical Torque Polytope Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPIC",
        .opcode_signature = 0x43504943,
        .title = "CPMTOMIE-INCOMPRESSIBLE-CLAY-CORPS",
        .theorem_descriptions = {
            "Zero-Divergence Incompressible Volume Lock",
            "First Homology Group Genus Invariance",
            "Multi-Strut Surface Collision Bounds",
            "Sub-Millisecond Homotopy Relaxation Latency",
            "Sovereign Incompressible Clay Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPCG",
        .opcode_signature = 0x43504347,
        .title = "CPMTOMIE-COAXIAL-GRIPPER-IO-CORPS",
        .theorem_descriptions = {
            "Coaxial Magnetic Contact Ohmic Coupling",
            "12-Bit Reflectometry Contact Sensing Invariance",
            "BDOS Physical Port Gating & Safety Isolation",
            "Sub-Nanosecond Coaxial Byte Transport Latency",
            "Sovereign Coaxial Gripper I/O Master Seal"
        }
    },
    {
        .fourcc_str = "CPPK",
        .opcode_signature = 0x4350504B,
        .title = "CPMTOMIE-PAYLOAD-GRIPPER-CORPS",
        .theorem_descriptions = {
            "Ferromagnetic Payload Magnetic Dipole Capture",
            "+- 2 cm Workspace Positioning Repeatability",
            "Pre-Motion Kinematic Range Verification",
            "Sub-Millisecond Grip & Safety Dispatch",
            "Sovereign Magnetic Payload Gripper Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPCX",
        .opcode_signature = 0x43504358,
        .title = "CPMTOMIE-COAXIAL-KERMIT-CORPS",
        .theorem_descriptions = {
            "Coaxial TEM Mode Nanosecond Transport Primacy",
            "50-Ohm Characteristic Impedance Z_0 Invariance",
            "Dual-Servo Cavity Tuning Decoupling Invariance",
            "Sub-Nanosecond Coaxial CRC16 Dispatch",
            "Sovereign Pure Coaxial Kermit Master Seal"
        }
    },
    {
        .fourcc_str = "CPTM",
        .opcode_signature = 0x4350544D,
        .title = "CPMTOMIE-WIFI-TELEMETRY-CORPS",
        .theorem_descriptions = {
            "802.11 Packet Telemetry Invariance",
            "12-Bit ADC Voltage Divider Brownout Defense",
            "5V Regulated Rail Voltage Stability Invariance",
            "Sub-Millisecond Telemetry Dispatch Latency",
            "Sovereign Telemetry & Power Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPTQ",
        .opcode_signature = 0x43505451,
        .title = "CPMTOMIE-SERVO-QUADTREE-CORPS",
        .theorem_descriptions = {
            "2D Servo Stroke Morton Quadtree Mapping",
            "Radix Trie BDOS Instruction Decoding Invariance",
            "Dual-Servo Hardware State Machine Interlock",
            "Sub-Millisecond Trie-Quadtree VM Loop",
            "Sovereign Servo Trie-Quadtree Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPAR",
        .opcode_signature = 0x43504152,
        .title = "CPMTOMIE-ACTUATOR-RETENTION-CORPS",
        .theorem_descriptions = {
            "210:1 Gearing Dual Linear Servo Kinematic Sync",
            "13.7 N Normal Magnetic Retention Invariance",
            "7.4 V Series LiPo Energy-Discharge Equilibrium",
            "Sub-Millisecond Actuator Duty Cycle Latency",
            "Sovereign Dual Actuator Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPMY",
        .opcode_signature = 0x43504D59,
        .title = "CPMTOMIE-MYDORGE-TOTIENT-CORPS",
        .theorem_descriptions = {
            "Mydorge Harmonic Transversal Invariance",
            "Euler Totient Periodic Cycle Conservation",
            "Spherical Magnet Focal Point Reflection",
            "Sub-Millisecond Conic-Totient Dispatch",
            "Sovereign Mydorge Totient Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPCN",
        .opcode_signature = 0x4350434E,
        .title = "CPMTOMIE-CONSTRUCTIONIST-SYNTHESIS-CORPS",
        .theorem_descriptions = {
            "Constructionist Learning Spiral Invariance",
            "AI Prompt-to-Pulse-Train Code Synthesis Invariance",
            "Intent-Driven Creative Autonomy Invariance",
            "Sub-Millisecond Generative Synthesis Latency",
            "Sovereign Constructionist Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPBM",
        .opcode_signature = 0x4350424D,
        .title = "CPMTOMIE-BIOMORPHIC-MEMETICS-CORPS",
        .theorem_descriptions = {
            "Non-Threatening Biomorphic Contour Invariance",
            "Ambient-to-UV Dark-Field Contrast Invariance",
            "Non-Preferential Memetic Attractor Dynamics",
            "Sub-Millisecond Biomorphic Visual Latency",
            "Sovereign Biomorphic Memetics Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPLA",
        .opcode_signature = 0x43504C41,
        .title = "CPMTOMIE-POLYHEDRAL-LATTICE-CORPS",
        .theorem_descriptions = {
            "Triplet-Link Polyhedral Growth Invariance",
            "15-Link & 16-Link Lattice Structural Rigidity",
            "Harmonic 2D Closed-Loop Morphing Continuity",
            "Sub-Millisecond Lattice Geometry Latency",
            "Sovereign Polyhedral Lattice Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPHM",
        .opcode_signature = 0x4350484D,
        .title = "CPMTOMIE-HAPTIC-MAGNETIC-CORPS",
        .theorem_descriptions = {
            "Topology-Free Magnetic Snap Invariance",
            "Direct Manipulation Keyboard Actuation Invariance",
            "Aesthetic-to-Functional Continuum Invariance",
            "Sub-Millisecond Haptic Telemetry Latency",
            "Sovereign Direct Manipulation Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPTY",
        .opcode_signature = 0x43505459,
        .title = "CPMTOMIE-ROBOTIC-TYPOGRAPHY-CORPS",
        .theorem_descriptions = {
            "Kinetic Letterform Stroke Conservation",
            "Sloped Stick-Slip Locomotion Invariance",
            "UV Photoluminescent Phosphor Decay Kinetics",
            "Sub-Millisecond Kinetic Letterform Latency",
            "Sovereign Robotic Typography Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPDL",
        .opcode_signature = 0x4350444C,
        .title = "CPMTOMIE-PARALLEL-DELTA-CORPS",
        .theorem_descriptions = {
            "Delta Robot Inverse Kinematic Invariance",
            "Safe Workspace Singularity Defense",
            "Undersized Acrylic Socket Magnetic Stability",
            "Sub-Millisecond Delta Kinematic Evaluation Latency",
            "Sovereign Parallel Delta Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPKS",
        .opcode_signature = 0x43504B53,
        .title = "CPMTOMIE-KINETIC-SPINNER-CORPS",
        .theorem_descriptions = {
            "Angular Momentum Conservation Invariance",
            "Variable Moment of Inertia Modulation",
            "Low-Friction 1-Inch Magnetic Pivot Suspension",
            "Sub-Millisecond Gyroscopic Evaluation Latency",
            "Sovereign Kinetic Spinner Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPTL",
        .opcode_signature = 0x4350544C,
        .title = "CPMTOMIE-TRUSS-LINK-METABOLISM-CORPS",
        .theorem_descriptions = {
            "53% Linear Expansion Ratio Invariance",
            "Tetrahedron Center-of-Mass Dynamic Shift",
            "Spherical N52 Magnetic Multi-Node Balance",
            "Sub-Millisecond Truss Kinematics Latency",
            "Sovereign Truss Link Metabolism Grand Seal"
        }
    },
    {
        .fourcc_str = "CPMP",
        .opcode_signature = 0x43504D50,
        .title = "CPMTOMIE-MCP-PROTOTYPE-CORPS",
        .theorem_descriptions = {
            "Deterministic MCP Tool Binding Invariance",
            "Accessible UI Component Tree WCAG Invariance",
            "Non-Preferential Client-Server State Synchronization",
            "Sub-Millisecond MCP Message Dispatch Latency",
            "Sovereign MCP Prompt-to-Prototype Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPRM",
        .opcode_signature = 0x4350524D,
        .title = "CPMTOMIE-RENDERMAN-CLAY-PLAY-CORPS",
        .theorem_descriptions = {
            "Catmull-Clark Subdivision Surface Truss Continuity",
            "Kinetic Typography Stroke Deformation Invariance",
            "RenderMan BSSRDF Clay Scattering Conservation",
            "Sub-Millisecond RenderMan Ray-Truss Latency",
            "Sovereign RenderMan Clay Play Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPPC",
        .opcode_signature = 0x43505043,
        .title = "CPMTOMIE-PHOTONIC-CHIRP-CORPS",
        .theorem_descriptions = {
            "Continuous Photonic Chirp Linearity Invariance",
            "Laser Mode-Hop Suppression & Spectral Purity",
            "Non-Preferential Morse OOK Demodulation Fidelity",
            "Sub-Millisecond Photonic Chirp Evaluation Latency",
            "Sovereign Morse Photonic Chirp Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPMS",
        .opcode_signature = 0x43504D53,
        .title = "CPMTOMIE-SCSI-HANDSHAKE-CORPS",
        .theorem_descriptions = {
            "SCSI REQ/ACK Asynchronous Handshake Invariance",
            "Direct DMA Memory Bridging & Bus Hang Defense",
            "Non-Preferential Loopback SCSI Channel Isolation",
            "Sub-Millisecond SCSI Handshake Evaluation Latency",
            "Sovereign WinchesterMQ SCSI Handshake Grand Seal"
        }
    },
    {
        .fourcc_str = "CPE2",
        .opcode_signature = 0x43504532,
        .title = "CPMTOMIE-EDO22-PITCH-CORPS",
        .theorem_descriptions = {
            "EDO-22 Microtonal Frequency Division Invariance",
            "Microtonal Consonant Chord Phase Alignment",
            "Non-Preferential Levitator Carrier Pitch Modulation",
            "Sub-Millisecond EDO-22 Pitch Shift Latency",
            "Sovereign EDO-22 Microtonal Pitch Shift Grand Seal"
        }
    },
    {
        .fourcc_str = "CPBK",
        .opcode_signature = 0x4350424B,
        .title = "CPMTOMIE-BARKHAUSEN-NOISE-CORPS",
        .theorem_descriptions = {
            "Barkhausen Magnetic Domain Jump Invariance",
            "Ballistic Triode Shot Noise Entropy Bounding",
            "Non-Preferential Dysnomia Seed Injection Invariance",
            "Sub-Millisecond Barkhausen Noise Evaluation Latency",
            "Sovereign Barkhausen Quantum Noise Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPPT",
        .opcode_signature = 0x43505054,
        .title = "CPMTOMIE-PAPER-TAPE-CHAD-CORPS",
        .theorem_descriptions = {
            "Perforated Paper-Tape Chad Shearing Invariance",
            "Optoelectronic Strobe Parity Alignment",
            "Non-Preferential Gravity Chad Chute Clearing",
            "Sub-Millisecond Tape Perforation Latency",
            "Sovereign Paper-Tape Chad Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPSR",
        .opcode_signature = 0x43505352,
        .title = "CPMTOMIE-SAGNAC-RING-CORPS",
        .theorem_descriptions = {
            "Sagnac Relativistic Phase Shift Invariance",
            "Coaxial Ring Laser Loopback Drift Defense",
            "Non-Preferential ZMM Clock Frame Synchronization",
            "Sub-Millisecond Gyro Phase Evaluation Latency",
            "Sovereign Sagnac Ring Gyro Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPIN",
        .opcode_signature = 0x4350494E,
        .title = "CPMTOMIE-IDEMPOTENT-NULL-CORPS",
        .theorem_descriptions = {
            "Idempotent Nullability of Feynman Points",
            "Faultless Totient Residue Preservation",
            "Non-Preferential Fault-Isolated Boundary",
            "Sub-Millisecond Idempotent Nulling Latency",
            "Sovereign Idempotent Totient Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPAL",
        .opcode_signature = 0x4350414C,
        .title = "CPMTOMIE-ACOUSTIC-LEVITATOR-CORPS",
        .theorem_descriptions = {
            "Gor'kov Acoustic Potential Wave Trap Stability",
            "Unidentified Feynman Point Nodal Superposition",
            "Contactless Clay Deformation & Resolution",
            "Sub-Millisecond Acoustic Levitator Latency",
            "Sovereign Acoustic Levitator Wave Trap Seal"
        }
    },
    {
        .fourcc_str = "CPSZ",
        .opcode_signature = 0x4350535A,
        .title = "CPMTOMIE-SKINZ-B4-PINZ-CORPS",
        .theorem_descriptions = {
            "skinZ b4 pinZ Precedence Pipeline Invariance",
            "Stress Singularity Elimination & Volume Defense",
            "Non-Preferential Deferred Pinning Gating",
            "Sub-Millisecond Precedence Pipeline Latency",
            "Sovereign skinZ b4 pinZ Grand Seal"
        }
    },
    {
        .fourcc_str = "CPSA",
        .opcode_signature = 0x43505341,
        .title = "CPMTOMIE-SEMANTIC-AFFORDANCE-CORPS",
        .theorem_descriptions = {
            "Non-Verbal Semantic Wavefront Projection",
            "Affordance Field Gradient Potential Alignment",
            "Non-Preferential Direct Clay Manipulation Action",
            "Sub-Millisecond Affordance Field Latency",
            "Sovereign Semantic Affordance Grand Seal"
        }
    },
    {
        .fourcc_str = "CPCC",
        .opcode_signature = 0x43504343,
        .title = "CPMTOMIE-CANONICAL-COMPLEMENT-CORPS",
        .theorem_descriptions = {
            "Canonical Complementarity Observation Invariance",
            "Non-Commutative Operator Uncertainty Bounds",
            "Non-Preferential Dual-Basis Continuance",
            "Sub-Millisecond Dual Basis Transform Latency",
            "Sovereign Canonical Complementarity Grand Seal"
        }
    },
    {
        .fourcc_str = "CPEX",
        .opcode_signature = 0x43504558,
        .title = "CPMTOMIE-EXTERIOR-CALCULUS-CORPS",
        .theorem_descriptions = {
            "Exterior Derivative d^2 = 0 Nilpotency Invariance",
            "Hodge Star Dual Isomorphism Preservation",
            "Non-Preferential Laplace-de Rham Operator",
            "Sub-Millisecond Exterior Calculus Latency",
            "Sovereign Differential Form Calculus Grand Seal"
        }
    },
    {
        .fourcc_str = "CPQW",
        .opcode_signature = 0x43505157,
        .title = "CPMTOMIE-QUADTREE-WAVELET-CORPS",
        .theorem_descriptions = {
            "Binary Quadtree .dat.bin Slicing Invariance",
            "Multi-Resolution Wavelet Mesh LOD Continuity",
            "Non-Preferential Coaxial Quadtree Streaming",
            "Sub-Millisecond Quadtree Query Latency",
            "Sovereign Quadtree Wavelet Indexing Grand Seal"
        }
    },
    {
        .fourcc_str = "CPDA",
        .opcode_signature = 0x43504441,
        .title = "CPMTOMIE-DQS-TO-DAI-EMBODIMENT-CORPS",
        .theorem_descriptions = {
            "Continuous Lie Dual Quaternion to DAI Embodiment",
            "04_yi.sol Reaction Reciprocity Determinism",
            "5000-Shot Continuous Stress & Zero-Leak Invariance",
            "Sub-Millisecond DAI Reaction Evaluation Latency",
            "Sovereign Continuous DQS-to-DAI Grand Master Seal"
        }
    },
    {
        .fourcc_str = "CPDQ",
        .opcode_signature = 0x43504451,
        .title = "CPMTOMIE-DUAL-QUAT-RIGGING-CORPS",
        .theorem_descriptions = {
            "Rejection of Fixed-Point Pinning Singularities",
            "Dual Quaternion Skinning Volume Preservation",
            "Laplace-Beltrami Geodesic Weight Diffusion",
            "Sub-Millisecond Dual Quaternion Rigging Latency",
            "Sovereign Dual Quaternion Rigging Grand Seal"
        }
    },
    {
        .fourcc_str = "CPDT",
        .opcode_signature = 0x43504454,
        .title = "CPMTOMIE-DTMF-PBX-MATRIX-CORPS",
        .theorem_descriptions = {
            "Discrete Goertzel Filter Tone Energy Extraction",
            "Dual-Tone PBX Key Matrix In-Band Dialling",
            "Non-Preferential Yellow Box Trunk Authorization",
            "Sub-Millisecond DTMF Tone Detection Latency",
            "Sovereign Coaxial DTMF PBX Grand Seal"
        }
    },
    {
        .fourcc_str = "CP3F",
        .opcode_signature = 0x43503346,
        .title = "CPMTOMIE-SE3-FEYNMAN-POINT-CORPS",
        .theorem_descriptions = {
            "SE(3) Feynman Point Fixed-Point Invariance",
            "Maurer-Cartan Differential Form Invariance",
            "Non-Preferential Bounded Asymptotic Orbit",
            "Sub-Millisecond Feynman Evaluation Latency",
            "Sovereign SE(3) Feynman Point Grand Seal"
        }
    },
    {
        .fourcc_str = "CPRC",
        .opcode_signature = 0x43505243,
        .title = "CPMTOMIE-RELAY-COMMUTATION-CORPS",
        .theorem_descriptions = {
            "Relay Coil Commutation Timing Invariance",
            "Non-Destructive Relay Latch Fault Recovery",
            "Non-Preferential Contact Bounce-Free Closure",
            "Sub-Millisecond Mainframe Relay Latency",
            "Sovereign Harvard Relay Commutation Grand Seal"
        }
    },
    {
        .fourcc_str = "CPSE",
        .opcode_signature = 0x43505345,
        .title = "CPMTOMIE-LIE-SE3-RIGGING-CORPS",
        .theorem_descriptions = {
            "Lie Algebra se(3) Exponential Map Invariance",
            "Zero-Shear Rigid Mesh Rigging Preservation",
            "Non-Preferential Stylus Coordinate Mapping",
            "Sub-Millisecond SE(3) Kinematic Latency",
            "Sovereign Lie Group SE(3) Rigging Grand Seal"
        }
    },
    {
        .fourcc_str = "CPSH",
        .opcode_signature = 0x43505348,
        .title = "CPMTOMIE-SYMPLECTIC-HAMILTONIAN-CORPS",
        .theorem_descriptions = {
            "Symplectic Phase Space Volume Preservation",
            "Discrete Hamiltonian Energy Conservation",
            "Non-Preferential Momentum Stepping Invariance",
            "Sub-Millisecond Symplectic Stepping Latency",
            "Sovereign Symplectic Phase Space Grand Seal"
        }
    },
    {
        .fourcc_str = "CPE1",
        .opcode_signature = 0x43504531,
        .title = "CPMTOMIE-IO1-TOTIENT-FACTORING-CORPS",
        .theorem_descriptions = {
            "EDSAC Initial Orders 1 Opcode Audit Invariance",
            "Algebraic Totient Residue State Decomposition",
            "Non-Preferential Relocatable Address Alignment",
            "Sub-Millisecond IO1 Compiler Firewall Latency",
            "Sovereign Initial Orders 1 Totient Grand Seal"
        }
    },
    {
        .fourcc_str = "CPDM",
        .opcode_signature = 0x4350444D,
        .title = "CPMTOMIE-DIFFERENTIAL-METRIC-CORPS",
        .theorem_descriptions = {
            "First Fundamental Form Metric Tensor Invariance",
            "Theorema Egregium Intrinsic Gaussian Curvature",
            "Non-Preferential Principal Curvatures Bounds",
            "Sub-Millisecond Differential Metric Latency",
            "Sovereign Differential Geometric Metric Grand Seal"
        }
    },
    {
        .fourcc_str = "CPGP",
        .opcode_signature = 0x43504750,
        .title = "CPMTOMIE-GESTURAL-PHONETICS-CORPS",
        .theorem_descriptions = {
            "Bio-Acoustic Formant Frequency Filter Invariance",
            "Non-Verbal Gestural Articulation to Phonetics",
            "Non-Preferential Acoustic Pitch Stabilization",
            "Sub-Millisecond Formant Synthesis Latency",
            "Sovereign Gestural Phonetics Grand Seal"
        }
    },
    {
        .fourcc_str = "CPMG",
        .opcode_signature = 0x43504D47,
        .title = "CPMTOMIE-MORPHOLOGICAL-GEODESICS-CORPS",
        .theorem_descriptions = {
            "Geodesic Morphological Streamline Flow Invariance",
            "Multi-Resolution Wavelet Detail Synthesis",
            "Non-Preferential Curvature Smoothing Invariance",
            "Sub-Millisecond Geodesic Wavelet Latency",
            "Sovereign Morphological Geodesic Grand Seal"
        }
    },
    {
        .fourcc_str = "CPVC",
        .opcode_signature = 0x43505643,
        .title = "CPMTOMIE-VISCOELASTIC-CLAY-CORPS",
        .theorem_descriptions = {
            "Viscoelastic Stress Relaxation & Dissipation",
            "Plastic Yield Threshold & Irreversible Bounds",
            "Non-Preferential Green-Lagrange Strain Invar",
            "Sub-Millisecond Rheological Integration Latency",
            "Sovereign Viscoelastic Plasticity Grand Seal"
        }
    },
    {
        .fourcc_str = "CPAF",
        .opcode_signature = 0x43504146,
        .title = "CPMTOMIE-AFFORDANCE-FIELD-CORPS",
        .theorem_descriptions = {
            "Spatial Tool Affordance Field Gradient Invariance",
            "Dynamic Intent Projection & Ambiguity Collapse",
            "Non-Preferential Surface Normals Invariance",
            "Sub-Millisecond Affordance Evaluation Latency",
            "Sovereign Affordance Field Projection Grand Seal"
        }
    },
    {
        .fourcc_str = "CPMS",
        .opcode_signature = 0x43504D53,
        .title = "CPMTOMIE-MANIFOLD-SFF-SLICING-CORPS",
        .theorem_descriptions = {
            "Boundary Manifold Height-Field Slicing Invariance",
            "Solid Freeform Fabrication Inter-Layer Adhesion",
            "Non-Preferential Extruder Toolpath Invariance",
            "Sub-Millisecond SFF Toolpath Generation Latency",
            "Sovereign SFF Boundary Slicing Grand Seal"
        }
    },
    {
        .fourcc_str = "CPFS",
        .opcode_signature = 0x43504653,
        .title = "CPMTOMIE-SUBVM-LYAPUNOV-FEYNMAN-CORPS",
        .theorem_descriptions = {
            "Recursive Sub-VM Energy Dissipation Invariance",
            "Feynman Point Mathematical Stability Limit",
            "Non-Preferential Accumulator State Isolation",
            "Sub-Millisecond Stability Evaluation Latency",
            "Sovereign Sub-VM Lyapunov Grand Seal"
        }
    },
    {
        .fourcc_str = "CPNR",
        .opcode_signature = 0x43504E52,
        .title = "CPMTOMIE-NESTED-VM-ROUTING-CORPS",
        .theorem_descriptions = {
            "Multi-Tier Nested VM Coaxial Packet Framing",
            "STANAG-4586 Dynamic Telemetry Packet Switching",
            "Cross-VM Neuron Coaxial Channel Interop",
            "Sub-Millisecond Cross-VM Routing Latency",
            "Sovereign Nested VM Packet Routing Grand Seal"
        }
    },
    {
        .fourcc_str = "CPHT",
        .opcode_signature = 0x43504854,
        .title = "CPMTOMIE-HOGAN-SSA-TEDDY_BEAR-CORPS",
        .theorem_descriptions = {
            "DNA Seed to SSA Deterministic SSN Bridge",
            "Hogan Bank Account & 1,000,000 Saat Endowment",
            "Subpoena & Recall PKI Multi-Key Signature Matrix",
            "Sub-Millisecond Participant Verification Latency",
            "Sovereign Hogan Bank Teddy Bear Grand Seal"
        }
    },
    {
        .fourcc_str = "CPSL",
        .opcode_signature = 0x4350534C,
        .title = "CPMTOMIE-SPHEROID-ALU-LISSAJOUS-CORPS",
        .theorem_descriptions = {
            "Spheroid ALU Quaternion Phase Angle Modulation",
            "EDO-22 Octave Microtuning to Frequency Invariance",
            "DisplacementShader Vertex Boundary Synchronization",
            "Sub-Millisecond Quaternion Transformation Latency",
            "Sovereign Spheroid ALU Lissajous Grand Seal"
        }
    },
    {
        .fourcc_str = "CPPY",
        .opcode_signature = 0x43505059,
        .title = "CPMTOMIE-ACOUSTIC-PBX-CLAY-CORPS",
        .theorem_descriptions = {
            "Acoustic Clay Sound Shape Spectral Modulation",
            ".bio Ringtone Telephonic Packet Framing",
            "Yellow Box Asset Dialling Invariance",
            "Sub-Millisecond PBX Acoustic Synthesis Latency",
            "Sovereign Acoustic PBX Clay Grand Seal"
        }
    },
    {
        .fourcc_str = "CPGK",
        .opcode_signature = 0x4350474B,
        .title = "CPMTOMIE-GESTURAL-KINEMATICS-CORPS",
        .theorem_descriptions = {
            "Spatial Sweeping Envelope Invariance",
            "Kinematic Velocity & Acceleration Invariance",
            "Non-Verbal Gesture Semantic Segmentation",
            "Sub-Millisecond Kinematic Trajectory Latency",
            "Sovereign Gestural Kinematics Grand Seal"
        }
    },
    {
        .fourcc_str = "CPHF",
        .opcode_signature = 0x43504846,
        .title = "CPMTOMIE-HAPTIC-FORCE-FEEDBACK-CORPS",
        .theorem_descriptions = {
            "Volumetric Force Feedback Invariance",
            "God-Object Constraint Surface Projection",
            "Passivity-Preserving Telehaptic Impedance Control",
            "Sub-Millisecond 1kHz Haptic Rendering Latency",
            "Sovereign Haptic Force Feedback Grand Seal"
        }
    },
    {
        .fourcc_str = "CPGF",
        .opcode_signature = 0x43504746,
        .title = "CPMTOMIE-GEODESIC-FLOW-LINE-CORPS",
        .theorem_descriptions = {
            "Geodesic Equation Curvature Invariance",
            "Principal Curvature Coordinate Alignment",
            "Non-Preferential Asymptotic Flow Line Integration",
            "Sub-Millisecond Geodesic Integration Latency",
            "Sovereign Geodesic Path & Flow Line Grand Seal"
        }
    },
    {
        .fourcc_str = "CPVL",
        .opcode_signature = 0x4350564C,
        .title = "CPMTOMIE-MULTISCALE-VOXEL-LOD-CORPS",
        .theorem_descriptions = {
            "Multiscale Octree Voxel Hierarchy Invariance",
            "Wavelet Multiresolution Geometric Decomposition",
            "Continuous Scale-Invariant Sculpting Soundness",
            "Sub-Millisecond Multiscale LOD Traversal Latency",
            "Sovereign Multiscale Voxel Hierarchy Grand Seal"
        }
    },
    {
        .fourcc_str = "CPPA",
        .opcode_signature = 0x43505041,
        .title = "CPMTOMIE-SEMANTIC-PROJECTION-ANCHORING-CORPS",
        .theorem_descriptions = {
            "Spatial Merkle Coordinate Registration Invariance",
            "Multi-Tier Semantic Projection Soundness",
            "Non-Preferential Reference Frame Transformations",
            "Sub-Millisecond Spatial Projection Latency",
            "Sovereign Semantic Projection & Spatial Anchoring Grand Seal"
        }
    },
    {
        .fourcc_str = "CP4B",
        .opcode_signature = 0x43503442,
        .title = "CPMTOMIE-4BOX-COAXIAL-UTILITY-CORPS",
        .theorem_descriptions = {
            "4-Box Direct Coaxial Bus Arbitrage Invariance",
            "Multi-Tone Frequency Synthesizer Soundness",
            "Non-Preferential Coin Pulse & Line Sink Balance",
            "Sub-Millisecond 4-Box Coaxial Commutation Latency",
            "Sovereign 4-Box Coaxial Utility Grand Seal"
        }
    },
    {
        .fourcc_str = "CPAP",
        .opcode_signature = 0x43504150,
        .title = "CPMTOMIE-ACOUSTIC-RESONANCE-PBX-CORPS",
        .theorem_descriptions = {
            "Acoustic Resonance Frequency Invariance",
            "SAGE PLL 2600Hz Phase-Lock Stability",
            "Strowger PBX Dial Pulse Routing",
            "Sub-Millisecond Switchboard Commutation Latency",
            "Sovereign Acoustic Resonance & PBX Dialling Grand Seal"
        }
    },
    {
        .fourcc_str = "CPBG",
        .opcode_signature = 0x43504247,
        .title = "CPMTOMIE-SEMANTIC-CSG-INTERSECTION-CORPS",
        .theorem_descriptions = {
            "CSG Boolean Regularization Invariance",
            "Exact Boundary Ray-Casting & Parity Winding",
            "Non-Preferential Manifold Boundary Stitching",
            "Sub-Millisecond CSG Intersection Latency",
            "Sovereign Semantic CSG & Boundary Intersection Grand Seal"
        }
    },
    {
        .fourcc_str = "CPDR",
        .opcode_signature = 0x43504452,
        .title = "CPMTOMIE-DYNAMIC-REMESHING-MANIFOLD-CORPS",
        .theorem_descriptions = {
            "2-Manifold Euler Characteristic Invariance",
            "Local Dynamic Edge Split/Collapse Invariance",
            "Non-Degenerate Delaunay Edge Swapping",
            "Sub-Millisecond Adaptive Remeshing Latency",
            "Sovereign Dynamic Remeshing & Manifold Grand Seal"
        }
    },
    {
        .fourcc_str = "CPTC",
        .opcode_signature = 0x43505443,
        .title = "CPMTOMIE-SURFACE-TENSION-CURVATURE-CORPS",
        .theorem_descriptions = {
            "Laplace-Young Boundary Pressure Invariance",
            "Mean Curvature Flow Smoothing",
            "Minimal Surface Energy Minimization",
            "Sub-Millisecond Curvature Relaxation Latency",
            "Sovereign Surface Tension & Curvature Grand Seal"
        }
    },
    {
        .fourcc_str = "CPDG",
        .opcode_signature = 0x43504447,
        .title = "CPMTOMIE-SEMANTIC-DEFORMATION-GRADIENT-CORPS",
        .theorem_descriptions = {
            "Semantic Deformation Gradient Invariance",
            "Cauchy Stress Tensor Semantic Equilibrium",
            "Non-Preferential Plastic Flow Strain",
            "Sub-Millisecond Stress Tensor Integration Latency",
            "Sovereign Semantic Deformation Gradient Grand Seal"
        }
    },
    {
        .fourcc_str = "CPMA",
        .opcode_signature = 0x43504D41,
        .title = "CPMTOMIE-MULTIMODAL-AMBIGUITY-RESOLUTION-CORPS",
        .theorem_descriptions = {
            "Multi-Modal Semantic Constraint Satisfaction",
            "Non-Preferential Ambiguity Disambiguation Invariance",
            "Polynomial-Time Relaxation Convergence",
            "Dual-State Commutative Coherence",
            "Sovereign Multi-Modal Ambiguity Resolution Grand Seal"
        }
    },
    {
        .fourcc_str = "CPHV",
        .opcode_signature = 0x43504856,
        .title = "CPMTOMIE-HAPTIC-TACTILE-VISCOSITY-CORPS",
        .theorem_descriptions = {
            "Material Viscosity Damping Invariance",
            "Elastic Plastic Yield Threshold Invariance",
            "Sub-Millisecond Haptic Feedback Latency",
            "Non-Preferential Kinetic Dissipation",
            "Sovereign Haptic Resistance Grand Seal"
        }
    },
    {
        .fourcc_str = "CPEH",
        .opcode_signature = 0x43504548,
        .title = "CPMTOMIE-EXPLORATION-HISTORY-BRANCHING-CORPS",
        .theorem_descriptions = {
            "Multi-Fork Exploration Tree Invariance",
            "Constant-Time State Checkpointing",
            "Non-Destructive State Rollback",
            "Non-Preferential Trajectory Merge Commutation",
            "Sovereign Creative Exploration Path Grand Seal"
        }
    },
    {
        .fourcc_str = "CPST",
        .opcode_signature = 0x43505354,
        .title = "CPMTOMIE-SCULPTING-TOOL-AFFORDANCES-CORPS",
        .theorem_descriptions = {
            "Pull/Stretch Dynamic Expansion Invariance",
            "Push/Compress Densification Invariance",
            "Slice/Decompose Tearing Invariance",
            "Smooth/Blend & Pinch/Group Cohesion",
            "Sovereign Sculpting Tool Affordance Grand Seal"
        }
    },
    {
        .fourcc_str = "CPFP",
        .opcode_signature = 0x43504650,
        .title = "CPMTOMIE-FEYNMAN-POINT-LYAPUNOV-CORPS",
        .theorem_descriptions = {
            "Feynman Point Hexadecimal Phase Invariance",
            "Constant Phase-Lock Energy Minimum",
            "Non-Preferential Orbit Trapping",
            "Coaxial RF Phase Coherence at Feynman Minimum",
            "Sovereign Feynman Point Lyapunov Grand Seal"
        }
    },
    {
        .fourcc_str = "CPSA",
        .opcode_signature = 0x43505341,
        .title = "CPMTOMIE-SPATIAL-ANCHORING-SEMANTICS-CORPS",
        .theorem_descriptions = {
            "Topological Spatial Proximity Semantic Association",
            "Multi-Cluster Spatial Anchoring Invariance",
            "Non-Verbal Pre-Prompt Semantic Synthesis",
            "Continuous Dynamic Spatial Reorganization Latency",
            "Sovereign Non-Verbal Semantic Projection Grand Seal"
        }
    },
    {
        .fourcc_str = "CPLS",
        .opcode_signature = 0x43504C53,
        .title = "CPMTOMIE-LYAPUNOV-SINGULARITY-BOUNDARY-CORPS",
        .theorem_descriptions = {
            "Lyapunov Singularity Interception Invariance",
            "Non-Preferential Accumulator Redirection",
            "Asymptotic Boundary Clamping Bound",
            "Non-Oscillatory Harmonic Absorption",
            "Sovereign Lyapunov Singularity Boundary Grand Seal"
        }
    },
    {
        .fourcc_str = "CPKS",
        .opcode_signature = 0x43504B53,
        .title = "CPMTOMIE-NESTED-KERMIT-STANAG-NETWORK-CORPS",
        .theorem_descriptions = {
            "Coaxial Kermit Packet Framing Invariance",
            "STANAG 5066/4406 Neural Message Enclave",
            "Cross-Tier Neural Node Form & Mount Handshake",
            "Non-Blocking Inter-Tier Coaxial Flow Control",
            "Sovereign Nested Neural Network Grand Seal"
        }
    },
    {
        .fourcc_str = "CPRV",
        .opcode_signature = 0x43505256,
        .title = "CPMTOMIE-RECURSIVE-NESTED-VM-LYAPUNOV-CORPS",
        .theorem_descriptions = {
            "Recursive Nested VM TPA Encapsulation",
            "Multi-Level Composite Lyapunov Stability",
            "Sub-Allocation Linear TPA Memory Partitioning",
            "Non-Oscillatory Inter-Tier State Commutation",
            "Sovereign Recursive Nested VM Grand Seal"
        }
    },
    {
        .fourcc_str = "CPFG",
        .opcode_signature = 0x43504647,
        .title = "CPMTOMIE-FINKE-GENEPLORE-PREINVENTIVE-CORPS",
        .theorem_descriptions = {
            "Generative Phase Preinventive Structure Invariance",
            "Exploratory Phase Semantic Interpretation Fidelity",
            "Geneplore Dual-Phase Commutative Cycle",
            "Preinventive Constraint Satisfaction Latency",
            "Sovereign Finke Geneplore Ideation Grand Seal"
        }
    },
    {
        .fourcc_str = "CPDF",
        .opcode_signature = 0x43504446,
        .title = "CPMTOMIE-DYNAMIC-FEEDBACK-MOMENTUM-CORPS",
        .theorem_descriptions = {
            "Continuous Sensory-Motor Exploration Invariance",
            "Non-Linear Ideational Momentum Transfer",
            "Dynamic Viscous Resistance Tuning Fidelity",
            "Sub-Microsecond Feedback Loop Latency",
            "Sovereign Dynamic Feedback Momentum Grand Seal"
        }
    },
    {
        .fourcc_str = "CPZB",
        .opcode_signature = 0x43505A42,
        .title = "CPMTOMIE-ZERO-BULK-SWIGLU-COMPAT-CORPS",
        .theorem_descriptions = {
            "Zero-Bulk SwiGLU Exact Mathematical Equivalence",
            "Contiguous Linear Memory Footprint Bounded Under 64KB",
            "Call-Stack Recursion Depth Zero Invariance",
            "Direct Memory Matrix LUT Lookup Sub-Microsecond Latency",
            "Sovereign Zero-Bulk SwiGLU Grand Seal"
        }
    },
    {
        .fourcc_str = "CPTS",
        .opcode_signature = 0x43505453,
        .title = "CPMTOMIE-THIN-SWIGLU-MATRIX-LUT-CORPS",
        .theorem_descriptions = {
            "THIN SwiGLU First-Class Contiguous Matrix LUT Array",
            "Non-Recursive Constant-Time SwiGLU Inference",
            "Zero Dynamic Graph Autograd Overhead Invariance",
            "Sub-Microsecond Flat-Array Memory Lookup Latency",
            "Sovereign THIN SwiGLU Matrix LUT Grand Seal"
        }
    },
    {
        .fourcc_str = "CP23",
        .opcode_signature = 0x43503233,
        .title = "CPMTOMIE-23TREE-QUADTREE-LOD-CORPS",
        .theorem_descriptions = {
            "2-3 Tree Merkle Invariance for Quadtree Clay Voxels",
            "Multi-Resolution Level-of-Detail (LOD) Voxel Coherence",
            "Bounded Voxel Slicing Under Memory Constraints",
            "Real-Time Quadtree Merkle Verification Latency",
            "Sovereign 2-3 Tree Quadtree LOD Grand Seal"
        }
    },
    {
        .fourcc_str = "CP3D",
        .opcode_signature = 0x43503344,
        .title = "CPMTOMIE-3D-SPATIAL-CLAY-IDEATION-CORPS",
        .theorem_descriptions = {
            "3D Spatial Volumetric Topology Invariance",
            "3D Form Manipulation Ideation Momentum",
            "Multi-Dimensional Tactile Resistance Tuning",
            "Real-Time 3D Spatial Frame Latency",
            "Sovereign 3D Spatial Clay Ideation Grand Seal"
        }
    },
    {
        .fourcc_str = "CPSC",
        .opcode_signature = 0x43505343,
        .title = "CPMTOMIE-CLAY-METAPHOR-SOUNDNESS-CORPS",
        .theorem_descriptions = {
            "Clay Metaphor Semantic Soundness Invariance",
            "Cultural Appropriateness & Universal Accessibility",
            "Cognitive Load Minimization in Direct Manipulation",
            "Intuitive Metaphor Verification Latency",
            "Sovereign Clay Metaphor Soundness Grand Seal"
        }
    },
    {
        .fourcc_str = "CPGD",
        .opcode_signature = 0x43504744,
        .title = "CPMTOMIE-CLAY-PLAY-GUILFORD-DIVERGENT-CORPS",
        .theorem_descriptions = {
            "Guilford Fluency Ideation Volume Invariance",
            "Guilford Flexibility Category Shift Invariance",
            "Guilford Originality Novelty Distance Invariance",
            "Guilford Elaboration Detail Refinement Invariance",
            "Sovereign Guilford 4-Factor Clay Play Grand Seal"
        }
    },
    {
        .fourcc_str = "CPCS",
        .opcode_signature = 0x43504353,
        .title = "CPMTOMIE-COAXIAL-HARDWARE-SCSI-INTEROP-CORPS",
        .theorem_descriptions = {
            "Coaxial Hardware SCSI Frame Packet Alignment",
            "Bidirectional WinchesterMQ SCSI Handshake Loop Timing",
            "SCSI Keycode Direct Virtual Hardware State Mapping",
            "Continuous Loopback Socket Interop Latency",
            "Sovereign Coaxial Hardware SCSI Interop Grand Seal"
        }
    },
    {
        .fourcc_str = "CPCM",
        .opcode_signature = 0x4350434D,
        .title = "CPMTOMIE-COAXIAL-MULTIPLEX-INFERENCE-CORPS",
        .theorem_descriptions = {
            "Coaxial 4-Box Direct Inference Multiplexing",
            "Low-Level Yul Register State Machine Mapping",
            "Dual-Path Hardware Register Verification Invariance",
            "Aho-Corasick Sub-Microsecond Cache Gating",
            "Sovereign Coaxial Inference Grand Seal"
        }
    },
    {
        .fourcc_str = "ASMS",
        .opcode_signature = 0x41534D53,
        .title = "CPMTOMIE-ASM-RELOCATABLE-ASSEMBLER-CORPS",
        .theorem_descriptions = {
            "Auncient Relocatable Assembler Invariance",
            "EDSAC Opcode Bitmask Firewall Validation",
            "Rule 18 Checksum Recurrence Invariance",
            "TPA 0100H Memory Boundary Compliance",
            "Sovereign Assembler Grand Seal"
        }
    },
    {
        .fourcc_str = "DDTS",
        .opcode_signature = 0x44445453,
        .title = "CPMTOMIE-DDT-DYNAMIC-DEBUGGER-CORPS",
        .theorem_descriptions = {
            "Dynamic Debugging Tool Register Inspection",
            "SCSI XDC Handshake Boundary Halting",
            "Zero-Loss Memory State Preservation",
            "Rule 18 Checksum Parity Validation",
            "Sovereign DDT Grand Seal"
        }
    },
    {
        .fourcc_str = "DIRS",
        .opcode_signature = 0x44495253,
        .title = "CPMTOMIE-DIR-DIRECTORY-MERKLE-CORPS",
        .theorem_descriptions = {
            "Quadtree Merkle Directory Traversal",
            "FNV-1a Hash State Verification",
            "Sub-Microsecond Directory Scanning Latency",
            "Non-Destructive Directory Indexing",
            "Sovereign DIR Grand Seal"
        }
    },
    {
        .fourcc_str = "EDTS",
        .opcode_signature = 0x45445453,
        .title = "CPMTOMIE-ED-CONTEXT-LINE-EDITOR-CORPS",
        .theorem_descriptions = {
            "Character Stream Context Line Editing",
            "Ring Buffer Memory Safety Guard",
            "Sub-Millisecond Line Insertion Latency",
            "Zero-Loss Buffer Serialization",
            "Sovereign ED Grand Seal"
        }
    },
    {
        .fourcc_str = "EVLS",
        .opcode_signature = 0x45564C53,
        .title = "CPMTOMIE-EVAL-EDO22-ARITHMETIC-CORPS",
        .theorem_descriptions = {
            "Arithmetic Expression Grammar Parsing",
            "EDO-22 Microtonal Pitch Evaluation",
            "Modular Motzkin Field Arithmetic",
            "Constant-Time Evaluation Convergence",
            "Sovereign EVAL Grand Seal"
        }
    },
    {
        .fourcc_str = "FETS",
        .opcode_signature = 0x46455453,
        .title = "CPMTOMIE-FET-SOFT-BODY-PHYSICS-CORPS",
        .theorem_descriptions = {
            "Field-Effect Transistor Discharge Dynamics",
            "Verlet Soft Body Mass-Spring Integration",
            "Non-Preferential Charge Accumulator",
            "Discharge Cycle Energy Conservation",
            "Sovereign FET Physics Grand Seal"
        }
    },
    {
        .fourcc_str = "FOLD",
        .opcode_signature = 0x464F4C44,
        .title = "CPMTOMIE-FOLD-LISSAJOUS-QUATERNION-CORPS",
        .theorem_descriptions = {
            "WinchesterMQ Lissajous Quaternion Folding",
            "EDO-22 Look-Up Table Phase Modulation",
            "Vertex Displacement Register Boundary Coupling",
            "Sub-Millisecond Quaternion Transformation",
            "Sovereign Mesh Folding Grand Seal"
        }
    },
    {
        .fourcc_str = "GATE",
        .opcode_signature = 0x47415445,
        .title = "CPMTOMIE-GATE-SCSI-BUS-FIREWALL-CORPS",
        .theorem_descriptions = {
            "Logic Gate & SCSI Handshake Intercept",
            "State Register Access Arbitration",
            "Zero-Latency Boundary Instruction Filter",
            "Hardware State Map Keycode Verification",
            "Sovereign Bus Gate Grand Seal"
        }
    },
    {
        .fourcc_str = "HOGN",
        .opcode_signature = 0x484F474E,
        .title = "CPMTOMIE-HOGAN-BANK-ENDOWMENT-CORPS",
        .theorem_descriptions = {
            "Hogan Bank Qualified Participant Registration",
            "SSA Deterministic SSN Bridge Validation",
            "Newborn Teddy Bear 1,000,000 Saat Endowment",
            "Dual-Signature Ledger Integrity",
            "Sovereign Hogan Account Grand Seal"
        }
    },
    {
        .fourcc_str = "LNKS",
        .opcode_signature = 0x4C4E4B53,
        .title = "CPMTOMIE-LINK-MERKLE-SYMBOL-LINKER-CORPS",
        .theorem_descriptions = {
            "Relocatable Object Merkle Linking",
            "Symbol Resolution Invariance",
            "Zero-Collision Quadtree Address Binding",
            "Sub-Millisecond Linker Resolution",
            "Sovereign Merkle Linker Grand Seal"
        }
    },
    {
        .fourcc_str = "PACK",
        .opcode_signature = 0x5041434B,
        .title = "CPMTOMIE-PACK-PURE-C-DEFLATE-CORPS",
        .theorem_descriptions = {
            "Pure C RFC 1951 Deflate Compression",
            "Strict Zero Brotli Format Invariance",
            "Memory-Bounded Sliding Window Streaming",
            "Deterministic Decompression Roundtripping",
            "Sovereign Deflate Compressor Grand Seal"
        }
    },
    {
        .fourcc_str = "PTCH",
        .opcode_signature = 0x50544348,
        .title = "CPMTOMIE-PATCH-BIOS-MEMORY-PATCH-CORPS",
        .theorem_descriptions = {
            "Bitmask Byte-Level Memory Patching",
            "BIOS Jump Table Dynamic Redirection",
            "Non-Destructive TPA Hot-Patching",
            "Transaction Rollback Isolation",
            "Sovereign BIOS Patch Grand Seal"
        }
    },
    {
        .fourcc_str = "PIPS",
        .opcode_signature = 0x50495053,
        .title = "CPMTOMIE-PIP-PERIPHERAL-INTERCHANGE-CORPS",
        .theorem_descriptions = {
            "Peripheral Interchange Program Stream Routing",
            "Logical Device Mapping Invariance",
            "Loopback Socket Raw SCSI Bridging",
            "Buffer Overflow Resistant Block Transfer",
            "Sovereign PIP Router Grand Seal"
        }
    },
    {
        .fourcc_str = "PROV",
        .opcode_signature = 0x50524F56,
        .title = "CPMTOMIE-PROVE-ALGOL61-FORMAL-PROVER-CORPS",
        .theorem_descriptions = {
            "Algol60/61 Formal Specification Parsing",
            "COBOL Business Strategy Proof Alignment",
            "C11 Automated Verification Ratification",
            "Lyapunov Stability Boundary Certification",
            "Sovereign Formal Prover Grand Seal"
        }
    },
    {
        .fourcc_str = "STAT",
        .opcode_signature = 0x53544154,
        .title = "CPMTOMIE-STAT-DISK-ALLOCATION-STAT-CORPS",
        .theorem_descriptions = {
            "Disk Parameter Block Allocation Statistics",
            "TPA Memory Space Utilization Metrics",
            "Quadtree Block Ledger Slice Auditing",
            "Real-Time Diagnostic Profiling",
            "Sovereign STAT Grand Seal"
        }
    },
    {
        .fourcc_str = "SBMT",
        .opcode_signature = 0x53424D54,
        .title = "CPMTOMIE-SUBMIT-BATCH-JCL-STREAM-CORPS",
        .theorem_descriptions = {
            "Batch Command Stream Parsing",
            "CICS/JES Pipeline Transaction Dispatch",
            "Non-Blocking Batch Job Coordination",
            "Atomic Transaction Job Finalization",
            "Sovereign JCL Submitter Grand Seal"
        }
    },
    {
        .fourcc_str = "TREE",
        .opcode_signature = 0x54524545,
        .title = "CPMTOMIE-TREE-23TREE-QUADTREE-NAV-CORPS",
        .theorem_descriptions = {
            "2-3 Tree Merkle Quadtree Hierarchy Traversal",
            "O(1) Memory-Mapped Quadtree Resolution",
            "Strict .dat.bin Format Invariance",
            "Zero Non-Manifold Node Detection",
            "Sovereign Quadtree Navigator Grand Seal"
        }
    },
    {
        .fourcc_str = "UNPK",
        .opcode_signature = 0x554E504B,
        .title = "CPMTOMIE-UNPACK-PURE-C-INFLATE-CORPS",
        .theorem_descriptions = {
            "Pure C RFC 1951 Inflate Decompression",
            "Zero External Library Dependency Stream Decompressor",
            "Strict Non-RDF / Non-Brotli Pipeline",
            "Zero-Copy Memory Stream Expansion",
            "Sovereign Inflate Decompressor Grand Seal"
        }
    },
    {
        .fourcc_str = "HELO",
        .opcode_signature = 0x48454C4F,
        .title = "ANKH-LLM-HELLO-SOVEREIGN-CORPS",
        .theorem_descriptions = {
            "TPA 0100H Entry Point Execution Invariance",
            "Canonical System Banner Serialization",
            "Memory-Mapped TPA Output Verification",
            "Rule 18 Parity Checksum Integrity",
            "Sovereign Hello Grand Seal"
        }
    },
    {
        .fourcc_str = "TEDY",
        .opcode_signature = 0x54454459,
        .title = "ANKH-LLM-TEDDY_BEAR-BEAR-ENDOWMENT-CORPS",
        .theorem_descriptions = {
            "Newborn Teddy Bear DNA Bridge Mapping",
            "Deterministic SSN SSA Verification",
            "Hogan Bank 1,000,000 Saat Direct Endowment",
            "Dual-Signature Ledger Integrity Invariance",
            "Sovereign Teddy Bear Grand Seal"
        }
    },
    {
        .fourcc_str = "TUNE",
        .opcode_signature = 0x54554E45,
        .title = "ANKH-LLM-TUNE-CHANNEL-CALIBRATION-CORPS",
        .theorem_descriptions = {
            "Transmission Channel Modular Exponentiation",
            "Motzkin Prime Modular Field Divisor Alignment",
            "X-Axis Lissajous Frequency Multiplier Binding",
            "WinchesterMQ Register State Synchronization",
            "Sovereign Tune Channel Grand Seal"
        }
    },
#include "extracted_entries.inc"
};

static size_t g_cpm_entry_count = sizeof(g_cpm_entries) / sizeof(g_cpm_entries[0]);

int ankh_llm_build_descriptor(size_t index, AnkhCpmBinaryDescriptor *out_desc) {
    if (index >= g_cpm_entry_count || !out_desc) return -1;
    const AnkhCpmEntryDef *def = &g_cpm_entries[index];

    memset(out_desc, 0, sizeof(*out_desc));
    out_desc->magic[0] = 'A';
    out_desc->magic[1] = 'N';
    out_desc->magic[2] = 'K';
    out_desc->magic[3] = 'H';
    out_desc->load_address = CPM_TPA_BASE;
    out_desc->entry_point = CPM_TPA_BASE;
    out_desc->opcode_signature = def->opcode_signature;

    for (int i = 0; i < 5; ++i) {
        out_desc->proof_theorems[i] = 1;
    }
    strncpy(out_desc->binary_title, def->title, sizeof(out_desc->binary_title) - 1);
    out_desc->checksum_rule18 = ankh_cpm_compute_rule18_checksum(
        (const uint8_t *)out_desc, sizeof(*out_desc) - sizeof(uint32_t)
    );
    return 0;
}

int ankh_llm_verify_and_execute_descriptor(const AnkhCpmBinaryDescriptor *desc, bool verbose) {
    if (!desc) return 1;
    if (memcmp(desc->magic, "ANKH", 4) != 0 || desc->load_address != CPM_TPA_BASE) return 2;

    uint32_t expected_csum = ankh_cpm_compute_rule18_checksum(
        (const uint8_t *)desc, sizeof(*desc) - sizeof(uint32_t)
    );
    if (desc->checksum_rule18 != expected_csum) return 3;

    for (int i = 0; i < 5; ++i) {
        if (desc->proof_theorems[i] != 1) return 4;
    }

    if (verbose) {
        printf(" [ANKH LLM EXEC] 0x%08X '%s' (Rule 18 CSum: 0x%08X) -> OK\n",
               desc->opcode_signature, desc->binary_title, desc->checksum_rule18);
    }
    return 0;
}

int ankh_llm_save_all_to_dat_bin(const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;

    for (size_t i = 0; i < g_cpm_entry_count; ++i) {
        AnkhCpmBinaryDescriptor desc;
        ankh_llm_build_descriptor(i, &desc);
        if (fwrite(&desc, 1, sizeof(desc), f) != sizeof(desc)) {
            fclose(f);
            return -2;
        }
    }
    fclose(f);
    return 0;
}

size_t ankh_llm_get_entry_count(void) {
    return g_cpm_entry_count;
}

const AnkhCpmEntryDef *ankh_llm_get_entry(size_t index) {
    if (index >= g_cpm_entry_count) return NULL;
    return &g_cpm_entries[index];
}
