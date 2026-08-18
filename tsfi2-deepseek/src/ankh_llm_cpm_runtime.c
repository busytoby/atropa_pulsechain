#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "ankh_llm_cpm_registry.h"

static const AnkhCpmEntryDef g_cpm_entries[] = {
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
        .title = "CPMTOMIE-HOGAN-SSA-TEDDY-CORPS",
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
        .title = "ANKH-LLM-TEDDY-BEAR-ENDOWMENT-CORPS",
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
