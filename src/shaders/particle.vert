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
  outColor = vec4(1,0.8,0.2,1.0);
  outColor.r *= inMass / 20.0;
  outColor.g *= inMass / 50.0;
  outColor.b *= inMass / 20.0;
  outColor = clamp(outColor,vec4(0),vec4(1));
  outColor.g -= clamp(0.2*(inMass-70)/30,0,1);
  outColor.r -= clamp(0.4*(inMass-70)/30,0,1);
  gl_Position = viewProjection.matrices[gl_ViewIndex] * world.matrix * vec4(inPos, 1.0);
  //point size depending on the distance!!
  gl_PointSize = 4 * pow(inMass,1.0/3.0) / ((gl_Position.z)); // try mass / 10 mass is between 10 and 100 At the moment
}