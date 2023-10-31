#extension GL_EXT_multiview : enable

layout(binding = 0) uniform World
{
    mat4 matrix;
} world;

layout(binding = 1) uniform ViewProjection
{
    mat4 matrices[2];
} viewProjection;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inSpeed;
layout (location = 2) in float inMass;

layout (location = 0) out vec4 outColor;

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
};

void main () 
{
  //TODO: varies point size depending on the distance!!
  outColor = vec4(0.2) + vec4(inSpeed, 0); //trying some speed visualization
  outColor.r *= (inMass / 30.0);
  gl_Position = viewProjection.matrices[gl_ViewIndex] * world.matrix * vec4(inPos, 1.0);
  gl_PointSize = 4 * sqrt(inMass) / ((gl_Position.z)); // try mass / 10 mass is between 10 and 100 At the moment
}