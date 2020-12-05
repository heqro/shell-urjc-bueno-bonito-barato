
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
	int estado; //1 es EJECUTANDO, 0 es YA SE HA EJECUTADO
	char buf[1024]; 
	struct nodo *sig;
} nodoL; //tipo dato estructura

typedef nodoL *lista;

void nuevaLista(lista L){
	L = NULL;
}




lista *ListaPID = NULL;




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
		if(L->estado == 1){
			printf("[%i] Ejecutando %s\n",L->indice,L->buf); //Imprimimos el valor del nodo
		}else{
			printf("[%i] Hecho %s\n",L->indice,L->buf); //Imprimimos el valor del nodo
		
		}
        L = L->sig; //Pasamos al siguiente nodo
    }
    //eliminarHechos();
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
void actualizarL(lista *L,pid_t pidh){  // En la lista L tiene que actualizar el estado de
										// ejecutando del nodo que tiene a pidh al estado terminado
	printf(" ");//Para que no warnee
	
}

int cambiarDirectorio(int nArgs, char** lArgs){
    char* nuevoDir = NULL;
    char* buf;
    char* barra;
    
    buf = NULL;
    barra = malloc(sizeof(char));
    
    if (nArgs == 1) { // ir a home
        nuevoDir = getenv("HOME");
    } else {
        if (nArgs > 2) {
            // número incorrecto de argumentos
            // lanzar mensaje
            fprintf(stderr,"Error numero incorrecto de argumentos\n");
            return 1; //No podemos matar al padre
        }
        if (*(lArgs[1]) != '/'){//ruta relativa
            nuevoDir = getcwd(nuevoDir, 0);
            *barra = '/';
            nuevoDir = strcat(nuevoDir, barra);
            nuevoDir = strcat(nuevoDir, lArgs[1]);
        } else { // ruta absoluta
            nuevoDir = lArgs[1];
        }
    }
    int aux = chdir(nuevoDir);
    if(aux==0){
		buf=getcwd(buf,0);
		printf("%s\n", buf);
	}else{
		fprintf(stderr,"Error en chdir.\n");
	}
	free(buf);
	free(barra);
	
    return aux;
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
    fflush(stdout);
	fprintf(stderr,"%s: No se encuentra el mandato\n",argumentos[0]);
    exit(1);
}


void redirStdin(char* inp){
	int dirAux;
	
	if (inp!=NULL){
		dirAux = open(inp,O_RDONLY);
		if(dirAux < 0){
			fprintf(stderr,"Fichero : Error. %s\n",strerror(errno));
			exit(1);
		}else{
			dup2(dirAux,STDIN_FILENO);
			close(dirAux);
		}
		
	}
}
void redirStdout(char* output){
	int dirAux;
	
	if (output!=NULL){
		dirAux = open(output,O_WRONLY | O_CREAT);
		if(dirAux < 0){
			fprintf(stderr,"Fichero : Error. %s\n",strerror(errno));
			exit(1);
		}else{
			dup2(dirAux,STDOUT_FILENO);
			close(dirAux);
		}
		
	}
}
void redirStderr(char* outerror){
	int dirAux;
	
	if (outerror!=NULL){
		dirAux = open(outerror,O_WRONLY| O_CREAT);
		if(dirAux < 0){
			fprintf(stderr,"Fichero : Error. %s\n",strerror(errno));
			exit(1);
		}else{
			dup2(dirAux,STDERR_FILENO);
			close(dirAux);
		}
		
	}
}
static void manejador(int sig, siginfo_t *siginfo, void *context){
	
	pid_t pidhijo = siginfo->si_pid;
	//actualizarL(&L,pid); //Solamente marcar como terminado el nodo con ese pidhijo
	
	
}

char* escribirPrompt2(){
    char* dirActual = NULL;
    dirActual = getcwd(dirActual, 0);
    char* inicio = getenv("HOME");
    char* promptShell = NULL;
    char* promptAux = NULL;
    promptShell = malloc(strlen(getenv("LOGNAME")) * sizeof(char));
    promptShell = strcpy(promptShell,getenv("LOGNAME"));
    if(!strcmp(inicio, dirActual)){ // estamos en home
        promptShell = realloc(promptShell, strlen("msh | ~ >> ") * sizeof(char));
        strcat(promptShell, "@msh | ~ >> ");
    } else {
        promptShell = realloc(promptShell, strlen("@msh | ") * sizeof(char));
        strcat(promptShell, "@msh | ");
        promptShell = realloc(promptShell, strlen(dirActual) * sizeof(char));
        strcat(promptShell, dirActual);
        promptShell = realloc(promptShell, strlen(" >> ") * sizeof(char));
        strcat(promptShell, " >> ");
    }
    promptAux = malloc(strlen(promptShell) * sizeof(char));
    strcpy(promptAux, promptShell);
    free(promptShell);
    return promptAux;
}

void escribirPrompt(){
    char* dirActual = NULL;
    dirActual = getcwd(dirActual, 0);
    char* inicio = getenv("HOME");
    if(!strcmp(inicio, dirActual)){
        printf("%s@msh | ~ >> ", getenv("LOGNAME"));
    } else {
        printf("%s@msh | %s >> ", getenv("LOGNAME"), dirActual);
    }
}

#define CD "cd"
#define JOBS "jobs"
#define FG "fg"

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
    struct sigaction siginfo;
    
    
    siginfo.sa_sigaction = &manejador;
    siginfo.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1,&siginfo,NULL);
    
    
    //signal(SIGUSR1,manejadorSigUsr1);
    signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
	
    //printf("msh> ");
    escribirPrompt();
    while(fgets(buf, 1024, stdin)){
		//Comprobar y limpiar lista de pids
		
		
        line = tokenize(buf);
        
        if (line==NULL || !strcmp(buf,"\n")) {
            escribirPrompt();
            continue;
        } else {
            if(strcmp(line->commands[0].argv[0],JOBS)==0){
                //mostrar(); este es especial para jobs
                escribirPrompt();
                continue;
            }else if(strcmp(line->commands[0].argv[0],CD)==0){
                cambiarDirectorio(line->commands[0].argc,line->commands[0].argv);
                //eliminaryMostrarLista
                escribirPrompt();
                continue;
            }
            else if(strcmp(line->commands[0].argv[0],FG)==0){
                //Recorremos la lista hasta llegar al ultimo nodo
                //Vemos si su estado es ejecutando
                //El padre shell manda un kill a su hpadre, el hpadre en ese kill hace kill a su hijo
                //y coloca la variable global a uno para los demas hijos que puedan crearse.
                //En ese caso se hace el waitpid del pid del nodo
                
                //En caso contrario escribimos por stdout que el proceso ha finalizado
                //WAITPID RECORDAR
                
                //eliminaryMostrarLista
                escribirPrompt();
                continue;
            }
					
				
				
			
			
			pid = fork();
			
			if (!esHijo(pid)){
				if(line->background==0){
					wait(&status);
					if(WTERMSIG(status)==SIGINT || WTERMSIG(status)==SIGQUIT ){
                        printf("\n");
                    } //print \n si se acaba con el hijo con Ctrl C
				}else{
					
					//añadirPorFinal(L,pid,buf); añadimos el proceso hijo bg
				}
				//eliminaryMostrarLista
				escribirPrompt();
				continue;
			}else{
				
				//Redireccionamientos
				
				redirStderr(line->redirect_error); //pasamos la redireccion de error 
				redirStdin(line->redirect_input); //pasamos la redireccion de entrada 
				redirStdout(line->redirect_output); //pasamos la redireccion de salida 
					
			}
         }
        
        
        
        if(line->ncommands == 1){
            pid = fork();
            if(esHijo(pid)){
				
				if(line->background != 0 ){ //Si no esta en bg
					signal(SIGQUIT,SIG_DFL);
					signal(SIGINT,SIG_DFL);
				}
                ejecutarComando(0,line);
                
            }else{
                wait(&status);
                if (WTERMSIG(status)==SIGINT || WTERMSIG(status)==SIGQUIT){
					printf("\n");
					exit(status); //devolvemos el status de su hijo
				}
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
					
					if(line->background!=0){
						signal(SIGQUIT,SIG_DFL);
						signal(SIGINT,SIG_DFL);
					}	
			
                    if (i == 0){ // si es la primera iteración
                        close(pipes[0][0]);
                        dup2(pipes[0][1],STDOUT_FILENO);
                        close(pipes[0][1]);
                    }
                    if (i != 0 && i != (line->ncommands-1)){
                        // si no es ni la primera ni la última iteración
                        
                        close(pipes[i][0]);
                        dup2(pipes[i-1][0], STDIN_FILENO);
                        close(pipes[i-1][0]);
                        dup2(pipes[i][1], STDOUT_FILENO);
                        close(pipes[i][1]);
                    }
                    if (i!= 0 && i == (line-> ncommands - 1)){
                        
                        dup2(pipes[i-1][0],STDIN_FILENO);
                        close(pipes[i-1][0]);
                    }
                    ejecutarComando(i, line);
                } else { // Padre
                    if(i != line->ncommands - 1){
                        close(pipes[i][1]); //cerramos el descriptor para que no se bloqueen las lecturas del hijo siguiente
                    }
                    wait(&status); // esperar al i-ésimo hijo
                    if (WTERMSIG(status)==SIGINT || WTERMSIG(status)==SIGQUIT){
						break;
					}
                    
                }
            }
            // después del for, padre cierra descriptores abiertos
            for(i = 0; i < line->ncommands - 1; i++){              
                free(pipes[i]); // para cada pipe inicializamos dos enteros
            }
            free(pipes);
        }
        fflush(stdout);
        if(line->background==0){
			exit(0);
		}else{
			kill(getppid(),SIGUSR1);
			exit(0);
		}
 

 
 
 
 
 
 
 
 
 
 
 
    }
    
    
    
    return 0;
}
