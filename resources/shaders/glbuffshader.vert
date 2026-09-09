layout (location = 0) in vec3 vaPos;
layout (location = 1) in vec2 vaTexUV;
layout (location = 2) in int vaOffset;
layout (location = 3) in int palid;

out vec2 texuv;
flat out int offset;
flat out int paletteid;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * vec4(vaPos, 1.0);
    texuv = vaTexUV;
    offset = vaOffset;
    paletteid = palid;
}
