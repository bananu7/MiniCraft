#version 330 core

out vec4 out_Color;
in vec2 var_texCoord;
in vec3 var_lightIntensity;
in float var_fogIntensity;

uniform sampler2D Texture;

void main () {
    //out_Color = vec4(0.0, 1.0, 0.0, 1.0);
    //out_Color = vec4((out_position.xyz + 1)/2, 1.0);
    //out_Color = vec4(out_texCoord, 0.0, 1.0);
    vec2 tc = var_texCoord;
    vec4 cubeTexture = vec4(texture(Texture, tc).rgb, 1.0);
    //out_Color = clamp(vec4(var_lightIntensity, 1.0), 0.0, 1.0) * vec4(texture(Texture, tc).rgb, 1.0);
    float fogFactor = var_fogIntensity * 0.0001f + var_fogIntensity*var_fogIntensity * 0.00001f;
    
    out_Color = mix(cubeTexture, vec4(63.f/255, 215.f/255, 252.f/255, 1.f), clamp(fogFactor, 0., 1.));
    //out_Color = vec4(var_lightIntensity, 1.0);
}
