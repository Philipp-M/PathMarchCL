#version 330

uniform sampler2D srcTex;
in vec2 texCoord;
out vec4 color;

// Reinhard Tone Mapping
vec3 tonemapReinhard(vec3 hdrColor) {
	return hdrColor / (hdrColor + vec3(1.0));
}
vec3 tonemapExposure(vec3 hdrColor, float exposure) {
	return vec3(1.0) - exp(-hdrColor * exposure);
}

void main() {
	vec4 fragColor = texture(srcTex, texCoord);
	// linear tonemapping with gamma correction
	// color = clamp(vec4(pow(fragColor.xyz, vec3(1.0f / 2.2f)), fragColor.a), 0.0f, 1.0f);

	// use reinhard tonemapping with gamma correction
	color = clamp(vec4(pow(tonemapReinhard(fragColor.xyz), vec3(1.0f / 2.2f)), fragColor.a), 0.0f, 1.0f);
}
