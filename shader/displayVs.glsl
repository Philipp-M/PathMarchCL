#version 330

uniform vec2 size;
uniform vec2 position;

in vec2 pos;
out vec2 texCoord;

void main(void) {
	texCoord = vec2(pos.x, -pos.y) * 0.5f + 0.5f;
	gl_Position = vec4(position + size * pos, 0.0, 1.0);
}
