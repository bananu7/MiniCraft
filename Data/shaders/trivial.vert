#version 330 core
precision mediump float;
layout(location = 0) in vec3 in_position;
uniform mat4 Projection, View;
void main() {
    gl_Position = Projection * View * vec4(in_position, 1.0);
}
