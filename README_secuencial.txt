Para compilar la versión secuencial "censorship.cpp" es necesario ejecutar el siguiente comando:

g++ censorship.cpp -o censorship -Wall --std=c++0x -O2 -lgmp

El argumento -lgmp se utiliza para indicar que se compilará con la librería GMP.
En caso de no poseer la librería GMP es necesario descargarla de https://gmplib.org/; extraerla y ejecutar los siguientes comandos:

./configure
make
make check
sudo make install

Una vez todo configurado, es posible ejecutar el código indicando dos argumentos, n y k. Por ejemplo:

./censorship 100 10
