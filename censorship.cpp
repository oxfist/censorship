#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <gmp.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include "time2.h"

#define avg(x, y) ((x - y) / 2)

using namespace std;

/* Par de enteros para almacenar límite inicial y final de un subgrupo
 * de usuarios a partir del grupo inicial */
typedef pair<int, int> pii;

void usage(char **argv);
void activate_adversaries(mpz_t users, const int &n, const int &k);
unsigned long mix(unsigned long a, unsigned long b, unsigned long c);

int main(int argc, char *argv[]) {
	if (argc >= 3) {
        bool adversario;
        int i, n, k, identificados, claves, tam_grupo, resto, inicio;
        pii limite, nuevo_limite;
        queue<pii> grupos;
        mpz_t users;
        unsigned long seed = mix(clock(), time(NULL), getpid());
        time_586 start, stop;

		/* Argumentos del programa.
		 * n = cantidad de usuarios.
		 * k = cantidad de adversarios
		 */
		n = atoi(argv[1]);
		k = atoi(argv[2]);

		/* Bitset para almacenar usuarios, n bits */
		mpz_init2(users, n);

        srand(seed);
		time2(start);

		/* Inicialmente creamos k grupos */
		tam_grupo = floor(n/k);
		resto = n%k;
        inicio = 0;
		if (resto > 0) {
			/* Creamos 'k-resto' grupos de tamaño tam_grupo */
			for (i = 0; i < k-resto; i++) {
				grupos.push(make_pair(inicio, inicio + tam_grupo-1));
				inicio += tam_grupo;
			}
			/* Creamos 'resto' grupos de tamaño tam_grupo+1 */
			for (i = 0; i < resto; i++) {
				grupos.push(make_pair(inicio, inicio + tam_grupo));
				inicio += tam_grupo+1;
			}
		} else {
            /* Si la división es exacta, simplemente creamos 'k' grupos
             * de tamaño tam_grupo */
            for (i = 0; i < k; i++) {
                grupos.push(make_pair(inicio, inicio + tam_grupo-1));
                inicio += tam_grupo;
            }
        }
        activate_adversaries(users, n, k);
		
		/* En un comienzo no hay adversarios identificados y hay k claves
		 * distribuidas (una para cada grupo) */
		identificados = 0;
		claves = k;
		
		/* Iteramos mientras no hayamos identificado todos los adversarios */
		while (identificados < k) {
			/* Revisamos el grupo para ver si hay algún adversario.
			 * Recordar que una restricción de nuestra implementación es
			 * que el adversario compromete la clave inmediatamente, lo que
			 * significa que al encontrar un adversario podemos considerar
			 * que la clave está comprometida y debemos dividir el grupo. */
			limite = grupos.front();
			grupos.pop();
			adversario = false;
			/* Si el grupo consiste en un solo usuario, significa que hemos
			 * logrado identificar al adversario */
			if (limite.first == limite.second) {
				if (mpz_tstbit(users, limite.first))
					identificados++;
			} else {
                /* Buscamos el adversario en el bitset, en los rangos limite.first
                 * y limite.second */
				long index = mpz_scan1(users, limite.first);
				if (index <= limite.second && index >= 0) {
                    adversario = true;
                }

				/* Adversario encontrado, dividir el grupo en dos subgrupos.
				 * Revocamos clave antigua y asignamos dos nuevas */
				if (adversario) {
					nuevo_limite = make_pair(limite.first, limite.first + avg(limite.second, limite.first));
					grupos.push(nuevo_limite);
					nuevo_limite = make_pair(nuevo_limite.second + 1, limite.second);
					grupos.push(nuevo_limite);
					claves += 1;
				}
			}
		}
		time2(stop);
		mpz_clear(users);
        /* Output: claves encontradas y tiempo de ejecución */
        printf("%d %.20fs\n", claves, time_diff(stop, start) / 1000000.0);
	} else {
	    usage(argv);
	}	
	return 0;
}

/* Asignar posiciones iniciales para los adversarios en el bitset de usuarios.
 * Las posiciones se asignan aleatoriamente dentro de todo el conjunto */
void activate_adversaries(mpz_t users, const int &n, const int &k) {
    for (int i = 0; i < k; i++) {
        while (true) {
            int adv_index = rand() % n;
            if (!mpz_tstbit(users, adv_index)) {
                mpz_setbit(users, adv_index);
                break;
            }
        }
    }
}

void usage(char **argv) {
    printf("\nUsage: %s n k\n", argv[0]);
    printf("	n: Cantidad de usuarios\n");
    printf("	k: Cantidad de adversarios. k << n\n\n");
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
