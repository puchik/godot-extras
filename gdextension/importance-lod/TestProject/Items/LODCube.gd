extends Node3D

func _process(_delta):
	var importance_text = "%f" % ($LODObject.get_importance())
	$ImportanceLabel.text = importance_text
	$ImportanceLabel.look_at(get_viewport().get_camera_3d().position)
	# look_at in Godot is opposite from what you'd expect... marked as "intended behaviour", so we flip manually
	$ImportanceLabel.scale.x = -1
