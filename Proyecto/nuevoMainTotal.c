
#include <stdio.h>
#include <string.h>
#include "parser.h"
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
	struct nodo *sig;
} nodoL; //tipo dato estructura

typedef nodoL *lista;

void nuevaLista(lista L){
	L = NULL;
}
void insertarDelante (lista *L, pid_t n){
	lista aux;
	aux = malloc(sizeof(nodoL)); //Crear un nuevo nodo.
	aux -> pid = n; //Asignar el valor al nodo.
	aux -> sig = *L; //Apuntar el nodo al nodo que apuntaba la lista.
	if (*L==NULL){
		aux->indice = 0;
	}else{
		(*L)->indice = aux->indice;
	}
	*L=aux; //Hacer que la lista apunte al nodo nuevo.
	
	
}/* inserta el valor n al frente de la lista */

int mostrar(lista L){
		//Debera mostrar los valores
		while(L != NULL){ //Mientras cabeza no sea NULL
        printf("%i\n",L->pid); //Imprimimos el valor del nodo
        L = L->sig; //Pasamos al siguiente nodo
    }
		return 0;
}
int borrarElemento(lista *L, int nelemento){
	
	int i=0;
	int j=0;
	lista L4,L2,L0;
	lista L1=*L;
	if(*L==NULL) {return 0;}
	if(nelemento==0){
		*L=(*L)->sig;
		free(L1);
		return 0;
	}
	while(i!=nelemento){
		L0=L1;
		L1=L1->sig;
		i=i+1;
	}
	
	//L0 antes de L1 y L2 despues de L1, L1 es el nodo a borrar
	//Si L1 es el ultimo nodo L2->sig sera null luego no hay mas casos que distinguir
	
	L2=L1->sig;
	L0->sig=L2;
	free(L1);
	
	//Tenemos que recontar los indices
	L4=*L;
	if(*L==NULL) {
		(*L)->indice=0;
		return 0;
	}
	
	while(L4!=NULL){
		L4->indice=j;
		L4=L4->sig;
		j=j+1;
	}
	
	
	return 0;
	


}

int cambiarDirectorio(int nArgs, char** lArgs){
    char* nuevoDir = NULL;
    if (nArgs == 1) { // ir a home
        nuevoDir = getenv("HOME");
    } else {
        if (nArgs > 2) {
            // número incorrecto de argumentos
            // lanzar mensaje
            exit(1);
        }
        if (*(lArgs[1]) != '/'){//ruta relativa
            nuevoDir = getcwd(nuevoDir, 0);
            char* barra = malloc(sizeof(char));
            *barra = '/';
            nuevoDir = strcat(nuevoDir, barra);
            nuevoDir = strcat(nuevoDir, lArgs[1]);
        } else { // ruta absoluta
            nuevoDir = lArgs[1];
        }
    }
    chdir(nuevoDir);
    printf("%s", nuevoDir);
    return 0;
}


void crearPrimero(int pipe[2], tline* line){
    
    close(pipe[1]);
    dup2(pipe[1],STDOUT_FILENO);
    
    // comprobación de error
    if(line->redirect_input){//hay redirección de entrada
        int aux = open(line->redirect_input, O_RDONLY);
        dup2(aux,STDIN_FILENO);
        close(aux);
    }
    
}

void crearUltimo(int pipe[2], tline* line,int* stdoutAux,int* stdinAux){
    close(pipe[1]); // Nunca utilizaremos el pipe de salida, por lo que lo cerramos
    
    if(line->redirect_output) { //hay redirección de salida
        //pipe[1] = open(line->redirect_output, O_CREAT, O_WRONLY);
        dup2(pipe[1],open(line->redirect_output, O_CREAT, O_WRONLY));
        
    } else { //Escuchar por stdout
		dup2(*stdoutAux,1);
        close(*stdoutAux);
        dup2(*stdinAux,0);
        close(*stdinAux);
    }
}

int esHijo(pid_t pid){
    return pid == 0; //devuelve 1 si es hijo
}

void errorFork(pid_t pid){
    if(pid < 0){
        fputs("Fallo en el fork", stderr);
        exit(1);
    }
}

int comprobarComando(char* comando){
    int esCd, esFg, esJobs;
    esCd = strcmp(comando, "cd");
    esFg = strcmp(comando, "fg");
    esJobs = strcmp(comando, "jobs");
    return esCd || esFg || esJobs;
}


void ejecutarComando(int i, tline* line){
	int status;
    char* const* argumentos = line->commands[i].argv;	
    execvp(argumentos[0], argumentos);
    fprintf(stderr,"Error al lanzar el comando. %s\n",strerror(errno));
    exit(1);
}


void manejadorSigUsr1(){
    
}

int main() {
    // Definición de variables
    char buf [1024];
    int i;
    pid_t pid;
    tline* line;
    int status;
    int stdoutAux = dup(1); //guardamos stdout actual
    int stdinAux = dup(0);	//guardamos stdin actual
    int stderrAux = dup(2);
    
    int **pipes;
    
    //signal(SIGUSR1,manejadorSigUsr1);
    
    printf("msh> ");
    while(fgets(buf, 1024, stdin)){
        line = tokenize(buf);
        if (line==NULL) {
            continue;
        } else {
            if(line->background){
                pid = fork(); // Vamos a tener que ejecutar al menos un mandato
                errorFork(pid); // ¿Ha habido algún error?
                if(!esHijo(pid)){
                    continue; // El padre no hace nada más
                }
            }
        }
        
        if(line->ncommands == 1){
            pid = fork();
            if(esHijo(pid)){
                ejecutarComando(0,line);
            }
        } else {
            pipes = malloc((line->ncommands - 1)*sizeof(int*));//inicializamos n-1 pipes
            for(i = 0; i < line->ncommands - 1; i++){
                pipes[i] = malloc(2*sizeof(int)); // para cada pipe inicializamos dos enteros
                pipe(pipes[i]); // para cada pipe inicializamos sus descriptores
            }
            for (i = 0; i < line->ncommands; i++) { // Ejecución de la línea
                pid = fork();
                errorFork(pid);
                if(esHijo(pid)){ // Hijo
                    if (i == 0){ // si es la primera iteración
                        close(pipes[i][0]);
                        dup2(pipes[i][1],STDOUT_FILENO);
                        close(pipes[i][1]);
                    }
                    if (i != 0 && i != (line->ncommands-1)){
                        // si no es ni la primera ni la última iteración
                        close(pipes[i-1][1]);
                        close(pipes[i][0]);
                        dup2(pipes[i-1])[0],STDIN_FILENO);
                        close(pipes[i-1][0]);
                        dup2(pipes[i][1], STDOUT_FILENO);
                        close(pipes[i][1]);
                    }
                    if (i == (line-> ncommands - 1)){
                        close(pipes[i-1][1]);
                        dup2(pipes[i-1][0], STDIN_FILENO);
                        close(pipes[i-1][0]);
                    }
                    ejecutarComando(i, line);//
                } else { // Padre
                    wait(&status);
                    if(WIFEXITED(status)!=0){
                        if(WEXITSTATUS(status)!=0){
                            fprintf(stderr,"Error - el comando no se ejecutó correctamente. %s\n",strerror(errno));
                            break;
                        }
                    }
                }
                
            }
        }
        wait(NULL);
        printf("msh> ");
        fflush(stdout);
    }
    return 0;
}
