#version 300 es
layout (location = 0) in vec3 aPos;   // 顶点位置
layout (location = 1) in vec2 aTexCoord; // 纹理坐标

out vec2 TexCoord; // 输出到片段着色器的纹理坐标

void main() {
    gl_Position = vec4(aPos, 1.0); // 直接输出顶点位置
    TexCoord = aTexCoord; // 将纹理坐标传递到片段着色器
}
