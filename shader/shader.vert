#version 450

vec2 positions[3] = vec2[](
vec2(0.0, 0.5),
vec2(0.5, 0.5),
vec2(-0.5, 0.5)
);

void main(){
    // gl_VertexIndex : 定点数量，会不断 ++
    // gl_Position : 顶点位置，将会被送到着色器中
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}