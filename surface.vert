#version 100

uniform mat4 projection;
uniform mat4 transform;

attribute vec3 position;

varying vec3 p;

void main()
{
    gl_Position = projection * transform  * vec4(position, 1.0);
    p = position;
}
