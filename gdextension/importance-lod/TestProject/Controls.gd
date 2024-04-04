extends Node
var camera: Camera3D

func _ready() -> void:
	camera = get_viewport().get_camera_3d()
	# Just an example. Not necessary if auto detection works.
	ImportanceLODManager.set_camera(camera)

func _input(event:InputEvent) -> void:
	if event is InputEventKey and event.keycode == KEY_F8:
		print("Shutting down LODManager thread")
		ImportanceLODManager.stop_loop()
		get_tree().quit()

func _on_LodMultiplier_text_entered(new_text: String) -> void:
	ProjectSettings.set_setting("performance/importance_lod/global_multiplier", new_text.to_float())
	ImportanceLODManager.update_lod_multipliers_from_settings()
	$"GlobalLODText/LineEdit".release_focus()

func _on_fov_text_entered(new_text: String) -> void:
	var fov: float = clamp(new_text.to_float(), 35, 140)
	$"Camera3D FOV/LineEdit".text = str(fov)
	camera.fov = fov
	ImportanceLODManager.update_fov()
	ImportanceLODManager.update_lod_AABBs()
	ImportanceLODManager.set_camera(get_viewport().get_camera_3d())
	$"Camera3D FOV/LineEdit".release_focus()
