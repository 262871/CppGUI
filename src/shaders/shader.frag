#version 450

layout(location = 0) in vec3 color;
layout(location = 1) in vec2 texCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler;

void main() {
    outColor = vec4(color * texture(texSampler, texCoord).rgb, 1.0);
}
