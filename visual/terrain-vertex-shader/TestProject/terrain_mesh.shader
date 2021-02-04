/*** TERRAIN/VERTEX COLOUR SHADER 1.0 ***/
/*** Arman Elgudzhyan (@puchik) ***/
/*** Licensed under MIT ***/

shader_type spatial;
render_mode blend_mix,depth_draw_opaque,cull_back,diffuse_burley,specular_schlick_ggx;

//// Texture 1
// Albedo
uniform vec4 albedo_1 : hint_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform sampler2D texture_albedo_1 : hint_albedo;
// Metallic
uniform float metallic_1 : hint_range(0,1);
//uniform sampler2D texture_metallic_1 : hint_white;
// Roughness
uniform float roughness_1 : hint_range(0,1.5) = 0.8;
uniform sampler2D texture_roughness_1 : hint_white;
// Normal
uniform sampler2D texture_normal_1 : hint_normal;
uniform float normal_scale_1 : hint_range(-16,16) = 1.0;
// Ambient occlusion
//uniform sampler2D texture_ao_1 : hint_white;
//uniform float ao_light_affect_1 : hint_range(0,1);
// Depth
uniform sampler2D texture_depth_1 : hint_black;
uniform float depth_scale_1 : hint_range(-16,16) = 0.05;
uniform vec2 depth_flip_1 = vec2(1.0, 1.0); // first = tangent, second = binormal
uniform bool use_depth_1;
uniform bool deep_parallax_1;
uniform int depth_min_layers_1: hint_range(0, 128) = 4;
uniform int depth_max_layers_1: hint_range(0, 128) = 16;
// Use this to use roughness, metalness, and ao at the same time on one texture
//uniform sampler2D texture_rgmtao_1 : hint_white;
uniform vec3 uv2_tex1_scale = vec3(1.0, 1.0, 1.0);
uniform vec3 uv2_tex1_offset;

//// Texture 2
// Albedo
uniform vec4 albedo_2 : hint_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform sampler2D texture_albedo_2 : hint_albedo;
// Metallic
uniform float metallic_2 : hint_range(0,1);
//uniform sampler2D texture_metallic_2 : hint_white;
// Roughness
uniform float roughness_2 : hint_range(0,1.5) = 0.8;
uniform sampler2D texture_roughness_2 : hint_white;
// Normal
uniform sampler2D texture_normal_2 : hint_normal;
uniform float normal_scale_2 : hint_range(-16,16) = 1.0;
// Ambient occlusion
//uniform sampler2D texture_ao_2 : hint_white;
//uniform float ao_light_affect_2 : hint_range(0,1);
// Depth
uniform sampler2D texture_depth_2 : hint_black;
uniform float depth_scale_2 : hint_range(-16,16) = 0.05;
uniform vec2 depth_flip_2 = vec2(1.0, 1.0); // first = tangent, second = binormal
uniform bool use_depth_2;
uniform bool deep_parallax_2;
uniform int depth_min_layers_2: hint_range(0, 128) = 4;
uniform int depth_max_layers_2: hint_range(0, 128) = 16;
// Use this to use roughness, metalness, and ao at the same time on one texture
//uniform sampler2D texture_rgmtao_2 : hint_white;
uniform vec3 uv2_tex2_scale = vec3(1.0, 1.0, 1.0);
uniform vec3 uv2_tex2_offset;

//// Texture 3
// Albedo
uniform vec4 albedo_3 : hint_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform sampler2D texture_albedo_3 : hint_albedo;
// Metallic
uniform float metallic_3 : hint_range(0,1);
//uniform sampler2D texture_metallic_3 : hint_white;
// Roughness
uniform float roughness_3 : hint_range(0,1.5) = 0.8;
uniform sampler2D texture_roughness_3 : hint_white;
// Normal
uniform sampler2D texture_normal_3 : hint_normal;
uniform float normal_scale_3 : hint_range(-16,16) = 1.0;
// Ambient occlusion
//uniform sampler2D texture_ao_3 : hint_white;
//uniform float ao_light_affect_3 : hint_range(0,1);
// Depth
uniform sampler2D texture_depth_3 : hint_black;
uniform float depth_scale_3 : hint_range(-16,16) = 0.05;
uniform vec2 depth_flip_3 = vec2(1.0, 1.0); // first = tangent, second = binormal
uniform bool use_depth_3;
uniform bool deep_parallax_3;
uniform int depth_min_layers_3: hint_range(0, 128) = 4;
uniform int depth_max_layers_3: hint_range(0, 128) = 16;
// Use this to use roughness, metalness, and ao at the same time on one texture
//uniform sampler2D texture_rgmtao_3 : hint_white;
uniform vec3 uv2_tex3_scale = vec3(1.0, 1.0, 1.0);
uniform vec3 uv2_tex3_offset;

//// Texture 4
// Albedo
uniform vec4 albedo_4 : hint_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform sampler2D texture_albedo_4 : hint_albedo;
// Metallic
uniform float metallic_4 : hint_range(0,1);
//uniform sampler2D texture_metallic_4 : hint_white;
// Roughness
uniform float roughness_4 : hint_range(0.0,1.5) = 0.8;
uniform sampler2D texture_roughness_4 : hint_white;
// Normal
uniform sampler2D texture_normal_4 : hint_normal;
uniform float normal_scale_4 : hint_range(-16,16) = 1.0;
// Ambient occlusion
//uniform sampler2D texture_ao_4 : hint_white;
//uniform float ao_light_affect_4 : hint_range(0,1);
// Depth
uniform sampler2D texture_depth_4 : hint_black;
uniform float depth_scale_4 : hint_range(-16,16) = 0.05;
uniform vec2 depth_flip_4 = vec2(1.0, 1.0); // first = tangent, second = binormal
uniform bool use_depth_4;
uniform bool deep_parallax_4;
uniform int depth_min_layers_4: hint_range(0, 128) = 4;
uniform int depth_max_layers_4: hint_range(0, 128) = 16;
// Use this to use roughness, metalness, and ao at the same time on one texture
//uniform sampler2D texture_rgmtao_4 : hint_white;
uniform vec3 uv2_tex4_scale = vec3(1.0, 1.0, 1.0);
uniform vec3 uv2_tex4_offset;

//// Texture channels (default: rough = red, metal = green, ao = blue)
// You will need this to use roughness, metalness, and ao at the same time in one texture
uniform vec4 roughness_texture_channel = vec4(1.0, 0.0, 0.0, 0.0);
//uniform vec4 metallic_texture_channel = vec4(0.0, 1.0, 0.0, 0.0);
//uniform vec4 ao_texture_channel = vec4(1.0, 0.0, 0.0, 1.0);

//// UV settings
uniform float uv2_blend_sharpness = 1.0;
varying vec2 uv2_tex1_pos;
varying vec2 uv2_tex2_pos;
varying vec2 uv2_tex3_pos;
varying vec2 uv2_tex4_pos;

uniform vec3 uv1_scale = vec3(1.0, 1.0, 1.0);
uniform vec3 uv1_offset;

//// Misc
uniform float specular = 0.5;
uniform float point_size : hint_range(0,128) = 1.0;
varying float dist;

//// Culling
uniform bool distance_culling = false;
// Range where this is visible
uniform float cull_min_dist = 100.0;
uniform float cull_max_dist = 300.0;
// Margin for fading out
uniform float cull_fade_margin = 20.0;

//// Anti-tiling
uniform bool prevent_tiling;
uniform sampler2D noise_tex : hint_white;

//// Blending
uniform bool height_blend = true; 
// Sharper values look better up close, faded values look better further away
uniform float height_blending_factor_min : hint_range(0.01, 1.0) = 0.05;
uniform float height_blending_factor_max : hint_range(0.01, 1.0) = 0.2;
uniform float height_blend_min_dist = 50.0;
uniform float height_blend_max_dist = 500.0;

uniform float height_offset_1 : hint_range(-10.0, 10.0) = 0.0;
uniform float height_multiplier_1 : hint_range(0.0, 10.0) = 1.0;
uniform float height_offset_2 : hint_range(-10.0, 10.0) = 0.0;
uniform float height_multiplier_2 : hint_range(0.0, 10.0) = 1.0;
uniform float height_offset_3 : hint_range(-10.0, 10.0) = 0.0;
uniform float height_multiplier_3 : hint_range(0.0, 10.0) = 1.0;
uniform float height_offset_4 : hint_range(-10.0, 10.0) = 0.0;
uniform float height_multiplier_4 : hint_range(0.0, 10.0) = 1.0;

//// Colour and terrain shape settings
uniform bool use_height_map = false;
uniform sampler2D height_map : hint_white;
uniform float height_map_scale = 1.0;
uniform bool use_splat_map = false;
uniform sampler2D splat_map : hint_white;

// TODO: Replace this with a texture lookup, or replace texture lookup with this?
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void vertex() {
	UV=UV*uv1_scale.xy+uv1_offset.xy;
	
	/// Displace the terrain first (based on UV1)
	if (use_height_map) {
		// Cut holes based on alpha
		vec4 height_tex = texture(height_map, UV);
		if (height_tex.a > 0.3) {
			VERTEX.y += height_tex.r * height_map_scale;
		} else {
			VERTEX = vec3(1.0 / 0.0);
		}
	}
	
	/// Used for height blending and culling
	float z_dist = clamp((MODELVIEW_MATRIX * vec4(VERTEX, 1.0)).z * -1.0,0.0, 2000.0 + 10.0);
	float x_dist = (MODELVIEW_MATRIX * vec4(VERTEX, 1.0)).x;
	dist = sqrt(z_dist * z_dist + x_dist * x_dist);

	if (distance_culling) {
		float num = 1000.0 / dist;
		
		// Object will not be visible between min and max dist
		float fade_ratio = 0.0;
		
		if (dist > cull_max_dist - cull_fade_margin) {
			fade_ratio = clamp((dist - (cull_max_dist - cull_fade_margin)) / cull_fade_margin, 0.0, 1.0);
		} else if (dist < cull_min_dist + cull_fade_margin) {
			fade_ratio = 1.0 - clamp((dist - cull_min_dist) / cull_fade_margin, 0.0, 1.0);
		}
		
		// Cull anything inside the cull range or behind us
		if (rand(vec2(VERTEX.x, VERTEX.y)) > fade_ratio || z_dist < 0.0) {
			VERTEX = vec3(1.0 / 0.0);
		}
	}
	
	TANGENT = vec3(0.0,0.0,-1.0) * abs(NORMAL.x);
	TANGENT+= vec3(1.0,0.0,0.0) * abs(NORMAL.y);
	TANGENT+= vec3(1.0,0.0,0.0) * abs(NORMAL.z);
	TANGENT = normalize(TANGENT);
	
	BINORMAL = vec3(0.0,-1.0,0.0) * abs(NORMAL.x);
	BINORMAL+= vec3(0.0,0.0,1.0) * abs(NORMAL.y);
	BINORMAL+= vec3(0.0,-1.0,0.0) * abs(NORMAL.z);
	BINORMAL = normalize(BINORMAL);
	
	// Calculate UVs
	uv2_tex1_pos = (VERTEX * uv2_tex1_scale + uv2_tex1_offset).xz;
	uv2_tex1_pos *= vec2(1.0, -1.0);
	uv2_tex2_pos = (VERTEX * uv2_tex2_scale + uv2_tex2_offset).xz;
	uv2_tex2_pos *= vec2(1.0, -1.0);
	uv2_tex3_pos = (VERTEX * uv2_tex3_scale + uv2_tex3_offset).xz;
	uv2_tex3_pos *= vec2(1.0, -1.0);
	uv2_tex4_pos = (VERTEX * uv2_tex4_scale + uv2_tex4_offset).xz;
	uv2_tex4_pos *= vec2(1.0, -1.0);
}

float sum_vec3(vec3 v) {
	return v.x + v.y + v.z;
}

vec3 texture_no_tile(sampler2D p_sampler, in vec2 x) {
	// See Technique 3 at:
	// www.iquilezles.org/www/articles/texturerepetition/texturerepetition.htm
    float k = texture(noise_tex, 0.005*x).x; // cheap (cache friendly) lookup
    
	float index = k * 8.0;
	float i = floor(index);
    //float l = k*8.0;
    float f = fract(index);
	
    //float ia = floor(l);
    //float ib = ia + 1.0;
	
	//vec2 offa = vec2(sin(ia), cos(ia)); // can replace with any other hash
	//vec2 offb = vec2(cos(ib), sin(ib)); // can replace with any other hash
	
	vec2 offa = sin(vec2(3.0,7.0)*(i+0.0)); // can replace with any other hash
	vec2 offb = sin(vec2(3.0,7.0)*(i+1.0)); // can replace with any other hash
	
	vec3 cola = texture(p_sampler, x + offa).xyz;
    vec3 colb = texture(p_sampler, x + offb).xyz;
    
    //return mix(cola, colb, smoothstep(0.2, 0.8, f - 0.1 * (cola-colb)));
	
	return mix(cola, colb, smoothstep(0.2, 0.8, f - 0.1 * sum_vec3(cola-colb)));
}

vec3 height_blend_vec3(vec3 input_1, float height_1, float color_1, vec3 input_2, float height_2, float color_2,
				  vec3 input_3, float height_3, float color_3, vec3 input_4, float height_4, float color_4) {
	// Based on http://untitledgam.es/2017/01/height-blending-shader/
	// Add 1.5 to all height maps, because the texture may have patches of full black and we don't
	// want those to just be empty spots on the mesh.
	// Also add the individual offsets.
	height_1 = (height_1 + 1.5 + height_offset_1) * color_1 * height_multiplier_1;
	height_2 = (height_2 + 1.5 + height_offset_2) * color_2 * height_multiplier_2;
	height_3 = (height_3 + 1.5 + height_offset_3) * color_3 * height_multiplier_3;
	height_4 = (height_4 + 1.5 + height_offset_4) * color_4 * height_multiplier_4;
	
	// Blend texture edges based on height
	// Calculate height blend based on distance
	float height_blending_factor = clamp(
		((dist - height_blend_min_dist) / (height_blend_max_dist - height_blend_min_dist))
		* (height_blending_factor_max - height_blending_factor_min) + height_blending_factor_min, 
		height_blending_factor_min, height_blending_factor_max);
	
	float height_start = max(max(height_1, height_2), max(height_3, height_4)) - height_blending_factor;
	
	float level_1 = max(height_1 - height_start, 0);
	float level_2 = max(height_2 - height_start, 0);
	float level_3 = max(height_3 - height_start, 0);
	float level_4 = max(height_4 - height_start, 0);
	return ((input_1 * level_1) + (input_2 * level_2) + 
			(input_3 * level_3) + (input_4 * level_4)) 
			/ (level_1 + level_2 + level_3 + level_4);// + (input_1 * color_1 + input_2 * color_2 + input_3 * color_3 + input_4 * color_4)) / 2.0;
}

float height_blend_float(float input_1, float height_1, float color_1, float input_2, float height_2, float color_2,
				float input_3, float height_3, float color_3, float input_4, float height_4, float color_4) {
	return height_blend_vec3(vec3(input_1), height_1, color_1, vec3(input_2), height_2, color_2,
				  vec3(input_3), height_3, color_3, vec3(input_4), height_4, color_4).r;
}

vec4 proj_texture(sampler2D p_sampler, vec2 pos) {
	vec4 samp = vec4(0.0);
	if (prevent_tiling) {
		samp.xyz += texture_no_tile(p_sampler, pos);
	} else {
		samp += texture(p_sampler, pos);
	}
	return samp;
}

vec2 calc_deep_parallax(vec3 view_dir, vec2 texture_UV, sampler2D texture_depth, 
						float depth_scale, int depth_max_layers, int depth_min_layers) {
	float num_layers = mix(float(depth_max_layers), float(depth_min_layers), abs(dot(vec3(0.0, 0.0, 1.0), view_dir)));
	float layer_depth = 1.0 / num_layers;
	float current_layer_depth = 0.0;
	
	vec2 P = view_dir.xy * depth_scale;
	vec2 delta = P / num_layers;
	
	vec2 ofs = texture_UV;
	
	float depth = proj_texture(texture_depth, ofs).r;
	float current_depth = 0.0;
	while (current_depth < depth) {
		ofs -= delta;
		depth = proj_texture(texture_depth, ofs).r;
		current_depth += layer_depth;
	}
	
	vec2 prev_ofs = ofs + delta;
	float after_depth  = depth - current_depth;
	float before_depth = proj_texture(texture_depth, ofs).r - current_depth + layer_depth;
	float weight = after_depth / (after_depth - before_depth);
	
	ofs = mix(ofs,prev_ofs,weight);
	return ofs;
}

vec2 calc_depth(vec3 view_dir, vec2 texture_UV, sampler2D texture_depth, float depth_scale) {
	float depth = proj_texture(texture_depth, texture_UV).r;
	vec2 ofs = view_dir.xy / view_dir.z * (depth * depth_scale);
	return texture_UV - ofs;
}

void fragment() {
	// Need to do this in fragment shader instead of vertex
	// otherwise using a splat map is pointless (vertex precision)
	float color_1;
	float color_2;
	float color_3;
	float color_4;
	if (use_splat_map) {
		vec4 splat_map_tex = texture(splat_map, UV);
		color_1 = splat_map_tex.r;
		color_2 = splat_map_tex.g;
		color_3 = splat_map_tex.b;
		color_4 = (1.0 - splat_map_tex.b - splat_map_tex.g - splat_map_tex.r);
	} else {
		color_1 = COLOR[0];
		color_2 = COLOR[1];
		color_3 = COLOR[2];
		color_4 = (1.0 - COLOR[2] - COLOR[1] - COLOR[0]);
	}
	
	// Calculate UV for each material based on depth texture
	vec2 texture_UV_1 = uv2_tex1_pos;
	{	
		vec3 view_dir = normalize(normalize(-VERTEX) * mat3(TANGENT * depth_flip_1.x, -BINORMAL * depth_flip_1.y, NORMAL));
		if (deep_parallax_1) {
			texture_UV_1 = calc_deep_parallax(view_dir, texture_UV_1, texture_depth_1, depth_scale_1, depth_min_layers_1, depth_max_layers_1); 
		} else if (use_depth_1) {
			texture_UV_1 = calc_depth(view_dir, texture_UV_1, texture_depth_1, depth_scale_1);
		}
	}
	vec2 texture_UV_2 = uv2_tex2_pos;
	{	
		vec3 view_dir = normalize(normalize(-VERTEX)*mat3(TANGENT*depth_flip_2.x,-BINORMAL*depth_flip_2.y,NORMAL));
		if (deep_parallax_2) {
			texture_UV_2 = calc_deep_parallax(view_dir, texture_UV_2, texture_depth_2, depth_scale_2, depth_min_layers_2, depth_max_layers_2); 
		} else if (use_depth_2) {
			texture_UV_2 = calc_depth(view_dir, texture_UV_2, texture_depth_2, depth_scale_2);
		}
	}
	vec2 texture_UV_3 = uv2_tex3_pos;
	{	
		vec3 view_dir = normalize(normalize(-VERTEX)*mat3(TANGENT*depth_flip_3.x,-BINORMAL*depth_flip_3.y,NORMAL));
		if (deep_parallax_3) {
			texture_UV_3 = calc_deep_parallax(view_dir, texture_UV_3, texture_depth_3, depth_scale_3, depth_min_layers_3, depth_max_layers_3); 
		} else if (use_depth_3) {
			texture_UV_3 = calc_depth(view_dir, texture_UV_3, texture_depth_3, depth_scale_3);
		}
	}
	vec2 texture_UV_4 = uv2_tex4_pos;
	{	
		vec3 view_dir = normalize(normalize(-VERTEX)*mat3(TANGENT*depth_flip_4.x,-BINORMAL*depth_flip_4.y,NORMAL));
		if (deep_parallax_4) {
			texture_UV_4 = calc_deep_parallax(view_dir, texture_UV_4, texture_depth_4, depth_scale_4, depth_min_layers_4, depth_max_layers_4); 
		} else if (use_depth_4) {
			texture_UV_4 = calc_depth(view_dir, texture_UV_4, texture_depth_4, depth_scale_4);
		}
	}
	
	// Get the height map values
	// Will potentially need these for height blend.
	float height_1_value = proj_texture(texture_depth_1, texture_UV_1).r;
	float height_2_value = proj_texture(texture_depth_2, texture_UV_2).r;
	float height_3_value = proj_texture(texture_depth_3, texture_UV_3).r;
	float height_4_value = proj_texture(texture_depth_4, texture_UV_4).r;
	
	// Specular strength
	SPECULAR = specular;
	
	// Apply albedo mixes ------------------------------------------------------------------------------------------------	
	vec3 albedo_mix_1 = proj_texture(texture_albedo_1, texture_UV_1).rgb * albedo_1.rgb;
	vec3 albedo_mix_2 = proj_texture(texture_albedo_2, texture_UV_2).rgb * albedo_2.rgb;
	vec3 albedo_mix_3 = proj_texture(texture_albedo_3, texture_UV_3).rgb * albedo_3.rgb;
	vec3 albedo_mix_4 = proj_texture(texture_albedo_4, texture_UV_4).rgb * albedo_4.rgb;
	
	if (height_blend) {
		ALBEDO = height_blend_vec3(albedo_mix_1, height_1_value, color_1,
				albedo_mix_2, height_2_value, color_2,
				albedo_mix_3, height_3_value, color_3,
				albedo_mix_4, height_4_value, color_4);
	} else {
		ALBEDO = albedo_mix_1 * color_1 + albedo_mix_2 * color_2 + albedo_mix_3 * color_3 + albedo_mix_4 * color_4;
	}
	
	// Apply metallic mixes ------------------------------------------------------------------------------------------------
	// There are too many textures if you add metallic and ao, but if you remove the roughness mix, and instead use a single roughness, metallic, ao map
	// you can enable this and have all of them
	//float metal_mix_1 = dot(proj_texture(texture_rgmtao_1, texture_UV_1), metallic_texture_channel) * metallic_1;
	//float metal_mix_2 = dot(proj_texture(texture_rgmtao_2, texture_UV_2), metallic_texture_channel) * metallic_2;
	//float metal_mix_3 = dot(proj_texture(texture_rgmtao_3, texture_UV_3), metallic_texture_channel) * metallic_3;
	//float metal_mix_4 = dot(proj_texture(texture_rgmtao_4, texture_UV_4), metallic_texture_channel) * metallic_4;
	//if (height_blend) {
		//METALLIC = height_blend_float(metal_mix_1, height_1_value,
				//metal_mix_2, height_2_value, 
				//metal_mix_3, height_3_value,
				//metal_mix_4, height_4_value);
	//} else {	
		//METALLIC = metal_mix_1 * color_1 + metal_mix_2 * color_2 + metal_mix_3 * color_3 + metal_mix_4 * color_4;
	//}
	if (height_blend) {
		METALLIC = height_blend_float(metallic_1, height_1_value, color_1,
				metallic_2, height_2_value, color_2,
				metallic_3, height_3_value, color_3,
				metallic_4, height_4_value, color_4);
	} else {	
		METALLIC = metallic_1 * color_1 + metallic_2 * color_2 + metallic_3 * color_3 + metallic_4 * color_4;
	}
	
	// Apply roughenss mixes ------------------------------------------------------------------------------------------------	
	// Change texture_roughness to texture_rgmtao to use roughness, metal, and ao at the same time (with correct texture colour scheme)
	float roughness_mix_1 = dot(proj_texture(texture_roughness_1, texture_UV_1), roughness_texture_channel) * roughness_1;
	float roughness_mix_2 = dot(proj_texture(texture_roughness_2, texture_UV_2), roughness_texture_channel) * roughness_2;
	float roughness_mix_3 = dot(proj_texture(texture_roughness_3, texture_UV_3), roughness_texture_channel) * roughness_3;
	float roughness_mix_4 = dot(proj_texture(texture_roughness_4, texture_UV_4), roughness_texture_channel) * roughness_4;
	if (height_blend) {
		ROUGHNESS = height_blend_float(roughness_mix_1, height_1_value, color_1,
				roughness_mix_2, height_2_value, color_2,
				roughness_mix_3, height_3_value, color_3,
				roughness_mix_4, height_4_value, color_4);
	} else {	
		ROUGHNESS = roughness_mix_1 * color_1 + roughness_mix_2 * color_2 + roughness_mix_3 * color_3 + roughness_mix_4 * color_4;
	}
	
	// Apply normal mixes ------------------------------------------------------------------------------------------------	
	vec3 normal_mix_1 = proj_texture(texture_normal_1, texture_UV_1).rgb;
	vec3 normal_mix_2 = proj_texture(texture_normal_2, texture_UV_2).rgb;
	vec3 normal_mix_3 = proj_texture(texture_normal_3, texture_UV_3).rgb;
	vec3 normal_mix_4 = proj_texture(texture_normal_4, texture_UV_4).rgb;
	if (height_blend) {
		NORMALMAP = height_blend_vec3(normal_mix_1, height_1_value, color_1,
				normal_mix_2, height_2_value, color_2,
				normal_mix_3, height_3_value, color_3,
				normal_mix_4, height_4_value, color_4);
		NORMALMAP_DEPTH = height_blend_float(normal_scale_1, height_1_value, color_1,
				normal_scale_2, height_2_value, color_2,
				normal_scale_3, height_3_value, color_3,
				normal_scale_4, height_4_value, color_4);
	} else {
		NORMALMAP = normal_mix_1 * color_1 + normal_mix_2 * color_2 + normal_mix_3 * color_3 + normal_mix_4 * color_4;
		NORMALMAP_DEPTH = normal_scale_1 * color_1 + normal_scale_2 * color_2 + normal_scale_3 * color_3 + normal_scale_4 * color_4;
	}

	// Apply ambient occlusion mixes ------------------------------------------------------------------------------------------------	
	// See comment under "metallic mixes"
	//float ao_mix_1 = dot(proj_texture(texture_rgmtao_1, texture_UV_1), ao_texture_channel);
	//float ao_mix_2 = dot(proj_texture(texture_rgmtao_2, texture_UV_2), ao_texture_channel);
	//float ao_mix_3 = dot(proj_texture(texture_rgmtao_3, texture_UV_3), ao_texture_channel);
	//float ao_mix_4 = dot(proj_texture(texture_rgmtao_4, texture_UV_4), ao_texture_channel);
		//if (height_blend) {
		//AO = height_blend_vec3(ao_mix_1, height_1_value,
				//ao_mix_2, height_2_value,
				//ao_mix_3, height_3_value,
				//ao_mix_4, height_4_value);
		//AO_LIGHT_AFFECT = height_blend_float(ao_light_affect_1, height_1_value,
				//ao_light_affect_2, height_2_value,
				//ao_light_affect_3, height_3_value,
				//ao_light_affect_4, height_4_value);
	//} else {
		//AO = ao_mix_1 * color_1 + ao_mix_2 * color_2 + ao_mix_3 * color_3 + ao_mix_4 * color_4;
		//AO_LIGHT_AFFECT = ao_light_affect_1 * color_1 + ao_light_affect_2 * color_2 + ao_light_affect_3 * color_3 + ao_light_affect_4 * color_4;
	//}
	
}
