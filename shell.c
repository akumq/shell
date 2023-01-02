Copy code
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>

// Définition d'un énumération pour représenter les types de tokens
typedef enum
{
    MOT,    // Mot ou chaîne de caractères entre guillemets
    TUB,    // Opérateur de redirection de sortie '|'
    INF,    // Opérateur de redirection d'entrée '<'
    SUP,    // Opérateur de redirection de sortie '>'
    SPP,    // Opérateur de redirection de sortie '>>'
    NL,     // Fin de ligne
    FIN     // Fin de fichier
} LEX;

// Fonction qui lit un caractère à la fois depuis l'entrée standard et utilise un automate à états finis
// pour identifier les différents types de tokens
static LEX getlex(char *mot)
{
    // États de l'automate à états finis
    enum
    {
        Neutre,   // État initial
        Spp,      // État intermédiaire pour identifier l'opérateur de redirection '>>'
        Equote,   // État intermédiaire pour identifier les chaînes de caractères entre guillemets
        Emot      // État intermédiaire pour identifier les mots simples (non entre guillemets)
    } etat = Neutre; 


    int c;             // Variable pour stocker le caractère lu
    char *w;           // Pointeur vers la chaîne de caractères où stocker le mot en cours de lecture
    w = mot;           // Initialisation du pointeur
    // Boucle qui lit un caractère à la fois depuis l'entrée standard
    
    int c;             // Variable pour stocker le caractère lu
char *w;           // Pointeur vers la chaîne de caractères où stocker le mot en cours de lecture
w = mot;           // Initialisation du pointeur
// Boucle qui lit un caractère à la fois depuis l'entrée standard
while ((c = getchar()) != EOF)
{
    // Selon l'état de l'automate, on traite le caractère de manière différente
    switch (etat)
    {
    case Neutre:
        // Si on est dans l'état initial, on vérifie le caractère courant pour identifier le type de token
        switch (c)
        {
        case '<':
            return (INF);   // Si c'est un '<', on retourne le type INF (opérateur de redirection d'entrée)
        case '>':
            etat = Spp;    // Si c'est un '>', on passe à l'état intermédiaire Spp
            continue;       // On continue la boucle sans exécuter la suite
        case '|':
            return (TUB);   // Si c'est un '|', on retourne le type TUB (opérateur de redirection de sortie)
        case '"':
            etat = Equote; // Si c'est un '"', on passe à l'état intermédiaire Equote
            continue;       // On continue la boucle sans exécuter la suite
        case ' ':
        case '\t':
            continue;       // Si c'est un espace ou une tabulation, on ignore et on continue la boucle
        case '\n':
            return (NL);    // Si c'est un fin de ligne, on retourne le type NL
        default:
            etat = Emot;   // Sinon, c'est un mot simple, on passe à l'état intermédiaire Emot
            *w++ = c;      // On ajoute le caractère courant au mot en cours de lecture
            continue;       // On continue la boucle sans exécuter la suite
        }
        case Spp:
            // Si on est dans l'état intermédiaire Spp, on vérifie si le caractère courant est un second '>'
            if (c == '>')
                return (SPP);   // Si c'est le cas, cela signifie que l'on a lu l'opérateur de redirection '>>', on retourne le type SPP
            ungetc(c, stdin);   // Sinon, on remet le caractère courant dans l'entrée standard et on retourne le type SUP (opérateur de redirection de sortie)
            return (SUP);
        case Equote:
            // Si on est dans l'état intermédiaire Equote, on est en train de lire une chaîne de caractères entre guillemets
            switch (c)
            {
            case '\':
            *w++ = c; // Si le caractère courant est un '', on l'ajoute au mot en cours de lecture
            continue; // On continue la boucle sans exécuter la suite
            case '"':
            *w = '\0'; // Si c'est un second guillemet, cela signifie que la chaîne de caractères est terminée, on ajoute un caractère de fin de chaîne
            return (MOT); // On retourne le type MOT
            default:
            *w++ = c; // Sinon, on ajoute le caractère courant au mot en cours de lecture
            continue; // On continue la boucle sans exécuter la suite
            }
            case Emot:
            // Si on est dans l'état intermédiaire Emot, on est en train de lire un mot simple
            switch (c)
            {
            case '|':
            case '<':
            case '>':
            case ' ':
            case '\t':
            case '\n':
            // Si on lit un caractère spécial (opérateur de redirection, espace, tabulation ou fin de ligne), cela signifie que le mot est terminé
            ungetc(c, stdin); // On remet le caract
                ère courant dans l'entrée standard
*w = '\0'; // On ajoute un caractère de fin de chaîne au mot en cours de lecture
return (MOT); // On retourne le type MOT
default:
*w++ = c; // Sinon, on ajoute le caractère courant au mot en cours de lecture
continue; // On continue la boucle sans exécuter la suite
}
}
}
// Si on sort de la boucle while, cela signifie qu'on a atteint la fin du flux d'entrée, on retourne le type FIN
return (FIN);
}
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
