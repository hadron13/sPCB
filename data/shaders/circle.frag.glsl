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
    vec2 uv = texCoord * (quad_size + thickness)-thickness/2.0;

    float d = sdCircle(uv - quad_size*0.5, quad_size.x*0.5) ;
    d = abs(d) - thickness/2.0f;
    
    vec4 final_color = color;
    final_color.a = smoothstep(0.03, -0.03, d);

    gl_FragColor = d < 0.0? final_color : vec4(0.0);
}
