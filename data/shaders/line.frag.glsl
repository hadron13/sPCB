#version 330

layout(origin_upper_left) in vec4 gl_FragCoord;
in vec2 texCoord;

uniform vec2 quad_origin;
uniform vec2 quad_size;
uniform float thickness;
uniform vec4 color;
uniform vec4 line;
uniform mat4 transform;


float sdSegment( in vec2 p, in vec2 a, in vec2 b ){
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}


void main(){
    vec2 uv = texCoord * (quad_size + thickness)-thickness/2.0;

    float d = sdSegment(uv, vec2(0), quad_size);
    d -= thickness/2.0f;
    
    gl_FragColor = d < 0.0? color : vec4(0.5);
}

