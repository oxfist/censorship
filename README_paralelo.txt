Para compilar la versión paralela "censorship_par.cpp" es necesario ejecutar el siguiente comando:

g++ censorship_par.cpp -o censorship_par -Wall --std=c++0x -O2 -lgmp -pthread

El argumento -pthread indica que se compilará con la librería pthread. El argumento -lgmp se utiliza
para indicar que se compilará con la librería GMP. En caso de no poseer la librería GMP es necesario
descargarla de https://gmplib.org/; extraerla y ejecutar los siguientes comandos:

./configure
make
make check
sudo make install

Una vez todo configurado, es posible ejecutar el código indicando tres argumentos, n, k y p. Por ejemplo:

./censorship_par 100 10 4
