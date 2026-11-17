#!/bin/sh
# Prueba de consola de SITRAC. Compila con C++98 y recorre los menus.
set -eu

RAIZ=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
cd "$RAIZ"

g++ -std=c++98 -Wall -Wextra -pedantic -Werror -O2 -o "$RAIZ/sitrac" \
    src/main.cpp src/util.cpp src/ciudades.cpp src/pasajeros.cpp \
    src/rutas.cpp src/cola.cpp src/historial.cpp src/busquedas.cpp \
    src/reportes.cpp src/archivos.cpp

TMP="$RAIZ/pruebas/tmp"
rm -rf "$TMP"
mkdir -p "$TMP/ejemplo" "$TMP/vacio"

cp "$RAIZ/sitrac" "$TMP/ejemplo/sitrac"
cp "$RAIZ/sitrac" "$TMP/vacio/sitrac"
cp -a "$RAIZ/datos" "$TMP/ejemplo/datos"

exigir() {
    archivo=$1
    texto=$2
    if ! grep -F -q -- "$texto" "$archivo"; then
        echo "FALTA: $texto"
        echo "---- $archivo ----"
        cat "$archivo"
        exit 1
    fi
}

prohibir() {
    archivo=$1
    texto=$2
    if grep -F -q -- "$texto" "$archivo"; then
        echo "NO DEBERIA APARECER: $texto"
        echo "---- $archivo ----"
        cat "$archivo"
        exit 1
    fi
}

echo "1. Datos de ejemplo"
(cd "$TMP/ejemplo" && ./sitrac < "$RAIZ/pruebas/entrada_ejemplo.txt" > "$TMP/ejemplo/salida.txt")
EJ="$TMP/ejemplo/salida.txt"
exigir "$EJ" "Informacion cargada correctamente desde la carpeta datos."
exigir "$EJ" "Ciudades: 6 | Rutas: 9 | Pasajeros: 3 | Pendientes: 2 | Historial: 1"
exigir "$EJ" "Bogota"
exigir "$EJ" "Bucaramanga"
exigir "$EJ" "Ana Perez"
exigir "$EJ" "Luis Gomez"
exigir "$EJ" "Marta Ruiz"
exigir "$EJ" "Bogota -> Cali"
exigir "$EJ" "461.00 km"
exigir "$EJ" "Resultado: ATENDIDA"
exigir "$EJ" "Solicitud #11 agregada al final de la cola."
prohibir "$EJ" "Se omitio"
prohibir "$EJ" "Avisos durante la carga"

echo "2. Eliminar ciudad y sus conexiones"
(cd "$TMP/vacio" && ./sitrac < "$RAIZ/pruebas/entrada_eliminar.txt" > "$TMP/vacio/eliminar.txt")
EL="$TMP/vacio/eliminar.txt"
exigir "$EL" "Conexion registrada correctamente."
exigir "$EL" "- Otra: 25.00 km"
exigir "$EL" "Ciudad y sus conexiones eliminadas correctamente."
exigir "$EL" "Esta ciudad no tiene conexiones."
exigir "$EL" "1. Otra"
prohibir "$EL" "1. Solo"
prohibir "$EL" "2. Otra"

echo "3. Flujo completo de menus"
(cd "$TMP/vacio" && ./sitrac < "$RAIZ/pruebas/entrada_flujo.txt" > "$TMP/vacio/flujo.txt")
FL="$TMP/vacio/flujo.txt"
exigir "$FL" "Ciudad registrada correctamente."
exigir "$FL" "Error: la ciudad ya existe."
exigir "$FL" "Conexion registrada correctamente."
exigir "$FL" "Esta ruta ya existe."
exigir "$FL" "Ruta registrada correctamente."
exigir "$FL" "Regreso: Ruta registrada correctamente."
exigir "$FL" "Camino mas corto: Bogota -> Pereira -> Cali"
exigir "$FL" "Distancia total: 500.00 km"
exigir "$FL" "Ruta eliminada correctamente."
exigir "$FL" "Error: el documento ya esta registrado."
exigir "$FL" "Nombre: Ana Perez"
exigir "$FL" "Pasajero actualizado correctamente."
exigir "$FL" "Pasajero eliminado correctamente."
exigir "$FL" "El pasajero no esta registrado."
exigir "$FL" "Solicitud #1 agregada al final de la cola."
exigir "$FL" "Resultado: ATENDIDA"
exigir "$FL" "Resultado: RECHAZADA"
exigir "$FL" "Detalle: Bogota -> Pereira -> Cali"
exigir "$FL" "Camino encontrado: Pereira -> Bogota -> Medellin"
exigir "$FL" "Distancia total: 715.00 km"
exigir "$FL" "Ciudad con mas conexiones de salida: Pereira"
exigir "$FL" "Ruta directa mas corta: Pereira -> Cali (200.00 km)"
exigir "$FL" "Ruta directa mas larga: Bogota -> Medellin (415.00 km)"
exigir "$FL" "Menores de 18: 0"
exigir "$FL" "Adultos: 2"
exigir "$FL" "Atendidas: 2"
exigir "$FL" "Rechazadas: 1"
exigir "$FL" "Ciudades: 5"
exigir "$FL" "Rutas dirigidas: 4"
exigir "$FL" "Distancia acumulada de la red: 1215.00 km"
exigir "$FL" "Informacion guardada correctamente en la carpeta datos."
exigir "$FL" "Debe ingresar un numero."
exigir "$FL" "Gracias por utilizar SITRAC."

echo "4. Cargar lo guardado en una nueva ejecucion"
(cd "$TMP/vacio" && ./sitrac < "$RAIZ/pruebas/entrada_carga.txt" > "$TMP/vacio/carga.txt")
CA="$TMP/vacio/carga.txt"
exigir "$CA" "Informacion cargada correctamente desde la carpeta datos."
exigir "$CA" "Ciudades: 5 | Rutas: 4 | Pasajeros: 2 | Pendientes: 0 | Historial: 3"
exigir "$CA" "Luis Gomez Actualizado"
exigir "$CA" "Cartagena"
exigir "$CA" "Resultado: ATENDIDA"
exigir "$CA" "Resultado: RECHAZADA"
prohibir "$CA" "Resultado: CANCELADA"
prohibir "$CA" "Se omitio"
prohibir "$CA" "Avisos durante la carga"
exigir "$CA" "Ciudades: 5"
exigir "$CA" "Registros de historial: 3"

if command -v valgrind >/dev/null 2>&1; then
    echo "5. Memoria"
    (cd "$TMP/vacio" && valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=definite \
        ./sitrac < "$RAIZ/pruebas/entrada_flujo.txt" > "$TMP/vacio/valgrind_out.txt" 2> "$TMP/vacio/valgrind.txt")
    if grep -E "definitely lost: [1-9]|ERROR SUMMARY: [1-9]" "$TMP/vacio/valgrind.txt" >/dev/null; then
        echo "Fuga o error de memoria"
        cat "$TMP/vacio/valgrind.txt"
        exit 1
    fi
fi

echo "Pruebas superadas."
