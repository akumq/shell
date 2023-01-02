#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>

typedef enum
{
    MOT,
    TUB,
    INF,
    SUP,
    SPP,
    NL,
    FIN
} LEX;

static LEX getlex(char *mot)
{
    enum
    {
        Neutre,
        Spp,
        Equote,
        Emot
    } etat = Neutre;


    int c;
    char *w;
    w = mot;
    while ((c = getchar()) != EOF)
    {
        switch (etat)
        {
        case Neutre:
            switch (c)
            {
            case '<':
                return (INF);
            case '>':
                etat = Spp;
                continue;
            case '|':
                return (TUB);
            case '"':
                etat = Equote;
                continue;
            case ' ':
            case '\t':
                continue;
            case '\n':
                return (NL);
            default:
                etat = Emot;
                *w++ = c;
                continue;
            }
        case Spp:
            if (c == '>')
                return (SPP);
            ungetc(c, stdin);
            return (SUP);
        case Equote:
            switch (c)
            {
            case '\\':
                *w++ = c;
                continue;
            case '"':
                *w = '\0';
                return (MOT);
            default:
                *w++ = c;
                continue;
            }
        case Emot:
            switch (c)
            {
            case '|':
            case '<':
            case '>':
            case ' ':
            case '\t':
            case '\n':
                ungetc(c, stdin);
                *w = '\0';
                return (MOT);
            default:
                *w++ = c;
                continue;
            }
        }
    }
    return (FIN);
}


void commande(LEX type,char *arg[],int nb_arg){
   
    if (strstr(arg[0],"cd") != NULL){
        if (chdir(arg[1]) == -1){
            printf("Erreur dans le changement de fichier \n");
        }

        return;
    }

    int pid,status;
    switch (pid = fork())
    {
    case -1:
        perror("Erreur de Fork");
        break;
    case 0:
        execvp(arg[0],arg);
        break;
    default:
        waitpid(pid,&status, WUNTRACED | WCONTINUED);
        break;
    }

}

void exec_fileappend(char *arg[],int nb_arg,char file[]){
    char path[PATH_MAX];
    int pid,status;

    int original_stdout = dup(STDOUT_FILENO);
    getcwd(path,sizeof(path));
    switch (pid = fork())
    {
    case -1:
        perror("Erreur de Fork");
        break;
    case 0:
        char *filePath = malloc(strlen(path) + strlen(file) + 2);
        sprintf(filePath, "%s/%s", path, file);
        int fd;
        printf("%s \n",filePath);
        fd = open(filePath, O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd < 0){
            printf("Erreur de file Descriptor\n");
            exit(1);
        }
        if(dup2(fd,STDOUT_FILENO) < 0){
            printf("Erreur de duplication de descripteur");
            exit(1);
        }
        execvp(arg[0],arg);
        if(dup2(original_stdout,STDOUT_FILENO) < 0){
            printf("Erreur de restauration de la sortie standard\n");
            exit(1);
       }
        break;
    default:
        wait(&status);
        break;
    }
}



void exec_file(char *arg[],int nb_arg,char file[]){
    char path[PATH_MAX];
    int pid,status;

    int original_stdout = dup(STDOUT_FILENO);
    getcwd(path,sizeof(path));
    switch (pid = fork())
    {
    case -1:
        perror("Erreur de Fork");
        break;
    case 0:
        char *filePath = malloc(strlen(path) + strlen(file) + 2);
        sprintf(filePath, "%s/%s", path, file);
        int fd;
        printf("%s \n",filePath);
        fd = open(filePath, O_WRONLY | O_CREAT, 0666);
        if (fd < 0){
            printf("Erreur de file Descriptor\n");
            exit(1);
        }
        if(dup2(fd,STDOUT_FILENO) < 0){
            printf("Erreur de duplication de descripteur");
            exit(1);
        }
        execvp(arg[0],arg);
        if(dup2(original_stdout,STDOUT_FILENO) < 0){
            printf("Erreur de restauration de la sortie standard\n");
            exit(1);
       }
        break;
    default:
        wait(&status);
        break;
    }
}

int execpipe (char ** argv1, char ** argv2) {
    int fds[2];
    pipe(fds);
    int i;
    int cpid,status;
    switch (cpid = fork())
    {
    case -1:
        perror("Erreur de Fork");
        break;
    case 0:
        pid_t pid = fork();
        if (pid == -1) { //error
            printf("error fork!!\n");
            return 1;
        } 
        if (pid == 0) { // child process
            close(fds[1]);
            dup2(fds[0], 0);
            //close(fds[0]);
            execvp(argv2[0], argv2); // run command AFTER pipe character in userinput
            printf("unknown command\n");
            exit(0);
        } else { // parent process
            close(fds[0]);
            dup2(fds[1], 1);
            //close(fds[1]);
            execvp(argv1[0], argv1); // run command BEFORE pipe character in userinput
            printf("unknown command\n");
            exit(0);
        }
        break;
    default:
        waitpid(cpid,&status, WUNTRACED | WCONTINUED);
        break;
    }

};





int main(int argc, char *argv[])
{
    char mot[200];
    char path[PATH_MAX];

    bool isPipe = false;
    bool isRedirection = false;
    bool isAppend = false;
    int pos;

    const char *lexeme_names[] = {
        "MOT",
        "TUB",
        "INF",
        "SUP",
        "SPP",
        "NL",
        "FIN",
    };


    getcwd(path,sizeof(path));
    //printf("[%s]$ ",path);
    
    printf("\033[4;33m[%s]$\033[0m ",path);

    char *arg[200];
    char *arg1[200];
    char *arg2[200];
    int nb_arg = 0;

    LEX motLex;

    while (1){
        switch (motLex = getlex(mot))
        {
        case MOT:
            arg[nb_arg] = strdup(mot);
            nb_arg += 1;
            break;
        case TUB:
            isPipe = true;
            
            for(int i =0; i < nb_arg; i++){
                arg1[i] = strdup(arg[i]);
                arg[i] = '\0';
            }
            nb_arg = 0;
            break;
        case INF:
            printf("REDIRECTION ENTREE\n");
            break;
        case SUP:           
            isRedirection = true;
            pos = nb_arg;
            break;
        case SPP:
            isAppend = true;
            pos = nb_arg;
            break;
        case NL:

            if (isPipe){
                execpipe(arg1,arg);
                isPipe = false;
            }else if (isRedirection){
                char *file = arg[pos];
                arg[pos] = NULL;
                nb_arg -= 1;
                exec_file(arg,nb_arg,file);
                isRedirection = false;
            }else if(isAppend){
                char *file = arg[pos];
                arg[pos] = NULL;
                nb_arg -= 1;
                exec_fileappend(arg,nb_arg,file);
                isAppend = false;
            }else{
                printf("%s \n",lexeme_names[motLex]);
                commande(motLex,arg,nb_arg);
            }
            for (int i = 0; i < 200; i++) {
                free(arg[i]);
                arg[i] = NULL;
            }
            nb_arg = 0;
            printf("\033[4;33m[%s]$\033[0m ",getcwd(path,sizeof(path)));
            break;
        case FIN:
            printf("FIN \n");
            exit(0);
        }

    }


}
