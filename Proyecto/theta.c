
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

    
typedef struct elemento
{
    pid_t pid;
    int estado;
    int indice;
    char* buf;
}elem_t;

elem_t* crearElemento(pid_t pid, char* buf){
    elem_t* elemAux = malloc(sizeof(elem_t));
    elemAux->buf = malloc(1024*sizeof(char));
    elemAux->pid = pid;
    elemAux->estado = 1; // 1 -> ejecutando; 0 -> ejecutado; 2 -> detenido
    elemAux->buf = strcpy(elemAux->buf, buf);
    elemAux->indice = 1;
    return elemAux;
}

void asignar(elem_t* elem1, elem_t* elem2){
    elem1->pid = elem2->pid;
    elem1->pid = elem2->pid;
    elem1->estado = elem2->estado;
    elem1->buf=elem2->buf;
    elem1->indice = elem2->indice;
}

int igual(elem_t* elem1, elem_t* elem2){
    if(elem1->pid == elem2->pid){
        return 0;
    } else {
        return 1;
    }
}

void mostrarElem(elem_t* elem){
	fprintf(stdout,"%s",elem->buf);
}

void ejecutarElem(elem_t* elem){
    elem->estado = 1;
}

void terminarElem(elem_t* elem){
    elem->estado = 0;
}

void detenerElem(elem_t* elem){
    elem->estado = 2;
}

pid_t pidElem(elem_t elem){
    return elem.pid;
}

typedef struct nodo //nombre estructura
{
	elem_t elem;
    //int indice;
	struct nodo* sig;
} nodo_t; //tipo dato estructura

void mostrarNodo(nodo_t* nodo){
    switch (nodo->elem.estado){
        case 0:
            printf("[%i] - Hecho     %s", nodo->elem.indice, nodo->elem.buf);
            break;
        case 1:
            printf("[%i] - Ejecutando     %s", nodo->elem.indice, nodo->elem.buf);
            break;
        case 2:
            printf("[%i] - Detenido     %s", nodo->elem.indice, nodo->elem.buf);
            break;
    }
}


typedef struct listaPIDInsercionFinal
{
    nodo_t* cabecera;
    nodo_t* final;
} listaPIDInsercionFinal_t;

//Constantes
#define CD "cd"
#define JOBS "jobs"
#define FG "fg"
//Lista 
listaPIDInsercionFinal_t ListaPID;
//foreground
int fg = 0;
pid_t pidglobal;



void crearVacia(listaPIDInsercionFinal_t* L){
    L->cabecera = NULL;
    L->final = NULL;
}

int esVacia(listaPIDInsercionFinal_t* L){
    return (L->cabecera == NULL) && (L->final == NULL);
}

void insertarFinal(elem_t elem, listaPIDInsercionFinal_t* L, int clasificar){
	//1 print 0 noprint
    nodo_t* nodoAux = malloc(sizeof(nodo_t));
    asignar(&(nodoAux->elem), &elem);
    nodoAux->sig = NULL;
    if(esVacia(L)){
        L->cabecera = nodoAux;
        L->final = nodoAux;
    } else {
        if(clasificar == 1){
            nodoAux->elem.indice = L->final->elem.indice + 1;
        }
        L->final->sig = nodoAux;
        L->final = L->final->sig;
    }
    if(clasificar == 1){
        printf("[%d] %d\n", nodoAux->elem.indice, nodoAux->elem.pid);
    }
}


void borrarElementoPID(pid_t pid, listaPIDInsercionFinal_t* L){
    nodo_t* ptrAnterior;
    nodo_t* ptrActual;
	ptrActual = L->cabecera;
	ptrAnterior = NULL;
    if(!esVacia(L)){
        
        while((ptrActual->sig != NULL) && pid != pidElem(ptrActual->elem)){
            ptrAnterior = ptrActual;
            ptrActual = ptrActual->sig;
        }
        
        if(ptrAnterior == NULL){
			if(pid == pidElem(ptrActual->elem)){
				if(ptrActual->sig == NULL){
					L->cabecera = NULL;
					L->final = NULL;
				
					free(ptrActual);
					
				}else{
					L->cabecera = ptrActual->sig;
					free(ptrActual);
					
				}
			}
		}else{
			if(pid == pidElem(ptrActual->elem)){
				if(ptrActual->sig == NULL){
					L->final = ptrAnterior;
					ptrAnterior->sig = NULL;
					free(ptrActual);
				
				}else{
					ptrAnterior->sig = ptrActual->sig;
					free(ptrActual);
				}
			}
		
		
		}
    }
}

void copiarLista(listaPIDInsercionFinal_t* L, listaPIDInsercionFinal_t* lAux){
    nodo_t* ptrAux;
    if(!esVacia(L) && esVacia(lAux)){
        ptrAux = L->cabecera;
        while(ptrAux != NULL){
            insertarFinal(ptrAux->elem, lAux,0);
            ptrAux = ptrAux->sig;
        }
    }
}

int limpiarLista(listaPIDInsercionFinal_t* L, int clasificar){ //1 JOBS 0 MANDATO NORMAL
	nodo_t* ptrAux;
    listaPIDInsercionFinal_t lAux;
    
    crearVacia(&lAux);
    copiarLista(L, &lAux);
	if(!esVacia(&lAux)){
		ptrAux = lAux.cabecera;
		while(ptrAux != NULL){
			if(ptrAux->elem.estado==0){
				if(clasificar == 0){
					mostrarNodo(ptrAux);
				}
				borrarElementoPID(ptrAux->elem.pid,L);
			}
			ptrAux = ptrAux->sig;
        }
	}
	return 0;
}

void mostrarLista(listaPIDInsercionFinal_t* L){ //jobs
    nodo_t* ptrAux = L->cabecera;
    while(ptrAux != NULL){
        mostrarNodo(ptrAux);
        ptrAux = ptrAux->sig;
    }
    limpiarLista(L,1);
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
nodo_t* getUltimo(listaPIDInsercionFinal_t* L){
    if (!esVacia(L)){
		return L->final;
	}else{		
		return NULL;
	}
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
	char* const* argumentos = line->commands[i].argv;	
    execvp(argumentos[0], argumentos);
	fprintf(stderr,"%s: No se encuentra el mandato\n",argumentos[0]);
    exit(1);
}


void redirStdin(char* inp){
	int dirAux;
	
	if (inp!=NULL){
		dirAux = open(inp,O_RDONLY);
		if(dirAux < 0){
			fprintf(stderr,"Fichero : Error. %s\n",strerror(errno));
            tcsetpgrp(STDIN_FILENO,pidglobal); // devolvemos poderes al padre dado que hemos fracasado
			exit(1);
		}else{
			if(dup2(dirAux,STDIN_FILENO)==-1){
				fprintf(stderr,"Error: dup2 %s\n",strerror(errno));
				tcsetpgrp(STDIN_FILENO,pidglobal); // devolver control del terminal al padre
				exit(2);
			}
			close(dirAux);
		}
		
	}
}
void redirStdout(char* output){
	int dirAux;
	
	if (output!=NULL){
		dirAux = open(output,O_WRONLY | O_CREAT | O_TRUNC , S_IRWXU);
		if(dirAux < 0){
			fprintf(stderr,"Fichero : Error. %s\n",strerror(errno));
            tcsetpgrp(STDIN_FILENO,pidglobal); // devolvemos poderes al padre dado que hemos fracasado
			exit(1);
		}else{
			if(dup2(dirAux,STDOUT_FILENO)==-1){
				fprintf(stderr,"Error: dup2 %s\n",strerror(errno));
				tcsetpgrp(STDIN_FILENO,pidglobal); // devolver control del terminal al padre
				exit(2);	
			}
			close(dirAux);
		}
		
	}
}
void redirStderr(char* outerror){
	int dirAux;
	
	if (outerror!=NULL){
		dirAux = open(outerror,O_WRONLY| O_CREAT | O_TRUNC, S_IRWXU);
		if(dirAux < 0){
			fprintf(stderr,"Fichero: Error. %s\n",strerror(errno));
            tcsetpgrp(STDIN_FILENO,pidglobal); // devolvemos poderes al padre dado que hemos fracasado
			exit(1);
		}else{
			if (dup2(dirAux,STDERR_FILENO)==-1){
				fprintf(stderr,"Error: dup2 %s\n",strerror(errno));
				tcsetpgrp(STDIN_FILENO,pidglobal); // devolver control del terminal al padre
				exit(2);	
			}
			close(dirAux);
		}
	}
}

struct sigaction devolverPoderes;
pid_t pidh; //PID del hijo que hace exec    

static void estamosEnForeground(int sig, siginfo_t *siginfo, void *context){	
    sigaction(SIGINT,&devolverPoderes,NULL);
    sigaction(SIGQUIT,&devolverPoderes,NULL);
	if(sig == SIGUSR2){
		fg = 1;
	}
}

static void devolverControl(int sig, siginfo_t *siginfo, void *context){
    printf("\n");
    kill(pidh,SIGTERM);
    tcsetpgrp(STDIN_FILENO,pidglobal); // devolver control del terminal al padre
    exit(0);
}

static void manejadorDetenido(int sig, siginfo_t *siginfo, void *context){
	
	if(sig == SIGUSR1){
		if (getpid() == pidglobal){ //Para que no entre el hijo sin hijo
			pid_t pidpadre = siginfo->si_pid;
			elem_t *aux = getElemPID(pidpadre,&ListaPID);
			if(aux !=NULL){
				detenerElem(aux);
            }
		} 
	}
}

int escribirPrompt(){
    char* dirActual = NULL;
    dirActual = getcwd(dirActual, 0);
    char* inicio = getenv("HOME");
    if(inicio == NULL) {
        return 0;
    }
    if(!strcmp(inicio, dirActual)){
        printf("%s@msh | ~ >> ", getenv("LOGNAME"));
    } else {
        printf("%s@msh | %s >> ", getenv("LOGNAME"), dirActual);
    }
    return 1;
}

void esperarHijos(){
    int status;
    pid_t aux = waitpid((pid_t)-1,&status,WNOHANG|WUNTRACED);
    if(aux > 0){
        //printf("Me ha comentado sus movidas %i\n", aux);
        elem_t* elemAux = getElemPID(aux, &ListaPID);
        if(WIFSTOPPED(status)){
            detenerElem(elemAux);
        }
        if(WIFEXITED(status)){
            terminarElem(elemAux);
        }
    }
    
}

int main() {
    // Definición de variables
    char buf [1024];
    int i;
    pid_t pid;
    tline* line;
    int status;
    int stdinAux = dup(0);	//guardamos stdin actual
    int **pipes;
    
    pidglobal = getpid();
        
    struct sigaction marcarDetenido;
    marcarDetenido.sa_sigaction = &manejadorDetenido;
    marcarDetenido.sa_flags = SA_SIGINFO | SA_RESTART;
    sigaction(SIGUSR1,&marcarDetenido,NULL); //No falla porque es una señal que podemos capturar
    
    struct sigaction marcarFg;
    marcarFg.sa_sigaction = &estamosEnForeground;
    marcarFg.sa_flags = SA_SIGINFO | SA_RESTART;
    sigaction(SIGUSR2,&marcarFg,NULL); //No falla porque es una señal que podemos capturar
    
    crearVacia(&ListaPID);
    
    devolverPoderes.sa_sigaction = &devolverControl;
    devolverPoderes.sa_flags = SA_SIGINFO | SA_RESTART;
    
    signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
    
    while(escribirPrompt() && fgets(buf, 1024, stdin)){
		//Comprobar y limpiar lista de pids
        esperarHijos();
        line = tokenize(buf);
        if (line==NULL || !strcmp(buf,"\n")) {
            esperarHijos();
            limpiarLista(&ListaPID,0);
            continue;
        } else {
            if(strcmp(line->commands[0].argv[0],JOBS)==0){
                esperarHijos();
                mostrarLista(&ListaPID);
                continue;
            }else if(strcmp(line->commands[0].argv[0],CD)==0){
                cambiarDirectorio(line->commands[0].argc,line->commands[0].argv);
                esperarHijos();
                limpiarLista(&ListaPID,0);
                continue;
            }
            else if(strcmp(line->commands[0].argv[0],FG)==0){
                nodo_t *ultimonodo=getUltimo(&ListaPID);
                if(ultimonodo == NULL){
                    printf("fg: actual: no existe tal trabajo\n");
                    continue;
                }
                mostrarElem(&ultimonodo->elem); //Mostramos el buffer
                switch(ultimonodo->elem.estado){
                    case 1: // en ejecución
                        //Darle control
                        kill(ultimonodo->elem.pid,SIGUSR2); // indicamos al hijo que ha de aceptar ctrl C, no puede fallar
                        tcsetpgrp(STDIN_FILENO,ultimonodo->elem.pid); // pasamos poderes al hijo
                        int status;
                        waitpid(ultimonodo->elem.pid, &status, 0);
                        if(WIFEXITED(status)){
                            terminarElem(&ultimonodo->elem);
                        }
                        break;
                    case 2: // detenido
                        ejecutarElem(&ultimonodo->elem);
                        kill(ultimonodo->elem.pid,SIGCONT); 
                        kill(ultimonodo->elem.pid,SIGUSR2); // indicamos al hijo que ha de aceptar ctrl C
                        tcsetpgrp(STDIN_FILENO,ultimonodo->elem.pid);
                        waitpid(ultimonodo->elem.pid, &status, 0);
                        if(WIFEXITED(status)){
                            terminarElem(&ultimonodo->elem);
                        }
                        break;
                    case 0: // hecho
                        fprintf(stdout, "msh: fg: El trabajo ha terminado");
                        break;
                }
                esperarHijos();
                limpiarLista(&ListaPID,0);
                continue;
            }
            
			pid = fork();
			
			if (!esHijo(pid)){ // si es padre
                setpgid(pidglobal,pidglobal);
				if(!line->background){ // mandato en fg
                    kill(pid,SIGUSR2); // indicarle al hijo que puede actuar ante SIGINT y SIGQUIT
                    tcsetpgrp(STDIN_FILENO, pid); // pasarle el control al hijo, no falla
					waitpid(pid,&status,0); // esperar hijo
				}else{
					//Tratar buf
					elem_t *elem = crearElemento(pid,buf);
					insertarFinal(*elem, &ListaPID,1);
				}
				esperarHijos();
				// saber sin esperar en qué estado están los hijos
				limpiarLista(&ListaPID,0);
				continue;
			}else{ // si es hijo
				//Redireccionamientos
				if(setpgid(getpid(), getpid()) < 0){
                    fprintf(stderr,"Error: setpgid\n");
                    tcsetpgrp(STDIN_FILENO, pidglobal); // pasarle el control al padreshell
					exit(1);
                }
				redirStderr(line->redirect_error); //pasamos la redireccion de error 
				redirStdin(line->redirect_input); //pasamos la redireccion de entrada 
				redirStdout(line->redirect_output); //pasamos la redireccion de salida 
			}
         }
        
        
        
        if(line->ncommands == 1){
            pid = fork();
            errorFork(pid);
            if(esHijo(pid)){
                ejecutarComando(0,line);
            }else{
				pidh = pid;
                waitpid(pid,&status,WUNTRACED);
                if(WIFSTOPPED(status) != 0){
                    // lanzar señal para que nos marque como detenidos
                    kill(pid, SIGCONT); // indicar al hijo que ha de continuar, no puede fallar por como lo hemos definido
                    waitpid(pid, &status, 0); // ya podemos esperar al hijo
                }
                if (WTERMSIG(status)==SIGINT || WTERMSIG(status)==SIGQUIT){
                    printf("\n");	
				}
				if(!line->background || fg){
					if (dup2(stdinAux,STDIN_FILENO)==-1){
                    	fprintf(stderr,"Error: dup2 %s\n",strerror(errno));
						tcsetpgrp(STDIN_FILENO,pidglobal); // devolver control del terminal al padre
						exit(2);
					}
                    tcsetpgrp(STDIN_FILENO,pidglobal); // devolver control del terminal al padre
                }
				exit(status); //devolvemos el status de su hijo
            }
        } else {
            pipes = malloc((line->ncommands - 1)*sizeof(int*));//inicializamos n-1 pipes
            if(pipes == NULL){
				fprintf(stderr, "Error: malloc\n");
				tcsetpgrp(STDIN_FILENO,pidglobal); // devolver control del terminal al padre
                exit(2);
			}
            for(i = 0; i < line->ncommands - 1; i++){
                pipes[i] = malloc(2*sizeof(int)); // para cada pipe inicializamos dos enteros
                if(pipes[i] == NULL){
					fprintf(stderr, "Error: malloc\n");
					tcsetpgrp(STDIN_FILENO,pidglobal); // devolver control del terminal al padre
					exit(2);
				}
                if(pipe(pipes[i])==-1){// para cada pipe inicializamos sus descriptores
						fprintf(stderr,"Error: pipe %s\n",strerror(errno));
						tcsetpgrp(STDIN_FILENO,pidglobal); // devolver control del terminal al padre
						exit(2);
				} 
                
            }
            for (i = 0; i < line->ncommands; i++) { // Ejecución de la línea
                pid = fork();
                errorFork(pid);
                if(esHijo(pid)){ // Hijo
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
					pidh = pid;
					if(i != line->ncommands - 1){
                        close(pipes[i][1]); //cerramos el descriptor para que no se bloqueen las lecturas del hijo siguiente
                    }
                    waitpid(pid, &status, WUNTRACED); // esperar al i-ésimo hijo
                    if (WTERMSIG(status)==SIGINT || WTERMSIG(status)==SIGQUIT){
                        tcsetpgrp(STDIN_FILENO,pidglobal); // devolver control del terminal al padre
                        printf("\n");
						break;
					}
                    if(i == (line->ncommands - 1) && fg){ // estamos en fg
                        tcsetpgrp(STDIN_FILENO,pidglobal); // devolver control del terminal al padre
                    }
                }
            }
            // después del for, padre cierra descriptores abiertos
            for(i = 0; i < line->ncommands - 1; i++){              
                free(pipes[i]); // para cada pipe inicializamos dos enteros
            }
            free(pipes);
        }
        exit(status);
    }    
    return 0;
}
