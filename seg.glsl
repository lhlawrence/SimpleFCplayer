#version 300 es
precision mediump float; // 设置浮点数精度

in vec2 TexCoord; // 从顶点着色器接收的纹理坐标

out vec4 FragColor; // 输出到屏幕的颜色

uniform sampler2D ourTexture; // 纹理采样器

void main() {
    FragColor = texture(ourTexture, TexCoord); // 使用纹理坐标采样纹理
}
