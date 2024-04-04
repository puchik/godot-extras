@tool
extends EditorPlugin
const AUTOLOAD_NAME = "ImportanceLODManager"

func _enter_tree():
	# Initialization of the plugin goes here.
	add_autoload_singleton(AUTOLOAD_NAME, "res://addons/importance_lod/lod_manager.tscn")
	ProjectSettings.set_setting("performance/importance_lod/global_multiplier", 1.0)
	ProjectSettings.set_setting("performance/importance_lod/lod1_multiplier", 1.0)
	ProjectSettings.set_setting("performance/importance_lod/lod2_multiplier", 1.0)
	ProjectSettings.set_setting("performance/importance_lod/lod3_multiplier", 1.0)
	ProjectSettings.set_setting("performance/importance_lod/hide_multiplier", 1.0)
	ProjectSettings.set_setting("performance/importance_lod/unload_multiplier", 1.0)
	pass


func _exit_tree():
	# Clean-up of the plugin goes here.
	remove_autoload_singleton(AUTOLOAD_NAME)
	ProjectSettings.set_setting("performance/importance_lod/global_multiplier", null)
	ProjectSettings.set_setting("performance/importance_lod/lod1_multiplier", null)
	ProjectSettings.set_setting("performance/importance_lod/lod2_multiplier", null)
	ProjectSettings.set_setting("performance/importance_lod/lod3_multiplier", null)
	ProjectSettings.set_setting("performance/importance_lod/hide_multiplier", null)
	ProjectSettings.set_setting("performance/importance_lod/unload_multiplier", null)
	pass
