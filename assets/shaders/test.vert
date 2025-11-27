#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inTexCoord;

out vec3 FragPos;   // World space position
out vec3 Normal;    // World space normal
out vec2 TexCoord;      // Regular texture coords
out vec2 CloudTexCoord; // Special coords for clouds

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool isEarth;       // if this is the earth
uniform bool showClouds;    // should we render clouds
uniform float cloudRotationAngle; // current cloud rotation

void main()
{
    gl_Position = projection * view * model * vec4(inPosition, 1.0); // vertex transform pipeline
    FragPos = vec3(model * vec4(inPosition, 1.0));  // pass world space position to fragment shader
    Normal = mat3(transpose(inverse(model))) * inNormal;    // transform normal to world space
    TexCoord = inTexCoord;  // pass through regular texture coords

    // handle case for earth's clouds
    if (isEarth && showClouds) {
        vec2 centeredUV = TexCoord - 0.5;   // center the UV coordinates
        float radius = length(centeredUV);  // convert to polar coordinates
        float angle = atan(centeredUV.y, centeredUV.x) + cloudRotationAngle;    
        CloudTexCoord = vec2(cos(angle), sin(angle)) * radius + 0.5;    // convert back to cartesian with rotation applied
    } else {    // for non earth objects, just use regular coords
        CloudTexCoord = TexCoord;
    }
}