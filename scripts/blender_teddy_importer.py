bl_info = {
    "name": "TSFi Teddy Bear Procedural Editor",
    "author": "Antigravity",
    "version": (1, 0),
    "blender": (2, 80, 0),
    "location": "View3D > Sidebar > Teddy Bear Editor",
    "description": "Procedurally imports and configures the TSFi teddy bear model with fur particle systems.",
    "category": "Object",
}

import bpy
import os

class OBJECT_OT_import_tsfi_bear(bpy.types.Operator):
    """Import the standard TSFi Teddy Bear model"""
    bl_idname = "object.import_tsfi_bear"
    bl_label = "Import TSFi Bear Mesh"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        # Locate the OBJ file relative to this script's path
        script_dir = os.path.dirname(os.path.realpath(__file__))
        obj_path = os.path.abspath(os.path.join(script_dir, "../tsfi2-deepseek/assets/teddy_bear.obj"))

        if not os.path.exists(obj_path):
            self.report({'ERROR'}, f"Could not find teddy_bear.obj at {obj_path}")
            return {'CANCELLED'}

        # Import OBJ
        if hasattr(bpy.ops.import_scene, 'obj'):
            bpy.ops.import_scene.obj(filepath=obj_path)
        else:
            # Blender 4.0+ usage
            bpy.ops.wm.obj_import(filepath=obj_path)

        # Retrieve imported bear objects (usually starts with teddy_bear)
        imported_objs = [obj for obj in context.selected_objects]
        if not imported_objs:
            self.report({'ERROR'}, "No mesh objects imported.")
            return {'CANCELLED'}

        # Tag objects as bear components
        for obj in imported_objs:
            obj["is_tsfi_bear"] = True
            obj.scale = (context.scene.tsfi_bear_scale, context.scene.tsfi_bear_scale, context.scene.tsfi_bear_scale)
            
            # Setup materials and shade smooth
            if obj.type == 'MESH':
                context.view_layer.objects.active = obj
                bpy.ops.object.shade_smooth()

        self.report({'INFO'}, f"Successfully imported TSFi bear components!")
        return {'FINISHED'}


class OBJECT_OT_apply_tsfi_fur(bpy.types.Operator):
    """Add hair particle system to the selected bear body mesh"""
    bl_idname = "object.apply_tsfi_fur"
    bl_label = "Apply Procedural Fur"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        obj = context.active_object
        if not obj or obj.type != 'MESH':
            self.report({'ERROR'}, "Please select a bear mesh body part.")
            return {'CANCELLED'}

        # Create hair particle modifier
        bpy.ops.object.modifier_add(type='PARTICLE_SYSTEM')
        part_sys = obj.particle_systems[-1]
        part_settings = part_sys.settings

        part_settings.type = 'HAIR'
        part_settings.count = context.scene.tsfi_fur_density
        part_settings.hair_length = context.scene.tsfi_fur_length
        part_settings.use_hair_bsdf = True
        
        # Enable children for thick fur
        part_settings.child_type = 'SIMPLE'
        part_settings.rendered_child_count = 30
        part_settings.child_length = 1.0
        part_settings.clump_factor = 0.1
        part_settings.roughness_1 = 0.05
        part_settings.roughness_2 = 0.05

        self.report({'INFO'}, f"Applied procedural fur modifier to {obj.name}.")
        return {'FINISHED'}


class VIEW3D_PT_tsfi_bear_panel(bpy.types.Panel):
    """UI Panel in View3D Sidebar"""
    bl_label = "TSFi Teddy Bear Editor"
    bl_idname = "VIEW3D_PT_tsfi_bear_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Teddy Bear Editor'

    def draw(self, context):
        layout = self.layout
        scene = context.scene

        layout.label(text="Import Controls", icon='IMPORT')
        layout.prop(scene, "tsfi_bear_scale")
        layout.operator("object.import_tsfi_bear", icon='MESH_MONKEY')

        layout.separator()

        layout.label(text="Procedural Fur Controls", icon='PARTICLE_DATA')
        layout.prop(scene, "tsfi_fur_length")
        layout.prop(scene, "tsfi_fur_density")
        layout.operator("object.apply_tsfi_fur", icon='PARTICLES')


def register():
    bpy.types.Scene.tsfi_bear_scale = bpy.props.FloatProperty(
        name="Import Scale",
        description="Global scale factor for the imported model",
        default=1.0,
        min=0.01,
        max=100.0
    )
    bpy.types.Scene.tsfi_fur_length = bpy.props.FloatProperty(
        name="Fur Length",
        description="Length of the procedural hair fibers",
        default=0.08,
        min=0.005,
        max=5.0
    )
    bpy.types.Scene.tsfi_fur_density = bpy.props.IntProperty(
        name="Base Hair Count",
        description="Number of parent hair guides generated",
        default=2500,
        min=10,
        max=100000
    )

    bpy.utils.register_class(OBJECT_OT_import_tsfi_bear)
    bpy.utils.register_class(OBJECT_OT_apply_tsfi_fur)
    bpy.utils.register_class(VIEW3D_PT_tsfi_bear_panel)


def unregister():
    del bpy.types.Scene.tsfi_bear_scale
    del bpy.types.Scene.tsfi_fur_length
    del bpy.types.Scene.tsfi_fur_density

    bpy.utils.unregister_class(OBJECT_OT_import_tsfi_bear)
    bpy.utils.unregister_class(OBJECT_OT_apply_tsfi_fur)
    bpy.utils.unregister_class(VIEW3D_PT_tsfi_bear_panel)


if __name__ == "__main__":
    register()
