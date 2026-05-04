#version 330

layout(origin_upper_left) in vec4 gl_FragCoord;
in vec2 texCoord;

uniform vec2 quad_origin;
uniform vec2 quad_size;

void main(){
    vec2 uv = (texCoord - 0.5f) * quad_size - quad_origin;

    if(mod(uv.x, 10.0) < 0.2f || mod(uv.y, 10.0f) < 0.2f)
        gl_FragColor = vec4(0, 0, 1.0, 0.2f);
    else 
        gl_FragColor = vec4(0.0f);
}
