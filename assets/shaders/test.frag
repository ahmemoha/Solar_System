#version 330 core

struct Material {
    sampler2D diffuse;   // daytime texture
    sampler2D specular;  // specular map
    sampler2D night;     // night lights texture
    sampler2D clouds;    // cloud texture
    float shininess;     // how shiny the surface is
}; 

// Light properties
struct Light {
    vec3 position; // sun position
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Inputs from vertex shader
in vec3 FragPos;      // world space position
in vec3 Normal;       // world space normal
in vec2 TexCoord;     // regular texture coords
in vec2 CloudTexCoord; // cloud texture coords

out vec4 fragColor; // output color

uniform Material material;  // surface material
uniform Light light;       // light source
uniform vec3 viewPos;      // camera position
uniform bool isSun;        // is this the sun?
uniform bool isEarth;      // is this the earth?
uniform bool showNightTexture; // do we need night light
uniform bool showClouds;   // show clouds

void main()
{
    if (isSun) { // for the sun, just render its texture
        fragColor = texture(material.diffuse, TexCoord);
        return; // Skip all other calculations
    }

    vec3 dayColor = texture(material.diffuse, TexCoord).rgb;   // Get base colors
    vec3 nightColor = isEarth ? texture(material.night, TexCoord).rgb * 0.8 : dayColor * 0.9; // Night color is dimmer: 80% for earth, 90% for moon
    
    // Lighting calculations
    vec3 norm = normalize(Normal); // normalized normal vector
    vec3 lightDir = normalize(light.position - FragPos); // light direction
    float diff = max(dot(norm, lightDir), 0.0); // diffuse lighting
    
    // dark side visibility
    vec3 ambient = light.ambient * dayColor * 0.6;
    vec3 diffuse = light.diffuse * diff * dayColor;
    
    // Specular highlights
    vec3 specular = vec3(0.0);
    if (diff > 0.0) { // only calculate if light is hitting surface
        vec3 viewDir = normalize(viewPos - FragPos); // view direction
        vec3 reflectDir = reflect(-lightDir, norm);  // reflection direction
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        specular = light.specular * spec * texture(material.specular, TexCoord).rgb;
    }

    // Combine lighting components
    vec3 result = ambient + diffuse + specular;

    // Enhanced night effect when enabled
    if (showNightTexture) {
        float nightBlend = smoothstep(0.0, 0.6, -dot(norm, lightDir)); // Wider transition
        vec3 nightEffect = isEarth ? nightColor : dayColor * 0.5; // Moon gets darker when night enabled
        result = mix(result, nightEffect, nightBlend * 0.8); // More subtle blending
    }
    else {
        float minLight = 0.1; // a little visibility when night textures are off
        result = max(result, dayColor * minLight);
    }

    // Cloud blending, only earth
    if (isEarth && showClouds) {
    // Sample cloud texture with wrapping
    vec2 wrappedCoords = fract(CloudTexCoord); // Ensure coordinates wrap around
    vec4 cloudColor = texture(material.clouds, wrappedCoords);
    
    // Better blending with edge feathering
    float edge = smoothstep(0.0, 0.1, min(CloudTexCoord.x, 1.0 - CloudTexCoord.x)) * 
                smoothstep(0.0, 0.1, min(CloudTexCoord.y, 1.0 - CloudTexCoord.y));
    result = mix(result, cloudColor.rgb, cloudColor.a * 0.3 * edge);
    }

    fragColor = vec4(result, 1.0); // Final output color
}