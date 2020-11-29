#include <stdio.h>
#include <string.h>
#include "parser.h"
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

void crearPrimero(int pipe[2], tline* line){
    close(pipe[0]);//nunca utilizaremos el pipe de entrada
    if(line->redirect_input){//hay redirección de salida
        close(STDIN_FILENO);
        pipe[0] = open(line->redirect_input, O_RDONLY);
    }
}

void crearUltimo(int pipe[2], tline* line){
    close(pipe[1]); // Nunca utilizaremos el pipe de salida, por lo que lo cerramos
    if(line->redirect_output) { //hay redirección de salida
        close(STDOUT_FILENO);
        pipe[1] = open(line->redirect_output, O_CREAT, O_WRONLY);
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
    int pipe_comandos[2];


    printf("msh> ");
    while(fgets(buf, 1024, stdin)){
        line = tokenize(buf);
        if (line==NULL) {
            continue;
        }
        //Siempre va a haber un proceso inicial
        pipe(pipe_comandos); // Creamos pipe para conectar procesos
        for (i = 1; i <= line->ncommands; i++) {
            pid = fork(); // Crear un proceso para ejecutar el comando i-ésimo
            errorFork(pid); // ¿fork creado correctamente?
            if(esHijo(pid)){
                if(i == line->ncommands){ // ¿es el último comando?
                    exit(0);
                }
                kill(getppid(),SIGUSR1); // que continúe el padre
            }else{ // el padre sale del bucle
                if(i == 1){ //¿Es el primer mandato?
                    crearPrimero(pipe_comandos, line);
                }
                if(i == line->ncommands){ // Somos el último comando
                    crearUltimo(pipe_comandos, line);
                }
                pause(); // esperar hasta que el hijo haya sido creado
                if(strcmp(line->commands[i-1].filename, "cd") == 0){
                    // si algún comando es cd, abortar
                    exit(1);
                }
                // Ejecutar el comando pedido
                for (j = 0; j < line->commands[i-1].argc; j++){
                    execvp(line->commands[i-1].filename, &line->commands[i].argv[j]);
                }
                //break;
            }


        }
        printf("msh> ");
    }
    return 0;
}
