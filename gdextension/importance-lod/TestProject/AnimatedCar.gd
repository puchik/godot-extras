extends Node3D

""" This script shows how to capture and use signals coming from the LOD system:
	* Every lod change
	* When unloaded
	
	Lod 4 means all of the meshes are invisible
"""


@export var car_speed: float = 1.0

var counter: int = 0


func _ready() -> void:
	#$Model.connect("lod_changed", Callable(self, "_on_lod_changed"))
	#$Model.connect("freed", Callable(self, "_on_freed"))
	pass


func _process(delta: float) -> void:
	var differential: float = counter * delta * car_speed
	counter += 1
	var x: float = 5 * cos(differential)
	var z: float = 5 * sin(differential)
	position = (Vector3(x, 0.35, z))
	transform.basis = Basis(Vector3.UP, 9.5-differential)


func enable() -> void:
	set_process(true)
	$AnimationPlayer.play()


func disable() -> void:
	set_process(false)
	$AnimationPlayer.stop(false)


func _on_lod_changed(lod: int) -> void:
	print("AnimatedCar changing lod level: ", lod)
	if lod > 1:
		disable()
	else:
		enable()


func _on_freed() -> void:
	print("AnimatedCar has been freed. RIP.")
	# $Model has been marked to be freed automatically by the LOD system.
	# Here we also unload the lights, animation player and AnimatedCar
	queue_free()
