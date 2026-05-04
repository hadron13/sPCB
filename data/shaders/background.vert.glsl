#version 330

layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aTexCoord;

// uniform mat4 projection; 
// uniform vec2 resolution; 
out vec2 texCoord;

void main(){
    // texCoord = (projection * vec4(resolution * (aTexCoord) * vec2(-1.0, 1.0), -1.0f, 1.0f)).xy;
    texCoord = aTexCoord;
    gl_Position = vec4(2.0f * aPos - 1.0f, -1.0, 1.0);
}
