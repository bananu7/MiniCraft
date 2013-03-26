#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
uniform mat4 Projection, View;
out vec2 var_texCoord;
out vec3 var_lightIntensity;

void main () {
    var_texCoord = texCoord;

    vec4 LightPosition = vec4(1000, 1000, 1000, 1);
    vec3 LightIntensity = vec3(0.8, 0.8, 0.8);
    vec3 MaterialDiffuseReflectivity = vec3(1.0, 1.0, 1.0);

    mat3 NormalMatrix = inverse(transpose(mat3(View)));
    vec3 tnorm = normalize(NormalMatrix * normal);
    vec4 eye = View * vec4(position, 1.0);
    vec3 s = normalize(vec3(LightPosition - eye));
    // The diffuse shading equation
    var_lightIntensity = LightIntensity * MaterialDiffuseReflectivity * max( dot( s, tnorm ), 0.0 );
    gl_Position = Projection * View * vec4(position, 1.0);
}
