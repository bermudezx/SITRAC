# SITRAC

Sistema Inteligente de Transporte y Gestión de Rutas.

Proyecto académico en C++ para la Universidad Santo Tomás. Modela una red de transporte por consola: ciudades, rutas, pasajeros, cola de solicitudes, historial, búsquedas, reportes y persistencia en archivos de texto.

Repositorio: [https://github.com/bermudezx/SITRAC](https://github.com/bermudezx/SITRAC)

## Descripción

SITRAC administra una red de ciudades conectadas por rutas dirigidas. Cada ciudad puede tener varias salidas y cada ruta guarda la distancia en kilómetros. Sobre esa red se registran pasajeros y una cola de solicitudes de viaje. Al atender una solicitud, el sistema busca el camino más corto y deja el resultado en el historial. Toda la información se puede guardar y volver a cargar.

## Objetivo general

Ofrecer una aplicación de consola, compilable en Dev-C++ con C++ estándar, que integre la gestión de una red de transporte usando estructuras de datos implementadas manualmente: listas enlazadas, un grafo por listas de adyacencia, una cola y el algoritmo de Dijkstra.

## Funcionalidades

El menú principal conecta estos módulos:

| Opción | Módulo | Qué permite |
| --- | --- | --- |
| 1 | Ciudades | Registrar, listar y eliminar ciudades. Agregar y consultar conexiones. |
| 2 | Rutas | Registrar rutas dirigidas, opcionalmente de ida y vuelta. Listar, eliminar, consultar una ruta directa y calcular el camino más corto. |
| 3 | Pasajeros | Registrar, listar, buscar, actualizar y eliminar pasajeros. |
| 4 | Cola de solicitudes | Encolar un viaje, ver el siguiente, atenderlo, listar la cola o cancelar el primero. |
| 5 | Historial | Ver los viajes atendidos, rechazados o cancelados, completos o por documento. |
| 6 | Búsquedas | Buscar ciudad, pasajero, ruta directa, camino, solicitudes e historial. |
| 7 | Reportes | Totales de la red, ciudad más conectada, rutas extremas, edades y estado de solicitudes. |
| 8 | Archivos | Guardar o cargar toda la información en la carpeta `datos/`. |
| 0 | Salir | Libera la memoria y cierra el programa. |

Reglas de operación:

- No se aceptan nombres vacíos ni el carácter `|`, porque ese carácter separa los campos en los archivos.
- Una ciudad no se conecta consigo misma y la distancia debe ser mayor que cero.
- Una solicitud exige un pasajero ya registrado y dos ciudades distintas.
- Atender una solicitud la saca de la cola. Si existe camino, queda `ATENDIDA` con su distancia. Si el pasajero ya no existe o no hay camino, queda `RECHAZADA`. Cancelar la primera solicitud la marca como `CANCELADA`.
- Las rutas son dirigidas. El regreso solo se crea si se responde `s` al registrarla.
- Las conexiones creadas en el módulo de ciudades y las rutas del módulo de rutas pertenecen al mismo grafo.

La carpeta `datos/` incluye un ejemplo con ciudades de Colombia, pasajeros, dos solicitudes y un viaje ya atendido. Para usarlo: opción `8`, luego `2`.

## Estructura de archivos

```text
SITRAC/
├── SITRAC.dev              Proyecto de Dev-C++
├── Makefile                Compilación opcional con g++
├── README.md
├── .gitignore
├── src/
│   ├── main.cpp            Menú principal e integración
│   ├── util.h / util.cpp   Lectura, recorte y formato
│   ├── ciudades.h / .cpp   Ciudades y conexiones
│   ├── pasajeros.h / .cpp  Pasajeros
│   ├── rutas.h / .cpp      Rutas y camino más corto
│   ├── cola.h / .cpp       Cola de solicitudes
│   ├── historial.h / .cpp  Historial
│   ├── busquedas.h / .cpp  Búsquedas
│   ├── reportes.h / .cpp   Reportes
│   └── archivos.h / .cpp   Guardar y cargar
├── datos/
│   ├── ciudades.txt
│   ├── rutas.txt
│   ├── pasajeros.txt
│   ├── cola.txt
│   └── historial.txt
└── pruebas/
    ├── probar.sh
    └── entrada_*.txt       Recorridos automáticos del menú
```

## Estructuras de datos utilizadas

No se usan `vector`, `list`, `queue`, `map` ni otras estructuras de la STL para el modelo. `std::string` se usa solo para el texto y `fstream` para los archivos.

- **Lista enlazada simple de ciudades.** Cada nodo guarda el nombre, el puntero a la siguiente ciudad y el inicio de sus conexiones.
- **Grafo dirigido con listas de adyacencia.** Cada conexión es un nodo con destino, distancia y puntero a la siguiente arista.
- **Lista enlazada simple de pasajeros.** Documento, nombre, edad y teléfono.
- **Cola FIFO de solicitudes.** Se inserta por el final y se atiende por el frente, con punteros `frente` y `fin`.
- **Lista enlazada de historial.** Los registros se agregan al final para conservar el orden cronológico.
- **Dijkstra sin cola de prioridad de la STL.** El camino más corto recorre un arreglo fijo de ciudades y elige en cada paso la distancia pendiente más pequeña. Las distancias del grafo son positivas.

```text
Ciudad -> Ciudad -> NULL
  |
  +--> Conexion -> Conexion -> NULL

Cola: frente -> Solicitud -> Solicitud -> fin -> NULL
```

Al cerrar el programa, o antes de una carga nueva, se liberan todos los nodos con `delete`.

## Requisitos

- Dev-C++ 5.11 u otra versión con MinGW g++ (el código está escrito en C++98).
- No hacen falta librerías externas.
- En Linux o macOS basta con `g++` y `make`, de forma opcional.
- El programa debe ejecutarse desde la carpeta del proyecto para que encuentre `datos/`.

Los decimales usan punto (`415.5`), no coma.

## Compilar y ejecutar en Dev-C++

1. Descargar o clonar este repositorio.
2. Abrir `SITRAC.dev` con Dev-C++. No hace falta crear otro proyecto ni agregar los archivos a mano: el proyecto ya incluye las fuentes de `src/`.
3. Elegir **Ejecutar > Compilar y ejecutar** (F11).
4. El ejecutable queda como `SITRAC.exe` en la carpeta del proyecto.
5. Si la consola se cierra al terminar, activar en Dev-C++ la opción de mantener abierta la ventana del programa, o abrir `cmd` en la carpeta del proyecto y ejecutar `SITRAC.exe`.

Para probar el ejemplo incluido: `8` (archivos) y después `2` (cargar).

### Compilación opcional con g++

Desde la carpeta del proyecto:

```bash
make
./sitrac
```

La prueba automática recorre ciudades, rutas, pasajeros, cola, historial, búsquedas, reportes, guardado y carga:

```bash
sh pruebas/probar.sh
```

## Formato de los archivos

| Archivo | Contenido |
| --- | --- |
| `datos/ciudades.txt` | Un nombre por línea. |
| `datos/rutas.txt` | `origen\|destino\|distancia` |
| `datos/pasajeros.txt` | `documento\|nombre\|edad\|telefono` |
| `datos/cola.txt` | Primera línea `#SIGUIENTE id`, luego `id\|documento\|origen\|destino\|fecha`. |
| `datos/historial.txt` | `id\|documento\|nombre\|origen\|destino\|distancia\|fecha\|resultado` |

Las líneas vacías y las que empiezan por `#` son comentarios, excepto `#SIGUIENTE` en la cola. Guardar reemplaza esos cinco archivos. Cargar vacía la memoria y reconstruye las listas a partir de ellos.
