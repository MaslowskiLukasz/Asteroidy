#version 330

//Zmienne jednorodne
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;


//Atrybuty
in vec4 vertex; //wspolrzedne wierzcholka w przestrzeni modelu
in vec4 color;  //kolor wierzcholka (na razie ignorowany)
in vec4 normal; //wektor normalny w wierzcholku w przestrzeni modelu

//Zmienne interpolowane
out vec4 iC; //Kolor wyliczony w vertex shaderze

void main(void) {

    vec4 lp=vec4(0,0,-6,1); //Wspolrzedne swiatla w przestrzeni swiata

    vec4 ka=vec4(0,0,0,0); //Kolor obiektu w swietle otoczenia
    vec4 kd=color; //Kolor obiektu w swietle rozproszonym

    vec4 la=vec4(0,0,0,0); //Kolor swiatla otoczenia
    vec4 ld=vec4(1,1,1,1); //Kolor swiatla rozpraszanego

    vec4 l=normalize(V*lp-V*M*vertex); //Wektor do swiatla w przestrzeni oka
    vec4 n=normalize(V*M*normal); //Wektor normalny w wierzcholku w przestrzeni oka

    float nl=max(dot(n,l),0); //Kosinus kata pomiedzy wektorami do swiatla i normalnym

    iC=ka*la+kd*ld*vec4(nl,nl,nl,1); //Lambertowski model swiatla rozproszonego

	gl_Position=P*V*M*vertex;
}


