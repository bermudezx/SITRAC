CXX = g++
CXXFLAGS = -std=c++98 -Wall -Wextra -pedantic -O2
SRC = src/main.cpp src/util.cpp src/ciudades.cpp src/pasajeros.cpp \
      src/rutas.cpp src/cola.cpp src/historial.cpp src/busquedas.cpp \
      src/reportes.cpp src/archivos.cpp
TARGET = sitrac

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe src/*.o *.o

test: $(TARGET)
	sh pruebas/probar.sh
