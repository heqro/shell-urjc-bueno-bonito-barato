
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

    
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

void crearUltimo(int pipe[2], tline* line,int stdoutAux,int stdinAux){
    close(pipe[1]); // Nunca utilizaremos el pipe de salida, por lo que lo cerramos
    
    if(line->redirect_output) { //hay redirección de salida
        //pipe[1] = open(line->redirect_output, O_CREAT, O_WRONLY);
        dup2(pipe[1],open(line->redirect_output, O_CREAT, O_WRONLY));
        
    } else { //Escuchar por stdout
		dup2(stdoutAux,1);
        close(stdoutAux);
        dup2(stdinAux,0);
        close(stdinAux);
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
    pid_t pidaux;
    pidaux=fork();
    if(esHijo(pidaux)){
		char* const* argumentos = line->commands[i-1].argv;
		execvp(argumentos[0], argumentos);
		
		
	}else{
		wait(&status);
		if(WIFEXITED(status)!=0){
			//Error
			fputs("Error1\n",stderr);
			if(WEXITSTATUS(status)!=0){
				fputs("Error2\n",stderr);
			}
		}
		
	}				
		
}

int main() {
    // Definición de variables
    char buf [1024];
    int i, j;
    pid_t pid;
    tline* line;
    int pipe_ph[2];
    int pipe_hp[2];
	
	int const stdoutAux = dup(1); //guardamos stdout actual
    int const stdinAux = dup(0);	//guardamos stdin actual
     

    printf("msh> ");
    while(fgets(buf, 1024, stdin)){
		
        line = tokenize(buf);
        if (line==NULL) {
            continue;
        } else {
            if(line->background){
                pid = fork(); // Vamos a tener que ejecutar al menos un mandato
                if(!esHijo(pid)){
                    continue; // El padre no hace nada más
                }
            }
        }
        pipe(pipe_ph); // Creamos pipe en sentido padre -> hijo
        pipe(pipe_hp); // Creamos pipe en sentido hijo -> padre
        pid = fork();
        errorFork(pid); // ¿Ha habido algún error?
        if (esHijo(pid)){//Hijo cierra sus descriptores correspondientes
			close(pipe_hp[0]);
            close(pipe_ph[1]);
		} else{//Padre cierra sus descriptores correspondientes
            close(pipe_hp[1]);
            close(pipe_ph[0]);
		}
		
        for (i = 1; i <= line->ncommands; i++) { // Ejecución de la línea
            if (esHijo(pid)){//Hijo
                if(i == 2){ // primera iteración del bucle cerrado
                    // Hacer que el stdin del hijo pase a ser ph[0]
                    dup2(pipe_ph[0],STDIN_FILENO);
                    // Hacer que el stdout del hijo lo escribimos en hp[1]
                    close(pipe_hp[1]);
                    dup2(pipe_hp[1],STDOUT_FILENO);
                }
                if (i == line->ncommands){ // Restaurar STDOUT para poder escribir en él
                    //crearUltimo(pipe_hp,line);
                    // Restaurar STDOUT
                    dup2(stdoutAux, 1);
                    close(stdoutAux);
                    
                    // Cerrar pipes, hemos acabado
                }
                if(i % 2 == 0){ // el hijo ejecuta comandos de i par
                    ejecutarComando(i, line);
                }
            } else {//Padre
                if (i == 1){//leer de stdin o bien redirect-input
                    
                    crearPrimero(pipe_ph, line);
                    
					
                }
                if (i == 2){ // primera iteración del bucle "cerrado"
					close(pipe_hp[0]);
                    dup2(pipe_hp[0],STDIN_FILENO);
                    dup2(pipe_ph[1],STDOUT_FILENO);
                }
                if (i == line->ncommands){ // último mandato: restaurar stdout o redirect-output
                    
                    //dup2(stdoutAux,1);
                    //close(stdoutAux);
                    //dup2(stdinAux,0);
                    //close(stdinAux);
					crearUltimo(pipe_hp,line,stdoutAux,stdinAux);
                }
                
                if(i % 2 == 1){ // el padre ejecuta comandos de i impar
                    ejecutarComando(i, line);
                    
                }
            }
            if(esHijo(pid)){ // Hijo cierra descriptores abiertos
                close(pipe_hp[1]);
                close(pipe_ph[0]);
                exit(0);
            }else{ // padre cierra descriptores abiertos
                close(pipe_hp[0]);
                close(pipe_ph[1]);
                if(line->background){
                    pause();
                }
            }
            // Suicidio colectivo
        }
        wait(NULL);
        printf("msh> ");
    }
    return 0;
}
