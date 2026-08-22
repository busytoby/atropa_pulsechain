# Furry TeddyBear Ballet Materials & Texgen Setup

This document details the procedural texture coordinate generation (texgen), standard USD material previews, and visual styling used to achieve a soft, furry/felt texture on the teddy bear and its sewn red heart.

## 1. Procedural Texture Coordinate Generation (Texgen)
The individual component USDA layers (`teddy_bear_body.usda`, `teddy_bear_head.usda`, etc.) include procedural coordinate generation mapping vertex indices:

```usd
    texcoord2f[] primvars:st = [
        (0.0000, 0.0000),
        (0.0000, 0.1429),
        ...
        (1.0000, 1.0000)
    ] (
        interpolation = "vertex"
    )
```

## 2. Standard Material Previews
Within the main composed `teddy_bear_ballet_scene.usda`, two distinct materials are defined using standard `UsdPreviewSurface` preview shaders and bound to the geometry:

* **Looks/FurMaterial**: Set to a soft brown color with roughness `0.95` and `0.0` metallic to represent fuzzy felt/fur.
* **Looks/ClothMaterial**: Set to a bright red color with roughness `0.85` and bound to the `SewnHeart` geometry to represent a sewn fabric/cloth texture.

## 3. Rendered Preview Reference
Below is the target premium rendering of the furry teddy bear ballet character:

![Fuzzy TeddyBear Ballet Preview](/home/mariarahel/.gemini/antigravity-cli/brain/6d0129c5-eb0b-4333-a95f-e0f48861d972/fuzzy_teddy_bear_ballet_1785108020764.jpg)
