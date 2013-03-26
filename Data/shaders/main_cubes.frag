#version 330 core

out vec4 out_Color;
in vec2 var_texCoord;
in vec3 var_lightIntensity;

uniform sampler2D Texture;

void main () {
    //out_Color = vec4(0.0, 1.0, 0.0, 1.0);
    //out_Color = vec4((out_position.xyz + 1)/2, 1.0);
    //out_Color = vec4(out_texCoord, 0.0, 1.0);
    vec2 tc = var_texCoord;
    out_Color = clamp(vec4(var_lightIntensity, 1.0), 0.0, 1.0) * vec4(texture(Texture, tc).rgb, 1.0);
    //out_Color = vec4(var_lightIntensity, 1.0);
}
