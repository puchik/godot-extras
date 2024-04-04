extends Node3D

""" This script shows how to capture and use signals coming from the LOD system:
	* Every lod change
	* When unloaded
	
	Lod 4 means all of the meshes are invisible
"""

@export var car_speed: float = 1.0

var counter: int = 0

func _ready() -> void:
	$Model.connect("lod_changed", Callable(self, "_on_lod_changed"))
	$Model.connect("freed", Callable(self, "_on_freed"))
	pass

func _process(delta: float) -> void:
	var differential: float = counter * delta * car_speed
	counter += 1
	var x: float = 5 * cos(differential)
	var z: float = 5 * sin(differential)
	position = (Vector3(x, 0.35, z))
	transform.basis = Basis(Vector3.UP, 9.5-differential)
	
	var importance_text = "%f" % ($Model.get_importance())
	$Model/ImportanceLabel.text = importance_text
	$Model/ImportanceLabel.look_at(get_viewport().get_camera_3d().position)
	# look_at in Godot is opposite from what you'd expect... marked as "intended behaviour", so we flip manually
	$Model/ImportanceLabel.scale.x = -1

func enable_animation() -> void:
	$AnimationPlayer.play()

func disable_animation() -> void:
	$AnimationPlayer.stop(false)

func enable_movement() -> void:
	set_process(true)

func disable_movement() -> void:
	set_process(false)

func _on_lod_changed(lod: int) -> void:
	print("AnimatedCar changing lod level: ", lod)
	if lod > 2:
		disable_movement()
		disable_animation()
	elif lod > 1:
		disable_animation()
		enable_movement()
	elif lod >= 0:
		enable_animation()
		enable_movement()

func _on_freed() -> void:
	print("AnimatedCar has been freed. RIP.")
	# $Model has been marked to be freed automatically by the LOD system.
	# Here we also unload the lights, animation player and AnimatedCar
	queue_free()
