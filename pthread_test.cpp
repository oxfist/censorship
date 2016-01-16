#include <pthread.h>
#include <iostream>
#include <cstdlib>

#define NUM_THREADS 5

void *PrintHello(void *threadid) {
    long tid;
    tid = (long) threadid;
    std::cout << "Hello, world! This is thread #" << tid << "!" << std::endl;
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int rc;
    long t;
    for (t = 0; t < NUM_THREADS; t++) {
        std::cout << "In main creating thread #" << t << std::endl;
        rc = pthread_create(&threads[t], NULL, PrintHello, (void *) t);
        if (rc) {
            std::cout << "ERROR; return code from pthread_create() is " << rc << std::endl;
            return(EXIT_FAILURE);
        }
    }

    pthread_exit(NULL);
}
