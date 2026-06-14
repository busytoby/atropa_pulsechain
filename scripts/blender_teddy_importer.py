bl_info = {
    "name": "TSFi Teddy Bear Procedural Editor",
    "author": "Antigravity",
    "version": (1, 1),
    "blender": (2, 80, 0),
    "location": "View3D > Sidebar > Teddy Bear Editor",
    "description": "Procedurally imports, edits and exports TSFi bear genome binary DNA files.",
    "category": "Object",
}

import bpy
import os
import struct

def get_dna_path():
    script_dir = os.path.dirname(os.path.realpath(__file__))
    return os.path.abspath(os.path.join(script_dir, "../tsfi2-deepseek/assets/bear_genome.dna"))

class OBJECT_OT_import_tsfi_bear(bpy.types.Operator):
    """Import the standard TSFi Teddy Bear model"""
    bl_idname = "object.import_tsfi_bear"
    bl_label = "Import TSFi Bear Mesh"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        script_dir = os.path.dirname(os.path.realpath(__file__))
        obj_path = os.path.abspath(os.path.join(script_dir, "../tsfi2-deepseek/assets/teddy_bear.obj"))

        if not os.path.exists(obj_path):
            self.report({'ERROR'}, f"Could not find teddy_bear.obj at {obj_path}")
            return {'CANCELLED'}

        if hasattr(bpy.ops.import_scene, 'obj'):
            bpy.ops.import_scene.obj(filepath=obj_path)
        else:
            bpy.ops.wm.obj_import(filepath=obj_path)

        imported_objs = [obj for obj in context.selected_objects]
        if not imported_objs:
            self.report({'ERROR'}, "No mesh objects imported.")
            return {'CANCELLED'}

        for obj in imported_objs:
            obj["is_tsfi_bear"] = True
            obj.scale = (context.scene.tsfi_bear_scale, context.scene.tsfi_bear_scale, context.scene.tsfi_bear_scale)
            if obj.type == 'MESH':
                context.view_layer.objects.active = obj
                bpy.ops.object.shade_smooth()

        self.report({'INFO'}, "Successfully imported TSFi bear components!")
        return {'FINISHED'}


class OBJECT_OT_import_tsfi_dna(bpy.types.Operator):
    """Load the 12-byte binary bear_genome.dna"""
    bl_idname = "object.import_tsfi_dna"
    bl_label = "Load genome.dna"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        dna_path = get_dna_path()
        if not os.path.exists(dna_path):
            self.report({'ERROR'}, f"Could not find DNA file at {dna_path}")
            return {'CANCELLED'}

        with open(dna_path, "rb") as f:
            data = f.read(12)
            if len(data) < 12:
                self.report({'ERROR'}, "Invalid DNA file size.")
                return {'CANCELLED'}
            
            dna = struct.unpack("12B", data)
            scene = context.scene
            scene.tsfi_fur_color = (dna[0]/255.0, dna[1]/255.0, dna[2]/255.0)
            scene.tsfi_eye_color = (dna[3]/255.0, dna[4]/255.0, dna[5]/255.0)
            scene.tsfi_sickness = dna[6] / 255.0
            scene.tsfi_bear_scale = dna[7] / 100.0
            scene.tsfi_fur_length = dna[8] / 1000.0
            scene.tsfi_light_angle = (dna[9] / 255.0) * 360.0
            scene.tsfi_breathing_freq = dna[10] / 128.0
            scene.tsfi_twitch_intensity = dna[11] / 255.0

        self.report({'INFO'}, "Loaded TSFi bear genome DNA successfully!")
        return {'FINISHED'}


class OBJECT_OT_export_tsfi_dna(bpy.types.Operator):
    """Save the 12-byte binary bear_genome.dna"""
    bl_idname = "object.export_tsfi_dna"
    bl_label = "Save genome.dna"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        dna_path = get_dna_path()
        scene = context.scene

        fur_r = int(max(0.0, min(1.0, scene.tsfi_fur_color[0])) * 255)
        fur_g = int(max(0.0, min(1.0, scene.tsfi_fur_color[1])) * 255)
        fur_b = int(max(0.0, min(1.0, scene.tsfi_fur_color[2])) * 255)

        eye_r = int(max(0.0, min(1.0, scene.tsfi_eye_color[0])) * 255)
        eye_g = int(max(0.0, min(1.0, scene.tsfi_eye_color[1])) * 255)
        eye_b = int(max(0.0, min(1.0, scene.tsfi_eye_color[2])) * 255)

        base_sickness = int(max(0.0, min(1.0, scene.tsfi_sickness)) * 255)
        base_scale = int(max(0.01, min(2.55, scene.tsfi_bear_scale)) * 100)
        base_fur_length = int(max(0.0, min(0.255, scene.tsfi_fur_length)) * 1000)
        light_angle_deg = int((max(0.0, min(360.0, scene.tsfi_light_angle)) / 360.0) * 255)
        breathing_freq = int(max(0.0, min(2.0, scene.tsfi_breathing_freq)) * 128)
        twitch_intensity = int(max(0.0, min(1.0, scene.tsfi_twitch_intensity)) * 255)

        data = struct.pack(
            "12B",
            fur_r, fur_g, fur_b,
            eye_r, eye_g, eye_b,
            base_sickness,
            base_scale,
            base_fur_length,
            light_angle_deg,
            breathing_freq,
            twitch_intensity
        )

        with open(dna_path, "wb") as f:
            f.write(data)

        self.report({'INFO'}, f"Saved genome DNA to {dna_path}")
        return {'FINISHED'}


class OBJECT_OT_apply_tsfi_fur(bpy.types.Operator):
    """Add hair particle system to selected mesh"""
    bl_idname = "object.apply_tsfi_fur"
    bl_label = "Apply Procedural Fur"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        obj = context.active_object
        if not obj or obj.type != 'MESH':
            self.report({'ERROR'}, "Please select a bear mesh body part.")
            return {'CANCELLED'}

        bpy.ops.object.modifier_add(type='PARTICLE_SYSTEM')
        part_sys = obj.particle_systems[-1]
        part_settings = part_sys.settings

        part_settings.type = 'HAIR'
        part_settings.count = int(25000 * context.scene.tsfi_bear_scale)
        part_settings.hair_length = context.scene.tsfi_fur_length
        part_settings.use_hair_bsdf = True
        
        part_settings.child_type = 'SIMPLE'
        part_settings.rendered_child_count = 35
        part_settings.child_length = 1.0
        part_settings.clump_factor = 0.08
        part_settings.roughness_1 = 0.04
        part_settings.roughness_2 = 0.04

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

        layout.label(text="DNA Data Standards", icon='DNA')
        row = layout.row(align=True)
        row.operator("object.import_tsfi_dna", icon='FILE_REFRESH')
        row.operator("object.export_tsfi_dna", icon='EXPORT')

        layout.separator()

        layout.label(text="Mesh Import Settings", icon='IMPORT')
        layout.prop(scene, "tsfi_bear_scale")
        layout.operator("object.import_tsfi_bear", icon='MESH_MONKEY')

        layout.separator()

        layout.label(text="Gene Editor Rollout", icon='PROPERTIES')
        layout.prop(scene, "tsfi_fur_color")
        layout.prop(scene, "tsfi_eye_color")
        layout.prop(scene, "tsfi_fur_length")
        layout.prop(scene, "tsfi_sickness")
        layout.prop(scene, "tsfi_light_angle")
        layout.prop(scene, "tsfi_breathing_freq")
        layout.prop(scene, "tsfi_twitch_intensity")

        layout.separator()
        layout.operator("object.apply_tsfi_fur", icon='PARTICLES')


def register():
    bpy.types.Scene.tsfi_bear_scale = bpy.props.FloatProperty(name="Scale", default=1.0, min=0.01, max=2.55)
    bpy.types.Scene.tsfi_fur_length = bpy.props.FloatProperty(name="Fur Length", default=0.08, min=0.0, max=0.255)
    bpy.types.Scene.tsfi_fur_color = bpy.props.FloatVectorProperty(name="Fur Color", subtype='COLOR', default=(0.62, 0.44, 0.22), min=0.0, max=1.0)
    bpy.types.Scene.tsfi_eye_color = bpy.props.FloatVectorProperty(name="Eye Color", subtype='COLOR', default=(0.0, 1.0, 0.0), min=0.0, max=1.0)
    bpy.types.Scene.tsfi_sickness = bpy.props.FloatProperty(name="Sickness", default=0.5, min=0.0, max=1.0)
    bpy.types.Scene.tsfi_light_angle = bpy.props.FloatProperty(name="Light Angle", default=135.0, min=0.0, max=360.0)
    bpy.types.Scene.tsfi_breathing_freq = bpy.props.FloatProperty(name="Breath Freq", default=1.0, min=0.0, max=2.0)
    bpy.types.Scene.tsfi_twitch_intensity = bpy.props.FloatProperty(name="Twitch", default=0.5, min=0.0, max=1.0)

    bpy.utils.register_class(OBJECT_OT_import_tsfi_bear)
    bpy.utils.register_class(OBJECT_OT_import_tsfi_dna)
    bpy.utils.register_class(OBJECT_OT_export_tsfi_dna)
    bpy.utils.register_class(OBJECT_OT_apply_tsfi_fur)
    bpy.utils.register_class(VIEW3D_PT_tsfi_bear_panel)


def unregister():
    del bpy.types.Scene.tsfi_bear_scale
    del bpy.types.Scene.tsfi_fur_length
    del bpy.types.Scene.tsfi_fur_color
    del bpy.types.Scene.tsfi_eye_color
    del bpy.types.Scene.tsfi_sickness
    del bpy.types.Scene.tsfi_light_angle
    del bpy.types.Scene.tsfi_breathing_freq
    del bpy.types.Scene.tsfi_twitch_intensity

    bpy.utils.unregister_class(OBJECT_OT_import_tsfi_bear)
    bpy.utils.unregister_class(OBJECT_OT_import_tsfi_dna)
    bpy.utils.unregister_class(OBJECT_OT_export_tsfi_dna)
    bpy.utils.unregister_class(OBJECT_OT_apply_tsfi_fur)
    bpy.utils.unregister_class(VIEW3D_PT_tsfi_bear_panel)


if __name__ == "__main__":
    register()
