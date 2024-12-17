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

struct sembuf Pmutex = {mutex, -1, 0}; /* P(mutex) */
struct sembuf Vmutex = {mutex, 1, 0}; /* V(mutex) */

key_t cle; /* Cle du segment */
int sem;
int seg_v1;
int *v1_var;
int seg_Acces_v1;
info *Acces_v1;

int main() {
    /* Récuperation des sémaphores */
    cle = ftok("./pgme_principal.c", 1);
    if (cle == -1){ printf("Erreur dans la création de la clé!\n"); exit(1); }

    sem = semget(cle, n+1, IPC_CREAT | 0666);
    if(sem == -1){ printf("Erreur de création des semaphore!\n"); exit(2); }
    
    /* Récuperation du segment de mémoire partagée de v1 et cpt */
    cle = ftok("./pgme_principal.c", 2);
    if (cle == -1){ printf("Erreur dans la création de la clé!\n"); exit(3); }

    seg_v1 = shmget(cle, 2*sizeof(int), IPC_CREAT | 0666);
    if(seg_v1 == -1){ printf("Erreur dans la création du segment v1_var!\n"); exit(4); }

    /* Attachement */
    v1_var = (int*) shmat(seg_v1, 0, 0);
    if (v1_var == NULL){ printf("Erreur lors de l'attachement de v1_var!\n"); exit(5); }

    /* Récuperation du segment de mémoire partagée de Acces_v1 */
    cle = ftok("./pgme_principal.c", 3);
    if (cle == -1){ printf("Erreur dans la création de la clé!\n"); exit(6); }

    seg_Acces_v1 = shmget(cle, n*sizeof(info), IPC_CREAT | 0666);
    if(seg_Acces_v1 == -1){ printf("Erreur dans la création du segment Acces_v1!\n"); exit(7); }

    /* Attachement */
    Acces_v1 = (info*) shmat(seg_Acces_v1, 0, 0);
    if (Acces_v1 == NULL){ printf("Erreur lors de l'attachement de Acces_v1!\n"); exit(8); }

    //----------------------------------------------------

    /* Début du processus(i) */
    int j, i;
    info element;

    semop(sem, &Pmutex, 1);
    j = v1_var[1];
    (v1_var[1])++;
    printf("Processus N: %d de PID = %d\n", j, getpid());
    semop(sem, &Vmutex, 1);
    struct sembuf Pt = {j, -1, 0}; /* P(t[j]) */
    semop(sem, &Pt, 1);
    v1_var[0] += j;
    element.pid = getpid();
    element.rang = j;
    element.val = v1_var[0];
    Acces_v1[j] = element;
    struct sembuf Vt = {(j-1+n) % n, 1, 0}; /* V(t[(j-1+n) mod n]) */
    semop(sem, &Vt, 1);
    if(j == 0){
        printf("Les resultats d'acces a la variable v1:\n");
        for (i = n-1; i >= 0 ; i--){
            printf("Acces N: %d par le processus de PID = %d de rang = %d, v1 = %d\n", n-i, Acces_v1[i].pid, Acces_v1[i].rang, Acces_v1[i].val); 
        }
    }
    /* Suite i*/

    //----------------------------------------------------

    /* Détachement des segments de mémoire partagée */
    shmdt(v1_var);
    shmdt(Acces_v1);

    return 0;
}