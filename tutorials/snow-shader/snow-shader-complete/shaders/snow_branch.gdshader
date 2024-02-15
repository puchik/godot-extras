shader_type spatial;
render_mode blend_mix,depth_draw_opaque,cull_disabled,diffuse_burley,specular_schlick_ggx;

uniform float snow_threshold : hint_range(0, 1.1) = 0.6;
uniform float snow_blend_range : hint_range(0, 0.9) = 0.1;

uniform float specular = 0.5;
uniform float metallic = 0.0;

uniform vec4 albedo : source_color;
uniform sampler2D texture_albedo : source_color;
uniform float alpha_cutout : hint_range(0, 1);
uniform float transmission : hint_range(0, 1) = 0.5;
uniform sampler2D texture_normal : hint_normal;
uniform float normal_scale : hint_range(-16,16) = 1.0;
uniform float roughness : hint_range(0,1) = 0.9;

uniform vec4 albedo_snow : source_color;
uniform sampler2D texture_albedo_snow : source_color;
uniform sampler2D texture_normal_snow : hint_normal;
uniform float normal_scale_snow : hint_range(-16,16) = 1.0;
uniform float roughness_snow : hint_range(0,1) = 0.75;

uniform vec3 uv1_scale;
uniform vec3 uv1_offset;
uniform vec3 uv2_scale;
uniform vec3 uv2_offset;

void vertex() {
	UV=UV*uv1_scale.xy+uv1_offset.xy;
}

void fragment() {
	vec2 base_uv = UV;
	
	// Shared
	METALLIC = metallic;
	SPECULAR = specular;
	
	// Mixed values
	vec4 albedo_tex = texture(texture_albedo,base_uv);
	vec3 albedo_total = albedo.rgb * albedo_tex.rgb;
	
	vec4 albedo_tex_snow = texture(texture_albedo_snow,base_uv);
	vec3 albedo_total_snow = albedo_snow.rgb * albedo_tex_snow.rgb;
	
	vec3 normal_tex = texture(texture_normal,base_uv).rgb;
	vec3 normal_tex_snow = texture(texture_normal_snow,base_uv).rgb;
	
	/// Calculation without normal map
	//vec3 world_normal = NORMAL * mat3(INV_CAMERA_MATRIX[0].xyz, INV_CAMERA_MATRIX[1].xyz, INV_CAMERA_MATRIX[2].xyz);
	//world_normal = (vec4(NORMAL, 0.0) * INV_CAMERA_MATRIX).xyz;
	
	/// Calculation including the normal map
	// Get TBN matrix
	vec3 T = (vec4(TANGENT, 0.0) * VIEW_MATRIX).xyz;
	vec3 B = (vec4(BINORMAL, 0.0) * VIEW_MATRIX).xyz;
	vec3 N = (vec4(NORMAL, 0.0) * VIEW_MATRIX).xyz;
	mat3 TBN = mat3(T, B, -N);
	
	// Calculate blend factor
	float snow_coeff = normalize(TBN * (normal_tex * 2.0 - vec3(1.0))).y;
	float blend_factor = smoothstep(snow_threshold - snow_blend_range, snow_threshold + snow_blend_range, snow_coeff);
	
	// Blend textures
	vec3 final_albedo = mix(albedo_total, albedo_total_snow, blend_factor);
	vec3 final_normal = mix(normal_tex, normal_tex_snow, blend_factor);
	float final_normal_scale = mix(normal_scale, normal_scale_snow, blend_factor);
	float final_roughness = mix(roughness, roughness_snow, blend_factor);
	
	// Apply final blended textures
	ALBEDO = final_albedo;
	NORMAL_MAP = final_normal;
	NORMAL_MAP_DEPTH = final_normal_scale;
	ROUGHNESS = final_roughness;
	ALPHA = albedo_tex.a;
	ALPHA_SCISSOR_THRESHOLD = alpha_cutout;
	BACKLIGHT = vec3(transmission);
}
