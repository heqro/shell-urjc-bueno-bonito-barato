
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

    
//Lista para manejar los bg
typedef struct nodo //nombre estructura
{
	pid_t pid;
	int indice;
	int estado; //1 es EJECUTANDO, 0 es YA SE HA EJECUTADO
	char *buf; 
	struct nodo *sig;
} nodoL; //tipo dato estructura


typedef nodoL *lista;


	
	
 
	


int insertarFinal ( lista *L, pid_t pid,char mandato[1024]){ //No añadimos el estado porque siempre entran ejecutando
	//FUNCIONA
	nodoL * aux; //Nodo a añadir
	
	lista *listaaux = NULL; //Nodo cabeza de la lista
	
	listaaux = L;
	
	aux = malloc(sizeof(nodoL)); //Crear un nuevo nodo.
	
	
	aux -> pid = pid; //Asignar el valor pid al nodo.
	aux -> estado = 1; //Asignar el valor estado de ejecutando al nodo.
	
	
	aux -> buf = strcat(mandato,""); //Asignar el valor del string mandato al nodo.
	
	aux -> sig = NULL; //Apuntar el nodo al nodo que apuntaba la lista.
	
	if(*L==NULL){ //Lista vacia
		aux -> indice = 1;
		*L = aux;
		return 0;
	}
	while((*listaaux)->sig != NULL){ //Lista no vacia
		listaaux = &((*listaaux)->sig); //Llegamos hasta el final
	}
	
	aux -> indice = ((*listaaux)->indice) + 1; //Tomamos el indice del ultimo + 1
	(*listaaux)->sig = aux;
	return 0;
	
}
int borrarPID(lista *L,pid_t pid); //Para poder usarse en eliminarHechos



int eliminarHechos(lista *L){
	
	lista *listaaux = L; //Nodo cabeza de la lista
	
	while(*listaaux != NULL){
		if((*listaaux)->estado == 0){
			borrarPID(listaaux,(*listaaux)->pid); //PID es unico podemos hacerlo sin problema
												  //No hay problema de eficiencia porque esta a 
												  //la cabeza siempre										
		}
		listaaux = &((*listaaux)->sig);
	}
	return 0;
}

int mostrar(lista *L){
	//FUNCIONA
		//Debera mostrar los valores
		lista *aux2 = L;
		aux2 = L;
		
		while(*aux2 != NULL){ //Mientras cabeza no sea NULL
		if((*aux2)->estado == 1){
			printf("[%i] Ejecutando %s\n",(*aux2)->indice,(*aux2)->buf); //Imprimimos el valor del nodo
		}else{
			printf("[%i] Hecho %s\n",(*aux2)->indice,(*aux2)->buf); //Imprimimos el valor del nodo
		
		}
        aux2 =&((*aux2)->sig); //Pasamos al siguiente nodo
		}
    eliminarHechos(L);
	return 0;
}

int borrarPID(lista *L, pid_t pid){
	
	lista *listapre = NULL;
	
	lista *listapost = L; //Nodo cabeza de la lista
	fprintf(stderr, "BON DIA 1\n");
	if(L != NULL){
	while((*listapost)->pid!=pid && (*listapost)->sig != NULL){
		*listapre = *listapost;
		listapost=&((*listapost)->sig);
	}
	//Si sale porque el siguiente es nulo
	if((*listapost)->sig==NULL){
		
	fprintf(stderr, "BON DIA 5\n");
		if(*listapre==NULL){
			if((*listapost)->pid==pid){
				*L = NULL;
				free(*listapost);
				return 0;
			}		
		}else{
			if((*listapost)->pid==pid){
				(*listapre)->sig = NULL;
				free(*listapost);
				return 0;
			}
			
		}
		return 0;
	}
	//Si sale porque tiene mismo pid
	fprintf(stderr, "BON DIA 2\n");
	//Borramos listapost y enlazamos las correspondientes
	if(listapre!=NULL){
			fprintf(stderr, "BON DIA 4\n");
		(*listapre)->sig = (*listapost) -> sig;
		free(*listapost);
		return 0;
	}else{
			fprintf(stderr, "BON DIA 3\n");
		*L = (*listapost)->sig;
		free(*listapost); //hago free del nodo
		
	}
	}//if
	return 0;
}
int actualizarL(lista *L,pid_t pid){  // En la lista L tiene que actualizar el estado de
										// ejecutando del nodo que tiene a pid al estado terminado
	//FUNCIONA
	
	lista *listaaux ; //Nodo cabeza de la lista
	listaaux = L;
	
	while((*listaaux)->pid != pid && (*listaaux)->sig != NULL){
		listaaux = &((*listaaux)->sig);	
	}
	
	if((*listaaux)->sig == NULL){
		if((*listaaux)-> pid == pid){
			(*listaaux)->estado = 0;
			return 0;
		}
		return 0;
	}else{
		(*listaaux)->estado = 0; //Asignamos el valor de Hecho al nodo con ese pid
		return 0;
	}
	
	
	
	
	
	
}
int main(){
	char buf[1024];
	
	lista ListaPID = NULL;
	

	
	fgets(buf,1024,stdin);
	
	
	insertarFinal(&ListaPID,14444,buf);
	
	insertarFinal(&ListaPID,14555,buf);
	
	insertarFinal(&ListaPID,14556,buf);
	
	insertarFinal(&ListaPID,14557,buf);
	
	insertarFinal(&ListaPID,14558,buf);
	actualizarL(&ListaPID,14554);
	actualizarL(&ListaPID,14555);
	
	mostrar(&ListaPID);
	
	
	mostrar(&ListaPID);
	actualizarL(&ListaPID,14557);
	
	
	mostrar(&ListaPID);
	
	mostrar(&ListaPID);


















return 0;
}
