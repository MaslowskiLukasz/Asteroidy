#version 330

out vec4 pixelColor; //Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela

in vec4 iC; //Zmienna interpolowana - kolor wyliczony w vertex shaderze

void main(void) {
	pixelColor=iC;
}
