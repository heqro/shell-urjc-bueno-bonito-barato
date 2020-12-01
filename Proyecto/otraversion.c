
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
void crearPrimero(int pipe[2], tline* line){
    close(pipe[1]);
    close(STDOUT_FILENO);
    dup(pipe[1]);
    
    // comprobación de error
    if(line->redirect_input){//hay redirección de entrada
        int aux = open(line->redirect_input, O_RDONLY);
        dup2(aux,STDIN_FILENO);
        close(aux);
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

    int stdoutAux = dup(1);
    

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
			close(pipe_hp[0]);
            close(pipe_ph[1]);
            
            
            
            // Caso General
//             close(STDIN_FILENO);
//             dup(pipe_ph[0]);
//             close(STDOUT_FILENO);
//             dup(pipe_hp[1]);
		} else{//Padre
            close(pipe_hp[1]);
            close(pipe_ph[0]);
            
            // Caso general
//             close(STDIN_FILENO);
//             dup(pipe_hp[0]);
//             close(STDOUT_FILENO);
//             dup(pipe_ph[1]);
		}
		
		
		
		
        errorFork(pid);
        for (i = 1; i <= line->ncommands; i++) {       
            if (esHijo(pid)){//Hijo
                if(i == 2){
                    // Hacer que el stdin del hijo pase a ser ph[0]
                    close(STDIN_FILENO);
                    dup(pipe_ph[0]);
                    // Hacer que el stdout del hijo lo escribimos en hp[1]
                    close(pipe_hp[1]);
                    close(STDOUT_FILENO);
                    dup(pipe_hp[1]);
                }
                if (i == line->ncommands){ // Restaurar STDOUT para poder escribir en él
                    // Restaurar STDOUT
                    dup2(stdoutAux, 1);
                    close(stdoutAux);
                    // Cerrar pipes, hemos acabado
                    
                }
                if(i % 2 == 0){
                    // ejecutarComando(i, line);
                }
                    
            } else {//Padre
                if (i == 1){//leer stdin - redirect_input
                    crearPrimero(pipe_ph, line);
                }
                if (i == 2){
                    close(pipe_hp[0]);
                    close(STDIN_FILENO);
                    dup(pipe_hp[0]);
                    close(STDOUT_FILENO);
                    dup(pipe_ph[1]);
                }
                if (i == line->ncommands){ // Restaurar STDOUT para poder escribir en él
                    dup2(stdoutAux, 1);
                    close(stdoutAux);
                }
                
                if(i % 2 == 1){
                    //Ejecutar comando
                    //ejecutarComando(i, line);
                    for (j=0; j<line->commands[i-1].argc; j++) {
                        printf("  argumento %d: %s\n", j, line->commands[i-1].argv[j]);
                    }
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
        printf("msh> ");
    }
    return 0;
}
