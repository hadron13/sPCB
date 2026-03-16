#version 330

layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aTexCoord;

uniform mat4 transform;
uniform mat4 projection;

out vec2 texCoord;

void main(){
    texCoord = aTexCoord;

    vec4 pos = projection * transform * vec4(aPos, -1.0, 1.0);
    gl_Position = pos;
}
