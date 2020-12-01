#include <stdio.h>
#include <string.h>
#include "parser.h"
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
void crearPrimero(int pipe[2], tline* line){
    close(pipe[0]);//nunca utilizaremos el pipe de entrada
    close(STDIN_FILENO);
    if(line->redirect_input){//hay redirección de entrada
        pipe[0] = open(line->redirect_input, O_RDONLY);
    }else{//Escuchar por stdin
        dup(pipe[0]);
    }
}

void crearUltimo(int pipe[2], tline* line){
    close(pipe[1]); // Nunca utilizaremos el pipe de salida, por lo que lo cerramos
    close(STDOUT_FILENO);
    if(line->redirect_output) { //hay redirección de salida
        pipe[1] = open(line->redirect_output, O_CREAT, O_WRONLY);
    } else { //Escuchar por stdout
        dup(pipe[1]);
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

int main() {
    // Definición de variables
    char buf [1024];
    int i, j;
    pid_t pid;
    tline* line;
    int pipe_ph[2];
    int pipe_hp[2];


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
        
        pipe(pipe_ph); // Creamos pipe para conectar procesos
        pipe(pipe_hp);
        pid = fork();
        if (esHijo(pid)){//Hijo
			close(pipe_ph[0]);
			close(pipe_hp[1]);
			//pipe_ph[1] sera el stdin del hijo
			close(STDIN_FILENO); //cerramos stdin
			dup(pipe_ph[1]);
			close(STDOUT_FILENO);
			dup(pipe_hp[0]);
			
			
			
			
		} else{//Padre
			
			close(pipe_ph[1]);
			close(pipe_hp[0]);
			//pipe_hp[1] sera el stdin del padre
			close(STDIN_FILENO); //cerramos stdin
			if(line->redirect_input){//hay redirección de entrada
				//funcion comprobar fichero
				pipe_ph[0] = open(line->redirect_input, O_RDONLY);
			}else{//Escuchar por stdin
				dup(pipe_ph[0]);
			}
			close(STDOUT_FILENO);
			dup(pipe_hp[0]);
			
		}
		
		
		
		
        errorFork(pid);
        for (i = 1; i <= line->ncommands; i++) {
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
        }
        printf("msh> ");
    }
    return 0;
}
