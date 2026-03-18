#version 330

layout(origin_upper_left) in vec4 gl_FragCoord;
in vec2 texCoord;

uniform vec2 quad_origin;
uniform vec2 quad_size; 
uniform float thickness;
uniform vec4 color;

float sdCircle( vec2 p, float r ){
    return length(p) - r;
}

void main(){
    float d = sdCircle(gl_FragCoord.xy - quad_origin - quad_size*0.5, quad_size.x*0.5) ;
    d = abs(d) - thickness/2.0f;
    gl_FragColor = d < 0.0? color : vec4(0.5);
}
