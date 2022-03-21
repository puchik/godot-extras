extends Node
var camera: Camera


func _ready() -> void:
	camera = get_viewport().get_camera()


func _on_LodMultiplier_text_entered(new_text: String) -> void:
	ProjectSettings.set_setting("rendering/quality/lod/global_multiplier", new_text.to_float())
	get_node("/root/LodManager").update_lod_multipliers_from_settings()
	$"GlobalLODText/LineEdit".release_focus()


func _on_fov_text_entered(new_text: String) -> void:
	var fov: float = clamp(new_text.to_float(), 45, 90)
	$"Camera FOV/LineEdit".text = str(fov)
	camera.fov = fov
	get_node("/root/LodManager").update_fov()
	get_node("/root/LodManager").update_lod_AABBs()
	get_node("/root/LodManager").set_camera(get_viewport().get_camera())
	$"Camera FOV/LineEdit".release_focus()
