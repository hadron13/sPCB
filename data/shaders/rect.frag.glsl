#version 330

layout(origin_upper_left) in vec4 gl_FragCoord;
in vec2 texCoord;

uniform vec2 quad_origin;
uniform vec2 quad_size;
uniform float thickness;
uniform vec4 color;


float sdRoundedBox( in vec2 p, in vec2 b, in vec4 r ){
    r.xy = (p.x>0.0)?r.xy : r.zw;
    r.x  = (p.y>0.0)?r.x  : r.y;
    vec2 q = abs(p)-b+r.x;
    return min(max(q.x,q.y),0.0) + length(max(q,0.0)) - r.x;
}

void main(){
    vec2 uv = texCoord * (quad_size + thickness)-thickness/2.0;
    
    float d = sdRoundedBox(uv - quad_size * 0.5, quad_size*0.5, vec4(thickness/4.0f)) ;
    d = abs(d) - thickness/2.0f;
    
    gl_FragColor = d < 0.0? color : vec4(0.0);
}
