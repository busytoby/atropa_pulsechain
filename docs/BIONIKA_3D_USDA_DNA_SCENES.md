# FORMAL PIXAR USDA SPECIFICATION: 3D DNA-EVOLVED SCENE ASSETS & MATERIALS

This document specifies the Pixar USD ASCII (`.usda`) schema representations and `UsdPreviewSurface` material bindings evolved from participant DNA seeds across each of the 7 scenes.

---

## 1. 3D USDA Scene Hierarchy & Prim Definitions

```usd
#usda 1.0
(
    defaultPrim = "BionikaSymphony3D"
    metersPerUnit = 1.0
    upAxis = "Y"
)

def Xform "BionikaSymphony3D" (
    kind = "assembly"
)
{
    # -------------------------------------------------------------------------
    # SCENE 1: VERSE 1 | Obsidian Silk Hyperbolic Torus (Alpha DNA Seed 0xAD4e)
    # -------------------------------------------------------------------------
    def Mesh "Scene1_ObsidianSilkMesh" (
        prepend apiSchemas = ["MaterialBindingAPI"]
    )
    {
        float3[] extent = [(-350, -120, -350), (350, 120, 350)]
        int[] faceVertexCounts = [4, 4, 4, 4]
        int[] faceVertexIndices = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]
        point3f[] points = [
            (-280, -90, -280), (280, -90, -280), (280, 90, 280), (-280, 90, 280),
            (-200, -60, -200), (200, -60, -200), (200, 60, 200), (-200, 60, 200),
            (-150, -40, -150), (150, -40, -150), (150, 40, 150), (-150, 40, 150),
            (-100, -20, -100), (100, -20, -100), (100, 20, 100), (-100, 20, 100)
        ]
        normal3f[] normals = [(0, 1, 0), (0, 1, 0), (0, 1, 0), (0, 1, 0)] (
            interpolation = "uniform"
        )
        rel material:binding = </BionikaSymphony3D/Materials/ObsidianSilkMaterial>
    }

    # -------------------------------------------------------------------------
    # SCENE 2: CHORUS 1 | Emerald Velvet Gyroscopic Rings (Beta DNA Seed 0xD07B)
    # -------------------------------------------------------------------------
    def Mesh "Scene2_EmeraldVelvetGyroscope" (
        prepend apiSchemas = ["MaterialBindingAPI"]
    )
    {
        float3[] extent = [(-250, -250, -100), (250, 250, 100)]
        int[] faceVertexCounts = [3, 3, 3, 3]
        int[] faceVertexIndices = [0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1]
        point3f[] points = [
            (0, 0, 50), (0, 220, 0), (220, 0, 0), (0, -220, 0), (-220, 0, 0)
        ]
        rel material:binding = </BionikaSymphony3D/Materials/EmeraldVelvetMaterial>
    }

    # -------------------------------------------------------------------------
    # SCENE 3: VERSE 2 | 3D Double-Helix DNA Lattice (Gamma DNA Seed 0xd32c)
    # -------------------------------------------------------------------------
    def Mesh "Scene3_DoubleHelixDnaLattice" (
        prepend apiSchemas = ["MaterialBindingAPI"]
    )
    {
        float3[] extent = [(-140, -140, -350), (140, 140, 350)]
        int[] faceVertexCounts = [4, 4, 4]
        int[] faceVertexIndices = [0, 1, 2, 3, 2, 3, 4, 5, 4, 5, 6, 7]
        point3f[] points = [
            (-140, 0, -300), (140, 0, -300), (0, 140, -150), (0, -140, -150),
            (140, 0, 0), (-140, 0, 0), (0, -140, 150), (0, 140, 150)
        ]
        rel material:binding = </BionikaSymphony3D/Materials/GoldenDamaskMaterial>
    }

    # -------------------------------------------------------------------------
    # SCENE 4: CHORUS 2 | 3D Dual-Trefoil Knot Manifold (Delta DNA Seed 0xed34)
    # -------------------------------------------------------------------------
    def Mesh "Scene4_DualTrefoilManifold" (
        prepend apiSchemas = ["MaterialBindingAPI"]
    )
    {
        float3[] extent = [(-240, -240, -80), (240, 240, 80)]
        int[] faceVertexCounts = [3, 3, 3, 3]
        int[] faceVertexIndices = [0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4]
        point3f[] points = [
            (240, 0, -80), (0, 240, 80), (-240, 0, -80), (0, -240, 80),
            (-240, 0, 80), (0, -240, -80), (240, 0, 80), (0, 240, -80)
        ]
        rel material:binding = </BionikaSymphony3D/Materials/CobaltCopperMaterial>
    }

    # -------------------------------------------------------------------------
    # SCENE 5: VERSE 3 | Geodesic Singularity Icosahedron (Epsilon DNA Seed 0x3e10)
    # -------------------------------------------------------------------------
    def Mesh "Scene5_GeodesicSingularity" (
        prepend apiSchemas = ["MaterialBindingAPI"]
    )
    {
        float3[] extent = [(-160, -160, -160), (160, 160, 160)]
        int[] faceVertexCounts = [3, 3, 3, 3, 3, 3, 3, 3]
        int[] faceVertexIndices = [
            0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5,
            6, 7, 8, 6, 8, 9, 6, 9, 10, 6, 10, 11
        ]
        point3f[] points = [
            (0, 160, 0), (143, 71, 0), (44, 71, 136), (-115, 71, 84), (-115, 71, -84), (44, 71, -136),
            (0, -160, 0), (115, -71, 84), (-44, -71, 136), (-143, -71, 0), (-44, -71, -136), (115, -71, -84)
        ]
        rel material:binding = </BionikaSymphony3D/Materials/CarbonKevlarMaterial>
    }

    # -------------------------------------------------------------------------
    # SCENE 6: CHORUS 3 | Volumetric Babyface Teddy Bear Avatar (Hogan Bank DNA)
    # -------------------------------------------------------------------------
    def Xform "Scene6_TeddyBearAvatar" (
        prepend apiSchemas = ["MaterialBindingAPI"]
    )
    {
        def Sphere "Head"
        {
            double radius = 45.0
            matrix4d xformOp:transform = ( (1,0,0,0), (0,1,0,0), (0,0,1,0), (0, -40, 0, 1) )
            uniform token[] xformOpOrder = ["xformOp:transform"]
        }
        def Sphere "LeftEar"
        {
            double radius = 18.0
            matrix4d xformOp:transform = ( (1,0,0,0), (0,1,0,0), (0,0,1,0), (-35, -80, 0, 1) )
            uniform token[] xformOpOrder = ["xformOp:transform"]
        }
        def Sphere "RightEar"
        {
            double radius = 18.0
            matrix4d xformOp:transform = ( (1,0,0,0), (0,1,0,0), (0,0,1,0), (35, -80, 0, 1) )
            uniform token[] xformOpOrder = ["xformOp:transform"]
        }
        def Sphere "Torso"
        {
            double radius = 60.0
            matrix4d xformOp:transform = ( (1,0,0,0), (0,1,0,0), (0,0,1,0), (0, 40, 0, 1) )
            uniform token[] xformOpOrder = ["xformOp:transform"]
        }
        rel material:binding = </BionikaSymphony3D/Materials/TeddyBearFeltFurMaterial>
    }

    # -------------------------------------------------------------------------
    # SCENE 7: OUTRO | 3D Golden-Ratio Fibonacci Dew Spiral (Chancery Leaf DNA)
    # -------------------------------------------------------------------------
    def Mesh "Scene7_FibonacciDewSpiral" (
        prepend apiSchemas = ["MaterialBindingAPI"]
    )
    {
        float3[] extent = [(-200, -200, -150), (200, 200, 250)]
        int[] faceVertexCounts = [2, 2, 2, 2]
        int[] faceVertexIndices = [0, 1, 1, 2, 2, 3, 3, 4]
        point3f[] points = [
            (0, 0, -150), (30, 20, -50), (-60, -40, 50), (120, 80, 150), (-200, -140, 250)
        ]
        rel material:binding = </BionikaSymphony3D/Materials/GossamerDewMaterial>
    }

    # -------------------------------------------------------------------------
    # Standard USDPreviewSurface Material Schemas
    # -------------------------------------------------------------------------
    def Scope "Materials"
    {
        def Material "ObsidianSilkMaterial"
        {
            def Shader "PbrPreview"
            {
                uniform token info:id = "UsdPreviewSurface"
                color3f inputs:diffuseColor = (0.24, 0.19, 0.32)
                float inputs:roughness = 0.15
                float inputs:metallic = 0.20
            }
        }
        def Material "EmeraldVelvetMaterial"
        {
            def Shader "PbrPreview"
            {
                uniform token info:id = "UsdPreviewSurface"
                color3f inputs:diffuseColor = (0.0, 0.95, 0.55)
                float inputs:roughness = 0.85
                float inputs:metallic = 0.0
            }
        }
        def Material "GoldenDamaskMaterial"
        {
            def Shader "PbrPreview"
            {
                uniform token info:id = "UsdPreviewSurface"
                color3f inputs:diffuseColor = (1.0, 0.84, 0.0)
                float inputs:roughness = 0.30
                float inputs:metallic = 0.85
            }
        }
        def Material "CobaltCopperMaterial"
        {
            def Shader "PbrPreview"
            {
                uniform token info:id = "UsdPreviewSurface"
                color3f inputs:diffuseColor = (0.36, 0.52, 1.0)
                float inputs:roughness = 0.25
                float inputs:metallic = 0.90
            }
        }
        def Material "CarbonKevlarMaterial"
        {
            def Shader "PbrPreview"
            {
                uniform token info:id = "UsdPreviewSurface"
                color3f inputs:diffuseColor = (1.0, 0.20, 0.0)
                float inputs:roughness = 0.60
                float inputs:metallic = 0.10
            }
        }
        def Material "TeddyBearFeltFurMaterial"
        {
            def Shader "PbrPreview"
            {
                uniform token info:id = "UsdPreviewSurface"
                color3f inputs:diffuseColor = (0.85, 0.65, 0.15)
                float inputs:roughness = 0.95
                float inputs:metallic = 0.0
            }
        }
        def Material "GossamerDewMaterial"
        {
            def Shader "PbrPreview"
            {
                uniform token info:id = "UsdPreviewSurface"
                color3f inputs:diffuseColor = (0.41, 0.61, 0.47)
                float inputs:roughness = 0.10
                float inputs:metallic = 0.0
            }
        }
    }
}
```
