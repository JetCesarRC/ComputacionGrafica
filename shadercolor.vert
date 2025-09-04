#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 inColor;

out vec3 vColor;

uniform mat4 model;
uniform mat4 projection;

void main(){
    vColor = inColor;
    gl_Position = projection * model * vec4(pos, 1.0);
}
