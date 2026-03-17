#version 330


uniform vec4 color;
in vec2 texCoord;


float sdRoundedBox( in vec2 p, in vec2 b, in vec4 r ){
    r.xy = (p.x>0.0)?r.xy : r.zw;
    r.x  = (p.y>0.0)?r.x  : r.y;
    vec2 q = abs(p)-b+r.x;
    return min(max(q.x,q.y),0.0) + length(max(q,0.0)) - r.x;
}

void main(){
    float d = sdRoundedBox(texCoord-0.5, vec2(0.5, 0.5), vec4(0.1));
    gl_FragColor = d < 0.0? color : vec4(0);
}
