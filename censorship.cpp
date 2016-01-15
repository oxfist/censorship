#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>
#include <gmp.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include "time2.h"

#define avg(x, y) ((x - y) / 2)

using namespace std;

typedef pair<int, int> pii;

void usage(char **argv);
unsigned long mix(unsigned long a, unsigned long b, unsigned long c);

int main(int argc, char *argv[])
{
	if (argc >= 3)
	{
        bool adversario;
        int i, j, n, k, identificados, claves, tam_grupo, resto, inicio;
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


		/* Vector para almacenar usuarios, n bits */
		//vector<bool> users(n);
		mpz_init2(users, n);

        srand(seed);
		
		time2(start);
		/* Inicialmente creamos k grupos */
		tam_grupo = floor(n/k);
		resto = n%k;
		if (resto > 0)
		{
			inicio = 0;
			/* k-resto grupos */
			for (i = 0; i < k-resto; i++)
			{
				grupos.push(make_pair(inicio, inicio + tam_grupo-1));
				/* Activar adversario en posición aleatoria */
				j = rand()%tam_grupo;
				mpz_setbit(users, inicio + j);
				inicio += tam_grupo;
			}
			/* resto grupos */
			for (i = 0; i < resto; i++)
			{
				grupos.push(make_pair(inicio, inicio + tam_grupo));
				/* Activar adversario en posición aleatoria */
				j = rand()%tam_grupo;
				mpz_setbit(users, inicio + j);
				inicio += tam_grupo+1;
			}
		}
		
		/* En un comienzo no hay adversarios identificados y hay k claves
		 * distribuidas (una para cada grupo) */
		identificados = 0;
		claves = k;
		
		/* Iteramos mientras no hayamos identificado todos los adversarios */
		while (identificados < k)
		{
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
			if (limite.first == limite.second)
			{
				if (mpz_tstbit(users, limite.first))
					identificados++;
			}
			else
			{
			    // TODO: cambiar por mpz_scan1()
				/*for (i = limite.first; i <= limite.second; i++)
				{
					if (mpz_tstbit(users, i))
					{
						adversario = true;
						break;
                    }
				}*/

				long index = mpz_scan1(users, limite.first);
				if (index <= limite.second && index > 0) {
                    adversario = true;
                }

				/* Adversario encontrado, dividir el grupo en dos subgrupos
				 * y asignar dos nuevas claves */
				if (adversario)
				{
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
		
        printf("%d %.20fs\n", claves, time_diff(stop, start) / 1000000.0);
	} else {
	    usage(argv);
	}	
	return 0;
}

void usage(char **argv) {
    printf("\nUsage: %s n k\n", argv[0]);
    printf("	n: Cantidad de usuarios\n");
    printf("	k: Cantidad de adversarios. k << n\n\n");
}

/* Función creada por Robert Jenkins. */
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
