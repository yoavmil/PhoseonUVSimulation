#version 100

uniform mat4 projection;
uniform mat4 transform;

attribute vec3 position;

void main(void)
{
    gl_Position = projection * transform  * vec4(position, 1.0);
}
