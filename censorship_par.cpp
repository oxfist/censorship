#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <queue>
#include <pthread.h>
#include <gmp.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include "time2.h"

#define avg(x, y) ((x - y) / 2)

/* Par de enteros para almacenar límite inicial y final de un subgrupo
 * de usuarios a partir del grupo inicial */
typedef std::pair<int, int> Limites;

pthread_mutex_t mutex_thread_pool, mutex_grupos, mutex_identificados, mutex_claves;
std::queue<Limites> grupos;
std::queue<unsigned long> thread_pool;
mpz_t usuarios;

/* En un comienzo no hay adversarios identificados */
unsigned int identificados, claves;

void usage(char **argv);
void *check_groups(void *thread_id);
void init_thread_pool(int m);
void asignar_adversarios(mpz_t usuarios, const int &n, const int &k);
unsigned long mix(unsigned long a, unsigned long b, unsigned long c);

int main(int argc, char *argv[]) {
    if (argc >= 4) {
        unsigned int n, p, k, tam_grupo, resto, inicio;
        unsigned long thread_activo;
        unsigned long seed = mix(clock(), time(NULL), getpid());
        time_586 start, stop;

        /* Argumentos del programa.
         * n = cantidad de usuarios.
         * k = cantidad de adversarios
         * p = cantidad de threads
         */
        n = atoi(argv[1]);
        k = atoi(argv[2]);
        p = atoi(argv[3]);

        pthread_t threads[p];
        init_thread_pool(p);
        pthread_attr_t attr;

        pthread_mutex_init(&mutex_thread_pool, NULL);
        pthread_mutex_init(&mutex_grupos, NULL);
        pthread_mutex_init(&mutex_identificados, NULL);
        pthread_mutex_init(&mutex_claves, NULL);
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        /* Bitset para almacenar usuarios, n bits */
        mpz_init2(usuarios, n);

        srand(seed);
        time2(start);

        /* Inicialmente creamos k grupos */
        tam_grupo = floor(n / k);
        resto = n % k;
        inicio = 0;
        if (resto > 0) {
            /* Creamos 'k-resto' grupos de tamaño tam_grupo */
            for (unsigned int i = 0; i < k-resto; i++) {
                grupos.push(std::make_pair(inicio, inicio + tam_grupo - 1));
                inicio += tam_grupo;
            }
            /* Creamos 'resto' grupos de tamaño tam_grupo+1 */
            for (unsigned int i = 0; i < resto; i++) {
                grupos.push(std::make_pair(inicio, inicio + tam_grupo));
                inicio += tam_grupo+1;
            }
        } else {
            /* Si la división es exacta, simplemente creamos 'k' grupos
             * de tamaño tam_grupo */
            for (unsigned int i = 0; i < k; i++) {
                grupos.push(std::make_pair(inicio, inicio + tam_grupo - 1));
                inicio += tam_grupo;
            }
        }
        asignar_adversarios(usuarios, n, k);

        /* En un comienzo hay k claves distribuidas (una para cada grupo) */
        claves = k;

        /* Iteramos mientras no hayamos identificado todos los adversarios */
        pthread_mutex_lock(&mutex_identificados);
        while (identificados < k) {
            pthread_mutex_unlock(&mutex_identificados);

            pthread_mutex_lock(&mutex_grupos);
            /* En este instante de tiempo puede que no haya grupos pendientes
             * por procesar, lo que no significa, necesariamente, que todos los
             * adversarios fueron identificados. De ser así, se va a seguir
             * iterando hasta que se agregue un nuevo grupo a procesar a la queue.
             */
            while (grupos.size() > 0) {
                pthread_mutex_unlock(&mutex_grupos);

                pthread_mutex_lock(&mutex_thread_pool);
                //std::cout << "thread_pool: " << thread_pool.size() << std::endl;
                /* Si bien puede haber grupos pendientes por procesar, puede
                 * ocurrir que en este instante de tiempo todos los threads
                 * estén ocupados. De ser así, se sigue iterando hasta que un
                 * thread se desocupe.
                 */
                if (thread_pool.size() > 0) {
                    /* Se guarda el id del primer thread que está desocupado. */
                    thread_activo = thread_pool.front();
                    thread_pool.pop();
                } else {
                    pthread_mutex_unlock(&mutex_thread_pool);
                    /* Si no hay threads disponibles, se espera hasta que uno lo esté. */
                    while (true) {
                        pthread_mutex_lock(&mutex_thread_pool);
                        if (thread_pool.size() > 0) {
                            /* Se guarda el id del primer thread que está desocupado. */
                            thread_activo = thread_pool.front();
                            thread_pool.pop();
                            break;
                        }
                        pthread_mutex_unlock(&mutex_thread_pool);
                    }
                }
                /* Se envía trabajo al thread desocupado. */
                pthread_create(&threads[thread_activo], &attr, check_groups, (void *) thread_activo);
                pthread_mutex_unlock(&mutex_thread_pool);
                pthread_mutex_lock(&mutex_grupos);
            }
            pthread_mutex_unlock(&mutex_grupos);

            pthread_mutex_lock(&mutex_identificados);
        }
        pthread_mutex_unlock(&mutex_identificados);
        time2(stop);
        while (true) {
            pthread_mutex_lock(&mutex_thread_pool);
            if (thread_pool.size() == p) {
                break;
            }
            pthread_mutex_unlock(&mutex_thread_pool);
        }
        mpz_clear(usuarios);
        /* Output: claves encontradas y tiempo de ejecución */
        printf("%d %d %.20fs\n", k, claves, time_diff(stop, start) / 1000000.0);
    } else {
        usage(argv);
    }
    pthread_mutex_destroy(&mutex_thread_pool);
    pthread_mutex_destroy(&mutex_grupos);
    pthread_mutex_destroy(&mutex_identificados);
    pthread_mutex_destroy(&mutex_claves);
    pthread_exit(EXIT_SUCCESS);
}

void *check_groups(void *thread_id) {
    int limite_inferior, limite_superior;
    bool adversario = false;
    Limites limites_grupo_actual;

    pthread_mutex_lock(&mutex_grupos);
    /* Revisamos el grupo para ver si hay algún adversario.
     * Recordar que una restricción de nuestra implementación es
     * que el adversario compromete la clave inmediatamente, lo que
     * significa que al encontrar un adversario podemos considerar
     * que la clave está comprometida y debemos dividir el grupo. */
    if (grupos.size() == 0) {
        pthread_mutex_unlock(&mutex_grupos);
        pthread_mutex_lock(&mutex_thread_pool);
        /* El thread se encola a sí mismo, indicando que está disponible. */
        thread_pool.push((unsigned long) thread_id);
        pthread_mutex_unlock(&mutex_thread_pool);

        pthread_exit(EXIT_SUCCESS);
    }
    limites_grupo_actual = grupos.front();
    grupos.pop();
    pthread_mutex_unlock(&mutex_grupos);

    limite_inferior = limites_grupo_actual.first;
    limite_superior = limites_grupo_actual.second;
    /* Si el grupo consiste en un solo usuario, significa que hemos
     * logrado identificar al adversario */
    if (limite_inferior == limite_superior) {
        if (mpz_tstbit(usuarios, limite_inferior)) {
            pthread_mutex_lock(&mutex_identificados);
            identificados++;
            pthread_mutex_unlock(&mutex_identificados);
        }
    } else {
        /* Buscamos el adversario en el bitset, entre los rangos
         * limite_inferior y limite_superior. */
        int index = mpz_scan1(usuarios, limite_inferior);
        if (index <= limite_superior && index >= 0) {
            adversario = true;
        }

        /* Adversario encontrado, dividir el grupo en dos subgrupos.
         * Revocamos clave antigua y asignamos dos nuevas */
        if (adversario) {
            int avg_limites = avg(limite_superior, limite_inferior);
            Limites nuevos_limites = std::make_pair(limite_inferior, limite_inferior + avg_limites);

            pthread_mutex_lock(&mutex_grupos);
            grupos.push(nuevos_limites);
            nuevos_limites = std::make_pair(nuevos_limites.second + 1, limite_superior);
            grupos.push(nuevos_limites);
            pthread_mutex_unlock(&mutex_grupos);

            pthread_mutex_lock(&mutex_claves);
            claves += 1;
            pthread_mutex_unlock(&mutex_claves);
        }
    }
    pthread_mutex_lock(&mutex_thread_pool);
    /* El thread se encola a sí mismo, indicando que está disponible. */
    thread_pool.push((unsigned long) thread_id);
    pthread_mutex_unlock(&mutex_thread_pool);

    pthread_exit(EXIT_SUCCESS);
}

/* Se encolan todos los identificadores de los threads a utilizar, lo que
 * servirá como una pool de threads. */
void init_thread_pool(int p) {
    for (int i = 0; i < p; i++) {
        thread_pool.push(i);
    }
}

/* Asignar posiciones iniciales para los adversarios en el bitset de usuarios.
 * Las posiciones se asignan aleatoriamente dentro de todo el conjunto */
void asignar_adversarios(mpz_t usuarios, const int &n, const int &k) {
    for (int i = 0; i < k; i++) {
        while (true) {
            int adv_index = rand() % n;
            if (!mpz_tstbit(usuarios, adv_index)) {
                mpz_setbit(usuarios, adv_index);
                break;
            }
        }
    }
}

void usage(char **argv) {
    printf("\nUsage: %s n k\n", argv[0]);
    printf("    n: Cantidad de usuarios\n");
    printf("    k: Cantidad de adversarios. k << n\n");
    printf("    p: Cantidad de threads a utlizar.\n");
}

/* Función creada por Robert Jenkins. Genera un seed único para cada
 * ejecución, independiente que éstas sean dentro del mismo ciclo
 * de reloj. En este caso, permite obtener distintas asignaciones
 * (aleatorias) de adversarios dentro del grupo de usuarios. */
unsigned long mix(unsigned long a, unsigned long b, unsigned long c) {
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}
