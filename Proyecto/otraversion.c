
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

int comprobarComando(char* comando){
    int esCd, esFg, esJobs;
    esCd = strcmp(comando, "cd");
    esFg = strcmp(comando, "fg");
    esJobs = strcmp(comando, "jobs");
    return esCd || esFg || esJobs;
}

void ejecutarComando(int i, tline* line){
    for (int j = 0; j<line->commands[i-1].argc; j++) {
                execvp(line->commands[i-1].filename, line->commands[i-1].argv[j]);
    
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
                if(i % 2 == 0){ // el hijo ejecuta comandos de i par
                    ejecutarComando(i, line);
                }
            } else {//Padre
                if (i == 1){//leer de stdin o bien redirect-input
                    crearPrimero(pipe_ph, line);
                }
                if (i == 2){ // primera iteración del bucle "cerrado"
                    close(pipe_hp[0]);
                    close(STDIN_FILENO);
                    dup(pipe_hp[0]);
                    close(STDOUT_FILENO);
                    dup(pipe_ph[1]);
                }
                if (i == line->ncommands){ // último mandato: restaurar stdout o redirect-output
                    dup2(stdoutAux, 1);
                    close(stdoutAux);
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
        printf("msh> ");
    }
    return 0;
}
