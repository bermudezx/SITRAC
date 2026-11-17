CXX = g++
CXXFLAGS = -std=c++98 -Wall -Wextra -pedantic -O2
CORE = src/util.cpp src/ciudades.cpp src/pasajeros.cpp src/rutas.cpp \
       src/cola.cpp src/historial.cpp src/busquedas.cpp src/reportes.cpp \
       src/archivos.cpp

.PHONY: test win clean

# La aplicacion de escritorio se abre con SITRAC.dev en Dev-C++.
# Este objetivo solo comprueba la logica en Linux.
test:
	$(CXX) $(CXXFLAGS) -Werror pruebas/logica_main.cpp $(CORE) -o pruebas/logica
	./pruebas/logica

# Compilacion cruzada opcional, si MinGW esta instalado.
win:
	x86_64-w64-mingw32-g++ -std=c++98 -Wall -Wextra -O2 -mwindows \
		src/main.cpp $(CORE) -o SITRAC.exe -lcomctl32 -lgdi32 -static-libgcc

clean:
	rm -f pruebas/logica SITRAC.exe src/*.o *.o
