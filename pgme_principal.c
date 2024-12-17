#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define n 6 /* Nombre de processus */

typedef struct { int pid; int rang; int val; } info;

#define mutex n /* Numéro du sémaphore mutex */

key_t cle; /* Cle du segment */
int sem;
int seg_v1;
int *v1_var;
int seg_Acces_v1;

int main() {
    /* Création et initialisation des sémaphores */
    cle = ftok("./pgme_principal.c", 1);
    if (cle == -1){ printf("Erreur dans la création de la clé!\n"); exit(1); }

    sem = semget(cle, n+1, IPC_CREAT | 0666);
    if(sem == -1){ printf("Erreur de création des semaphores!\n"); exit(2); }

    for(int i = 0; i < n-1; i++){
        semctl(sem, i, SETVAL, 0); /* t[i] init (0) */
    }
    semctl(sem, n-1, SETVAL, 1); /* t[n-1] init (1) */
    semctl(sem, mutex, SETVAL, 1); /* mutex init (1) */
    
    /* Création du segment de mémoire partagée de v1 et cpt et initialisation */
    cle = ftok("./pgme_principal.c", 2);
    if (cle == -1){ printf("Erreur dans la création de la clé!\n"); exit(3); }

    seg_v1 = shmget(cle, 2*sizeof(int), IPC_CREAT | 0666);
    if(seg_v1 == -1){ printf("Erreur dans la création du segment v1_var!\n"); exit(4); }

    /* Attachement */
    v1_var = (int*) shmat(seg_v1, 0, 0);
    if (v1_var == NULL){ printf("Erreur lors de l'attachement de v1_var!\n"); exit(5); }

    v1_var[0] = 0; /* v1 init (0) */
    v1_var[1] = 0; /* cpt init (0) */

    /* Détachement du segement de v1_var */
    shmdt(v1_var);

    /* Création du segment de mémoire partagée de Acces_v1 */
    cle = ftok("./pgme_principal.c", 3);
    if (cle == -1){ printf("Erreur dans la création de la clé!\n"); exit(6); }

    seg_Acces_v1 = shmget(cle, n*sizeof(info), IPC_CREAT | 0666);
    if(seg_Acces_v1 == -1){ printf("Erreur dans la création du segment Acces_v1!\n"); exit(7); }

    //----------------------------------------------------

    /* Début du programme principal */
    int p, err;

    for(int i = 0; i < n; i++){
        p = fork();
        if(p == -1){ printf("Erreur lors de la création du processus fils %d.\n", i); exit(8); }

        if(p == 0){ // Le fils i
            err = execlp("./pgme_processus", "./pgme_processus", NULL);
            printf("Erreur de code %d lors de l'exécution du processus fils %d.\n", err, i); exit(9);
        }
    }
    /* Attente de la fin des processus fils */
    while(wait(NULL) != -1);

    //----------------------------------------------------

    /* Suppression des sémaphores et segments */
    semctl(sem, IPC_RMID, 0);
    shmctl(seg_v1, IPC_RMID, 0);
    shmctl(seg_Acces_v1, IPC_RMID, 0);
}