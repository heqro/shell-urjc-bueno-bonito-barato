
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

    
//Lista para manejar los bg
typedef struct elemento
{
    pid_t pid;
    int estado;
    char* buf;
}elem_t;

elem_t* crearElemento(pid_t pid, char* buf){
    elem_t* elemAux = malloc(sizeof(elem_t));
    elemAux -> pid = pid;
    elemAux -> estado = 1; // 1 -> ejecutando; 0 -> ejecutado;
    elemAux -> buf = buf;
    return elemAux;
}

void asignar(elem_t* elem1, elem_t* elem2){
    elem1->pid = elem2->pid;
    elem1->pid = elem2->pid;
    elem1->estado = elem2->estado;
    elem1->buf = elem2->buf;
}

int igual(elem_t* elem1, elem_t* elem2){
    if(elem1->pid == elem2->pid){
        return 0;
    } else {
        return 1;
    }
}

void terminarElem(elem_t* elem){
    elem->estado = 0;
}

pid_t pidElem(elem_t elem){
    return elem.pid;
}

typedef struct nodo //nombre estructura
{
	elem_t elem;
    int indice;
	struct nodo* sig;
} nodo_t; //tipo dato estructura

void mostrarNodo(nodo_t* nodo){
    if(nodo->elem.estado == 0){
        printf("[%i] - Hecho\n", nodo->indice);
    }else{
        printf("[%i] - Ejecutando\n", nodo->indice);
    }
}

typedef struct listaPIDInsercionFinal
{
    nodo_t* cabecera;
    nodo_t* final;
} listaPIDInsercionFinal_t;

void crearVacia(listaPIDInsercionFinal_t* L){
    L->cabecera = NULL;
    L->final = NULL;
}

int esVacia(listaPIDInsercionFinal_t* L){
    return (L->cabecera == NULL) && (L->final == NULL);
}

void insertarFinal(elem_t elem, listaPIDInsercionFinal_t* L){
    if(esVacia(L)){
        L->cabecera = malloc(sizeof(nodo_t));
        asignar(&(L->cabecera->elem), &elem);
        L->cabecera->indice = 1;
        L->final = L->cabecera;
        printf("[1] %d\n", L->cabecera->elem.pid);
    } else {
        nodo_t* ptrAux = malloc(sizeof(nodo_t));
        asignar(&ptrAux->elem, &elem);
        ptrAux->indice = L->final->indice + 1;
        L->final->sig = ptrAux;
        ptrAux->sig = NULL;
        L->final = ptrAux;
        printf("[%d] %d\n", ptrAux->indice, ptrAux->elem.pid);
    }
}


void borrarElementoPID(pid_t pid, listaPIDInsercionFinal_t* L){
    nodo_t* ptrAnterior;
    nodo_t* ptrActual;
    if(!esVacia(L)){
        ptrActual = L->cabecera;
        while(pid != pidElem(ptrActual->elem)){
            ptrAnterior = ptrActual;
            ptrActual = ptrActual->sig;
        }
        ptrAnterior->sig = ptrActual->sig;
        free(ptrActual);
    }
}

void mostrarLista(listaPIDInsercionFinal_t* L){
    nodo_t* ptrAux = L->cabecera;
    while(ptrAux != NULL){
        mostrarNodo(ptrAux);
        ptrAux = ptrAux->sig;
    }
}

elem_t* getElemPID(pid_t pid, listaPIDInsercionFinal_t* L){
    nodo_t* ptrAux = L->cabecera;
    while(ptrAux != NULL){
        if(pid == pidElem(ptrAux->elem)){
            return &ptrAux->elem;
        } else {
            ptrAux = ptrAux->sig;
        }
    }
    return NULL;
}


int main(){
	char buf[1024];
	
	listaPIDInsercionFinal_t ListaPID;
	crearVacia(&ListaPID);

	
	fgets(buf,1024,stdin);
	elem_t* elem1 = crearElemento(1444, buf);
	insertarFinal(*elem1, &ListaPID);
    elem_t* elem2 = crearElemento(1445, buf);
	insertarFinal(*elem2, &ListaPID);
    elem_t* elem3 = crearElemento(1446, buf);
	insertarFinal(*elem3, &ListaPID);
    elem_t* elem4 = crearElemento(1447, buf);
	insertarFinal(*elem4, &ListaPID);
    elem_t* elem5 = crearElemento(1448, buf);
	insertarFinal(*elem5, &ListaPID);
    mostrarLista(&ListaPID);
    elem_t* elemAux = getElemPID(1447, &ListaPID);
    terminarElem(elemAux);
    borrarElementoPID(1447,&ListaPID);
    mostrarLista(&ListaPID);
	



return 0;
}

