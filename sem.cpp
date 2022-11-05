#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <map>
using namespace std;

#define TOT_PATIENTS 50
#define TOT_VACS 70

map<pthread_t, int> pat2vac;

int vac1[3] = {0, 0, 0};
int nvacs = 3;

sem_t vac2[3];
sem_t full;
sem_t mux;

void *producer(void *n){
    int in = 0;
    for (int i = 0; i < (long)n; i++){
        /* Produz vacinas */
        vac1[in]++; // PRIMEIRA DOSE

        printf("vaccine %d - produced = %d\n", in, vac1[in]);

        sem_post(&vac2[in]); // SEGUNDA DOSE
        sem_post(&full); // se eu produzi alguma vacina...

        in = (in + 1) % nvacs;
    }
    pthread_exit(NULL);
}

void *patient(void *n){

    for (int i = 0; i < (long)n; i++){
        /* consome vacinas */
        int out;
        if (pat2vac[pthread_self()] == 0){ // Se não tomou nenhuma dose...

            sem_wait(&full); // Checa se tem vacina em estoque...
            sem_wait(&mux); // ENTRA REGIÃO CRÍTICA

            // procura por uma vacina produzida...
            for (int j = 0; j < nvacs; j++){
                if (vac1[j] != 0)
                    out = j;
            }
            pat2vac[pthread_self()] = out; // marca que paciente x tomou vacina y
            vac1[out]--;                   // consome a dose out

            printf("patient |%d| - 1st dose - vaccine %d\n", (int)(pthread_self()), out);

            sem_post(&mux); // FIM REGIÃO CRÍTICA

        } else{ // se ja tomou uma dose...

            int patient = pat2vac[pthread_self()];
            sem_wait(&vac2[patient]);
            
            printf("patient |%d| - 2nd dose - vaccine %d\n", (int)(pthread_self()), out);
        }
    }
    pthread_exit(NULL);
}

int main(){
    
    // semaforos para cada vacina 2a dose
    sem_t v1, v2, v3;
    sem_init(&v1, 0, 0);
    sem_init(&v2, 0, 0);
    sem_init(&v3, 0, 0);
    vac2[0] = v1;
    vac2[1] = v2;
    vac2[2] = v3;

    // semaforo RC
    sem_init(&mux, 0, 1);
    
    // semaforo vacinas total 1a dose
    sem_init(&full, 0, 0);

    /* cria vacinas */
    pthread_t v;
    pthread_create(&v, NULL, producer, (void*)TOT_VACS);
    pthread_join(v, NULL);
    
    cout << "\n";

    /* cria pacientes */
    pthread_t p[TOT_PATIENTS];
    for(int i=0; i<TOT_PATIENTS; i++){
        p[i]=pthread_t();
        pthread_create(&p[i], NULL, patient, (void*)2);
    }

    for(int i=0; i<TOT_PATIENTS; i++)
        pthread_join(p[i], NULL);

    /* desaloca */
    for(int i=0; i<nvacs; i++)
        sem_destroy(&vac2[i]);
    sem_destroy(&full);
    sem_destroy(&mux);

    return 0;
}