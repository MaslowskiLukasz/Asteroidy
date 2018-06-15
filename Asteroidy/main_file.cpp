/*
Niniejszy program jest wolnym oprogramowaniem; mo¿esz go
rozprowadzaæ dalej i / lub modyfikowaæ na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundacjê Wolnego
Oprogramowania - wed³ug wersji 2 tej Licencji lub(wed³ug twojego
wyboru) którejœ z póŸniejszych wersji.

Niniejszy program rozpowszechniany jest z nadziej¹, i¿ bêdzie on
u¿yteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyœlnej
gwarancji PRZYDATNOŒCI HANDLOWEJ albo PRZYDATNOŒCI DO OKREŒLONYCH
ZASTOSOWAÑ.W celu uzyskania bli¿szych informacji siêgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnoœci¹ wraz z niniejszym programem otrzyma³eœ te¿ egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeœli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE // GLM_SWIZZLE_FULL //na pocz¹tku pr.

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

float aspect=1; //Stosunek szerokoœci do wysokoœci okna

//Uchwyty na shadery
ShaderProgram *shaderProgram; //WskaŸnik na obiekt reprezentuj¹cy program cieniuj¹cy.


//Czajnik
float* vertices=Models::TeapotInternal::vertices;
float* colors=Models::TeapotInternal::colors;
float* normals=Models::TeapotInternal::vertexNormals;
int vertexCount=Models::TeapotInternal::vertexCount;


//Procedura obs³ugi b³êdów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

//Procedura obs³ugi klawiatury
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

//Procedura ob³ugi zmiany rozmiaru bufora ramki
void windowResize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height); //Obraz ma byæ generowany w oknie o tej rozdzielczoœci
    if (height!=0) {
        aspect=(float)width/(float)height; //Stosunek szerokoœci do wysokoœci okna
    } else {
        aspect=1;
    }
}


//Procedura inicjuj¹ca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który nale¿y wykonaæ raz, na pocz¹tku programu************
	glClearColor(0, 0, 0, 1); //Czyœæ ekran na czarno
	glEnable(GL_DEPTH_TEST); //W³¹cz u¿ywanie Z-Bufora
	glfwSetKeyCallback(window, key_callback); //Zarejestruj procedurê obs³ugi klawiatury
	glfwSetFramebufferSizeCallback(window,windowResize); //Zarejestruj procedurê obs³ugi zmiany rozmiaru bufora ramki

	shaderProgram=new ShaderProgram("vshader.vert",NULL,"fshader.frag"); //Wczytaj program cieniuj¹cy

}

//Zwolnienie zasobów zajêtych przez program
void freeOpenGLProgram() {
	delete shaderProgram; //Usuniêcie programu cieniuj¹cego

}

void drawObject(ShaderProgram *shaderProgram, mat4 mP, mat4 mV, mat4 mM) {
	//W³¹czenie programu cieniuj¹cego, który ma zostaæ u¿yty do rysowania
	//W tym programie wystarczy³oby wywo³aæ to raz, w setupShaders, ale chodzi o pokazanie,
	//¿e mozna zmieniaæ program cieniuj¹cy podczas rysowania jednej sceny
	shaderProgram->use();

	//Przeka¿ do shadera macierze P,V i M.
	//W linijkach poni¿ej, polecenie:
	//  shaderProgram->getUniformLocation("P")
	//pobiera numer przypisany zmiennej jednorodnej o podanej nazwie
	//UWAGA! "P" w powy¿szym poleceniu odpowiada deklaracji "uniform mat4 P;" w vertex shaderze,
	//a mP w glm::value_ptr(mP) odpowiada argumentowi  "mat4 mP;" TYM pliku.
	//Ca³a poni¿sza linijka przekazuje do zmiennej jednorodnej P w vertex shaderze dane z argumentu mP niniejszej funkcji
	//Pozosta³e polecenia dzia³aj¹ podobnie.
	//Poni¿sze polecenia s¹ z grubsza odpowiednikami glLoadMatrixf ze starego opengla
	glUniformMatrix4fv(shaderProgram->getUniformLocation("P"),1, false, glm::value_ptr(mP));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("V"),1, false, glm::value_ptr(mV));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"),1, false, glm::value_ptr(mM));

	//Powiedz OpenGL ¿e podczas rysowania nalezy przes³aæ dane do atrybutów o numerach wskazanych jako argument.
	//Polecenie shaderProgram->getAttribLocation("vertex") zwraca numer atrybutu o nazwie "vertex".
	//Odpowiada to deklaracji "in vec4 vertex;" w vertex shaderze.
	//Z grubsza odpowiednik polecenia glEnableClientState
    glEnableVertexAttribArray(shaderProgram->getAttribLocation("vertex"));
    glEnableVertexAttribArray(shaderProgram->getAttribLocation("color"));
    glEnableVertexAttribArray(shaderProgram->getAttribLocation("normal"));

    //Wska¿ tablicê z której nale¿y pobraæ dane do atrybutu o konkretnym numerze.
    //Pierwszym argumentem jest numer przypisany atrybutowi, ostatnim tablica zdanymi.
    //Z grubsza s¹ to odpowiedniki poleceñ glVertexPointer,glColorPointer i glNormalPointer ze starego OpenGL
    glVertexAttribPointer(shaderProgram->getAttribLocation("vertex"),4,GL_FLOAT,false,0,vertices);
    glVertexAttribPointer(shaderProgram->getAttribLocation("color"),4,GL_FLOAT,false,0,colors);
    glVertexAttribPointer(shaderProgram->getAttribLocation("normal"),4,GL_FLOAT,false,0,normals);

	//Narysowanie obiektu
	glDrawArrays(GL_TRIANGLES,0,vertexCount);

	//Posprz¹tanie po sobie
    //Odpowiednik sekwencji poleceñ glDisableClientState
	glDisableVertexAttribArray(shaderProgram->getAttribLocation("vertex"));
    glDisableVertexAttribArray(shaderProgram->getAttribLocation("color"));
    glDisableVertexAttribArray(shaderProgram->getAttribLocation("normal"));
}

//Procedura rysuj¹ca zawartoœæ sceny
void drawScene(GLFWwindow* window, float angle_x, float angle_y, float position_z) {
	//************Tutaj umieszczaj kod rysuj¹cy obraz******************l

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //Wykonaj czyszczenie bufora kolorów i g³êbokoœci

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

	//Przerzuæ tylny bufor na przedni
	glfwSwapBuffers(window);

}



int main(void)
{
	GLFWwindow* window; //WskaŸnik na obiekt reprezentuj¹cy okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurê obs³ugi b³êdów

	if (!glfwInit()) { //Zainicjuj bibliotekê GLFW
		fprintf(stderr, "Nie mo¿na zainicjowaæ GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Je¿eli okna nie uda³o siê utworzyæ, to zamknij program
	{
		fprintf(stderr, "Nie mo¿na utworzyæ okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje siê aktywny i polecenia OpenGL bêd¹ dotyczyæ w³aœnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekê GLEW
		fprintf(stderr, "Nie mo¿na zainicjowaæ GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjuj¹ce

	float angle_x = 0; //K¹t obrotu obiektu
	float angle_y = 0; //K¹t obrotu obiektu
	float position_z = 0;

	glfwSetTime(0); //Wyzeruj licznik czasu

	//G³ówna pêtla
	while (!glfwWindowShouldClose(window)) //Tak d³ugo jak okno nie powinno zostaæ zamkniête
	{
		angle_x += speed_x*glfwGetTime(); //Zwiêksz k¹t o prêdkoœæ k¹tow¹ razy czas jaki up³yn¹³ od poprzedniej klatki
		angle_y += speed_y*glfwGetTime(); //Zwiêksz k¹t o prêdkoœæ k¹tow¹ razy czas jaki up³yn¹³ od poprzedniej klatki
		position_z += move_z*glfwGetTime(); //Zwiêksz pozycjê - oddalenie od kamery
		glfwSetTime(0); //Wyzeruj licznik czasu
		drawScene(window,angle_x,angle_y, position_z); //Wykonaj procedurê rysuj¹c¹
		glfwPollEvents(); //Wykonaj procedury callback w zaleznoœci od zdarzeñ jakie zasz³y.
	}

	freeOpenGLProgram();

	glfwDestroyWindow(window); //Usuñ kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajête przez GLFW
	exit(EXIT_SUCCESS);
}
