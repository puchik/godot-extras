extends Node
var camera: Camera

# Declare member variables here. Examples:
# var a = 2
# var b = "text"


# Called when the node enters the scene tree for the first time.
func _ready():
	camera = get_viewport().get_camera()


# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass

func _on_LodMultiplier_text_changed(new_text):
	ProjectSettings.set_setting("rendering/quality/lod/global_multiplier", new_text as float)
	get_node("/root/LodManager").update_lod_multipliers_from_settings()


func _on_fov_text_changed(new_text):
	camera.fov = new_text as float
	get_node("/root/LodManager").update_fov()
	get_node("/root/LodManager").update_lod_AABBs()
	get_node("/root/LodManager").set_camera(get_viewport().get_camera())
