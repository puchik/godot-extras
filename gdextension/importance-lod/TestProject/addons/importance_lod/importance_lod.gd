@tool
extends EditorPlugin
const AUTOLOAD_NAME = "ImportanceLODManager"

func _enter_tree():
	# Initialization of the plugin goes here.
	add_autoload_singleton(AUTOLOAD_NAME, "res://addons/importance_lod/lod_manager.tscn")
	pass


func _exit_tree():
	# Clean-up of the plugin goes here.
	remove_autoload_singleton(AUTOLOAD_NAME)
	pass
