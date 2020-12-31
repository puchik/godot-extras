tool
extends EditorPlugin

var project_settings := "rendering/quality/lod/"

func _enter_tree() -> void:
	name = "Multi LOD Addon"
	add_custom_type("LODParent", "ImmediateGeometry", preload("lod.gdns"), preload("lod_icon.svg"))
	add_autoload_singleton("LodManager", "res://addons/MultiLOD/LodManager.tscn")

	create_project_setting(project_settings + "global_multiplier", 1.0, TYPE_REAL, "The number all LOD distances will be multiplied by. You can use this to change graphics/geometry settings at runtime. Larger number means longer distances before lowering details.")
	create_project_setting(project_settings + "lod1_multiplier", 1.0, TYPE_REAL, "The number the LOD1 distance will be multiplied by. Use this for finer global LOD multiplier control. Larger number means longer distances before lowering details.")
	create_project_setting(project_settings + "lod2_multiplier", 1.0, TYPE_REAL, "The number the LOD2 distance will be multiplied by. Use this for finer global LOD multiplier control. Larger number means longer distances before lowering details.")
	create_project_setting(project_settings + "lod3_multiplier", 1.0, TYPE_REAL, "The number the LOD3 distance will be multiplied by. Use this for finer global LOD multiplier control. Larger number means longer distances before lowering details.")
	create_project_setting(project_settings + "hide_multiplier", 1.0, TYPE_REAL, "The number the hide distance will be multiplied by. Use this for finer global LOD multiplier control. Larger number means longer distances before lowering details.")
	create_project_setting(project_settings + "unload_multiplier", 1.0, TYPE_REAL, "The number the unload distance will be multiplied by. Use this for finer global LOD multiplier control. Larger number means longer distances before lowering details.")
	create_project_setting(project_settings + "shadow_disable_multiplier", 1.0, TYPE_REAL, "The number the shadow fading out distance will be multiplied by. Use this for finer global LOD multiplier control. Larger number means longer distances before fading the shadow out.")

func _exit_tree():
	remove_custom_type("LODParent")
	remove_autoload_singleton("LodManager")

func create_project_setting(setting : String, default, type : int, hint : String) -> void:
	if not ProjectSettings.has_setting(setting):
		ProjectSettings.set_setting(setting, default)
		ProjectSettings.add_property_info({
			name = setting,
			type = type,
			hint = PROPERTY_HINT_NONE,
			hint_string = hint
		})
