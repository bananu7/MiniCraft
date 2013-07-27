#version 330 core

// main cubes vert

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
uniform mat4 Projection, View;
uniform vec3 Eye;

out vec2 var_texCoord;
out vec3 var_lightIntensity;
out float var_fogIntensity;

float CalcPointLightFactor(vec3 lightPosition, vec3 normal, vec3 position);
float CalcDirectionalLightFactor(vec3 lightDirection, vec3 normal);

void main () {
    var_texCoord = texCoord;

    struct DirectionalLight {
        vec3 Color;
        vec3 AmbientIntensity;
        vec3 DiffuseIntensity;
        vec3 Direction;
    } Light0;

    struct PointLight {
        vec3 Color;
        vec3 AmbientIntensity;
        vec3 DiffuseIntensity;
        vec3 Position;
    } Light1;
    
    Light0.Color = vec3(1.0, 1.0, 1.0);
    Light0.AmbientIntensity = vec3(0.1, 0.1, 0.1);
    Light0.DiffuseIntensity = vec3(0.8, 0.8, 0.8);
    Light0.Direction = vec3(-10, -100, -100);

    Light1.Color = vec3(1.0, 1.0, 1.0);
    Light1.AmbientIntensity = vec3(0.1, 0.1, 0.1);
    Light1.DiffuseIntensity = vec3(0.8, 0.8, 0.8);
    Light1.Position = vec3(20, 20, 20);

    vec3 MaterialDiffuseReflectivity = vec3(1.0, 1.0, 1.0);

    //mat3 NormalMatrix = inverse(transpose(mat3(View)));
    //vec3 tnorm = normalize(NormalMatrix * normal);
    //vec3 tnorm = normal;
    //vec3 tnorm = (View * vec4(normal, 0.0)).xyz;

    vec3 AmbientColor = Light0.AmbientIntensity * Light0.Color;
    //vec3 DiffuseColor = Light0.Color * Light0.DiffuseIntensity * CalcDirectionalLightFactor(Light0.Direction, normal);
    vec3 DiffuseColor = Light1.Color * Light1.DiffuseIntensity * CalcPointLightFactor(Light1.Position, normal, position);

    var_lightIntensity = DiffuseColor + AmbientColor;

    //fog
    var_fogIntensity = distance(position, Eye);

    //var_lightIntensity = tnorm;
    gl_Position = Projection * View * vec4(position, 1.0);
}
