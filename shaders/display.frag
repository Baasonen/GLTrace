#version 460 core

out vec4 FragColor;
in vec2 v_uv;

uniform sampler2D u_texture;

void main()
{
    vec3 color = texture(u_texture, v_uv).rgb;

    FragColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0f);
}