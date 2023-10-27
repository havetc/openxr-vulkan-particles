#extension GL_EXT_multiview : enable

layout (location = 0) in vec4 inColor;
layout (location = 1) in float inGradientPos;

layout (location = 0) out vec4 outFragColor;

void main () 
{
	const float radius = 0.25;
	if (length(gl_PointCoord - vec2(0.5)) > radius) {
		discard;
	}
	vec3 color = inColor.rgb;
	outFragColor.rgba = vec4(color, 1.0);
}
