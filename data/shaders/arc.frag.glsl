#version 330

layout(origin_upper_left) in vec4 gl_FragCoord;
in vec2 texCoord;

uniform vec2 quad_origin;
uniform vec2 quad_size; 
uniform float thickness;
uniform vec4 color;
uniform vec2 start;
uniform vec2 end;



float DFarc(vec2 p, vec2 c, vec2 p1, vec2 p2){
    vec2 v1 = p1 - c;
    vec2 v2 = p2 - c;
    vec2 v = p - c;

    vec2 w = vec2( dot(v, vec2( v1.y,-v1.x)), 
                   dot(v, vec2(-v2.y, v2.x)));
    bool longarc = dot(v1, vec2(-v2.y, v2.x)) < 0.0;

    float z = longarc ? max(w.x,w.y) : min(w.x,w.y);
    
    return z>0.0 ? min(length(p1-p), length(p2-p)) : abs(length(v) - length(v1));
}


void main(){
    vec2 uv = texCoord * (quad_size + thickness)-thickness/2.0;
    vec2 p = uv - quad_size*0.5;
 

    float d = DFarc(p, vec2(0, 0), start, end) - thickness/2.0f;

    vec4 smooth_color = color;
    smooth_color.a = smoothstep(0.03, -0.03, d);

    gl_FragColor = d < 0.0? smooth_color : vec4(0.0);
}
