/*
Niniejszy program jest wolnym oprogramowaniem; mo�esz go
rozprowadza� dalej i / lub modyfikowa� na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundacj� Wolnego
Oprogramowania - wed�ug wersji 2 tej Licencji lub(wed�ug twojego
wyboru) kt�rej� z p�niejszych wersji.

Niniejszy program rozpowszechniany jest z nadziej�, i� b�dzie on
u�yteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domy�lnej
gwarancji PRZYDATNO�CI HANDLOWEJ albo PRZYDATNO�CI DO OKRE�LONYCH
ZASTOSOWA�.W celu uzyskania bli�szych informacji si�gnij do
Powszechnej Licencji Publicznej GNU.

Z pewno�ci� wraz z niniejszym programem otrzyma�e� te� egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
je�li nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE // GLM_SWIZZLE_FULL //na pocz�tku pr.

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "allmodels.h"
#include "lodepng.h"
#include "shaderprogram.h"

using namespace glm;

float speed_x = 0; // [radiany/s]
float speed_y = 0; // [radiany/s]
float move_z = 0;

float aspect=1; //Stosunek szeroko�ci do wysoko�ci okna

//Uchwyty na shadery
ShaderProgram *shaderProgram; //Wska�nik na obiekt reprezentuj�cy program cieniuj�cy.


//Czajnik
float* vertices=Models::TeapotInternal::vertices;
float* colors=Models::TeapotInternal::colors;
float* normals=Models::TeapotInternal::vertexNormals;
int vertexCount=Models::TeapotInternal::vertexCount;


//Procedura obs�ugi b��d�w
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

//Procedura obs�ugi klawiatury
void key_callback(GLFWwindow* window, int key,
	int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) speed_y = -3.14;
		if (key == GLFW_KEY_RIGHT) speed_y = 3.14;
		if (key == GLFW_KEY_UP) speed_x = -3.14;
		if (key == GLFW_KEY_DOWN) speed_x = 3.14;
	}


	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_y = 0;
		if (key == GLFW_KEY_RIGHT) speed_y = 0;
		if (key == GLFW_KEY_UP) {
                move_z += 0.5;
                speed_x = 0;
		}
		if (key == GLFW_KEY_DOWN) speed_x = 0;
	}
}

//Procedura ob�ugi zmiany rozmiaru bufora ramki
void windowResize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height); //Obraz ma by� generowany w oknie o tej rozdzielczo�ci
    if (height!=0) {
        aspect=(float)width/(float)height; //Stosunek szeroko�ci do wysoko�ci okna
    } else {
        aspect=1;
    }
}


//Procedura inicjuj�ca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, kt�ry nale�y wykona� raz, na pocz�tku programu************
	glClearColor(0, 0, 0, 1); //Czy�� ekran na czarno
	glEnable(GL_DEPTH_TEST); //W��cz u�ywanie Z-Bufora
	glfwSetKeyCallback(window, key_callback); //Zarejestruj procedur� obs�ugi klawiatury
	glfwSetFramebufferSizeCallback(window,windowResize); //Zarejestruj procedur� obs�ugi zmiany rozmiaru bufora ramki

	shaderProgram=new ShaderProgram("vshader.vert",NULL,"fshader.frag"); //Wczytaj program cieniuj�cy

}

//Zwolnienie zasob�w zaj�tych przez program
void freeOpenGLProgram() {
	delete shaderProgram; //Usuni�cie programu cieniuj�cego

}

void drawObject(ShaderProgram *shaderProgram, mat4 mP, mat4 mV, mat4 mM) {
	//W��czenie programu cieniuj�cego, kt�ry ma zosta� u�yty do rysowania
	//W tym programie wystarczy�oby wywo�a� to raz, w setupShaders, ale chodzi o pokazanie,
	//�e mozna zmienia� program cieniuj�cy podczas rysowania jednej sceny
	shaderProgram->use();

	//Przeka� do shadera macierze P,V i M.
	//W linijkach poni�ej, polecenie:
	//  shaderProgram->getUniformLocation("P")
	//pobiera numer przypisany zmiennej jednorodnej o podanej nazwie
	//UWAGA! "P" w powy�szym poleceniu odpowiada deklaracji "uniform mat4 P;" w vertex shaderze,
	//a mP w glm::value_ptr(mP) odpowiada argumentowi  "mat4 mP;" TYM pliku.
	//Ca�a poni�sza linijka przekazuje do zmiennej jednorodnej P w vertex shaderze dane z argumentu mP niniejszej funkcji
	//Pozosta�e polecenia dzia�aj� podobnie.
	//Poni�sze polecenia s� z grubsza odpowiednikami glLoadMatrixf ze starego opengla
	glUniformMatrix4fv(shaderProgram->getUniformLocation("P"),1, false, glm::value_ptr(mP));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("V"),1, false, glm::value_ptr(mV));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"),1, false, glm::value_ptr(mM));

	//Powiedz OpenGL �e podczas rysowania nalezy przes�a� dane do atrybut�w o numerach wskazanych jako argument.
	//Polecenie shaderProgram->getAttribLocation("vertex") zwraca numer atrybutu o nazwie "vertex".
	//Odpowiada to deklaracji "in vec4 vertex;" w vertex shaderze.
	//Z grubsza odpowiednik polecenia glEnableClientState
    glEnableVertexAttribArray(shaderProgram->getAttribLocation("vertex"));
    glEnableVertexAttribArray(shaderProgram->getAttribLocation("color"));
    glEnableVertexAttribArray(shaderProgram->getAttribLocation("normal"));

    //Wska� tablic� z kt�rej nale�y pobra� dane do atrybutu o konkretnym numerze.
    //Pierwszym argumentem jest numer przypisany atrybutowi, ostatnim tablica zdanymi.
    //Z grubsza s� to odpowiedniki polece� glVertexPointer,glColorPointer i glNormalPointer ze starego OpenGL
    glVertexAttribPointer(shaderProgram->getAttribLocation("vertex"),4,GL_FLOAT,false,0,vertices);
    glVertexAttribPointer(shaderProgram->getAttribLocation("color"),4,GL_FLOAT,false,0,colors);
    glVertexAttribPointer(shaderProgram->getAttribLocation("normal"),4,GL_FLOAT,false,0,normals);

	//Narysowanie obiektu
	glDrawArrays(GL_TRIANGLES,0,vertexCount);

	//Posprz�tanie po sobie
    //Odpowiednik sekwencji polece� glDisableClientState
	glDisableVertexAttribArray(shaderProgram->getAttribLocation("vertex"));
    glDisableVertexAttribArray(shaderProgram->getAttribLocation("color"));
    glDisableVertexAttribArray(shaderProgram->getAttribLocation("normal"));
}

//Procedura rysuj�ca zawarto�� sceny
void drawScene(GLFWwindow* window, float angle_x, float angle_y, float position_z) {
	//************Tutaj umieszczaj kod rysuj�cy obraz******************l

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //Wykonaj czyszczenie bufora kolor�w i g��boko�ci

	glm::mat4 P = glm::perspective(50 * PI / 180, aspect, 1.0f, 50.0f); //Wylicz macierz rzutowania

	glm::mat4 V = glm::lookAt( //Wylicz macierz widoku
		glm::vec3(0.0f, 0.0f, -5.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));


	//Wylicz macierz modelu rysowanego obiektu
	glm::mat4 M = glm::mat4(1.0f);
    M = glm::translate(M, glm::vec3(0.0f, 0.0f, position_z));
	M = glm::rotate(M, angle_x, glm::vec3(1, 0, 0));
	M = glm::rotate(M, angle_y, glm::vec3(0, 1, 0));


	//Narysuj obiekt
	drawObject(shaderProgram,P,V,M);

	//Przerzu� tylny bufor na przedni
	glfwSwapBuffers(window);

}



int main(void)
{
	GLFWwindow* window; //Wska�nik na obiekt reprezentuj�cy okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedur� obs�ugi b��d�w

	if (!glfwInit()) { //Zainicjuj bibliotek� GLFW
		fprintf(stderr, "Nie mo�na zainicjowa� GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utw�rz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Je�eli okna nie uda�o si� utworzy�, to zamknij program
	{
		fprintf(stderr, "Nie mo�na utworzy� okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje si� aktywny i polecenia OpenGL b�d� dotyczy� w�a�nie jego.
	glfwSwapInterval(1); //Czekaj na 1 powr�t plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotek� GLEW
		fprintf(stderr, "Nie mo�na zainicjowa� GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjuj�ce

	float angle_x = 0; //K�t obrotu obiektu
	float angle_y = 0; //K�t obrotu obiektu
	float position_z = 0;

	glfwSetTime(0); //Wyzeruj licznik czasu

	//G��wna p�tla
	while (!glfwWindowShouldClose(window)) //Tak d�ugo jak okno nie powinno zosta� zamkni�te
	{
		angle_x += speed_x*glfwGetTime(); //Zwi�ksz k�t o pr�dko�� k�tow� razy czas jaki up�yn�� od poprzedniej klatki
		angle_y += speed_y*glfwGetTime(); //Zwi�ksz k�t o pr�dko�� k�tow� razy czas jaki up�yn�� od poprzedniej klatki
		position_z += move_z*glfwGetTime(); //Zwi�ksz pozycj� - oddalenie od kamery
		glfwSetTime(0); //Wyzeruj licznik czasu
		drawScene(window,angle_x,angle_y, position_z); //Wykonaj procedur� rysuj�c�
		glfwPollEvents(); //Wykonaj procedury callback w zalezno�ci od zdarze� jakie zasz�y.
	}

	freeOpenGLProgram();

	glfwDestroyWindow(window); //Usu� kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zaj�te przez GLFW
	exit(EXIT_SUCCESS);
}
