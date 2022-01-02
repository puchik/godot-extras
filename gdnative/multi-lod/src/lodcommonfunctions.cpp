#include "lod.h"

using namespace godot;

#define PI 3.14f

bool LODCommonFunctions::try_register(Node* node, bool state) {
    if (node->get_node("/root/LodManager")) {
        if (state) {
            node->get_node("/root/LodManager")->call("add_object", node);
            node->set("registered", true);
        } else {
            node->get_node("/root/LodManager")->call("remove_object", node);
            node->set("registered", false);
            node->set("interacted_with_manager", false);
        }
        return true;
    }
    return false;
}

// Returns the tan theta component when calculating the distances we need for screen percentage lod.
float LODCommonFunctions::lod_calculate_AABB_distance_tan_theta(float fov) {
    // Use an isosceles triangle to get a worst-case estimate of the distances.
    // Don't forget the degrees to radians conversion.
    return tan((fov * PI / 180.0f) / 2.0f);
}