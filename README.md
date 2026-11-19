# SITRAC

Sistema Inteligente de Transporte y Gestión de Rutas.

Aplicación de escritorio para Windows, hecha en C++ para la Universidad Santo Tomás. Se abre como ventana, con una barra lateral morada, tarjetas, listas y botones. No usa consola.

Repositorio: [https://github.com/bermudezx/SITRAC](https://github.com/bermudezx/SITRAC)

## Descripción

SITRAC administra una red de ciudades conectadas por rutas dirigidas. Cada ruta guarda la distancia en kilómetros. Sobre esa red se registran pasajeros y una cola de solicitudes de viaje. Al atender la solicitud que está de primera, el sistema busca el camino más corto y guarda el resultado en el historial. Toda la información se puede guardar y volver a cargar.

Al iniciar, el programa intenta cargar la carpeta `datos` que está junto al ejecutable. El repositorio ya trae un ejemplo con ciudades de Colombia.

## Objetivo general

Ofrecer una aplicación Windows, compilable en Dev-C++ con C++ estándar, para gestionar una red de transporte usando estructuras de datos implementadas manualmente: listas enlazadas, un grafo por listas de adyacencia, una cola y el algoritmo de Dijkstra.

## Funcionalidades

La ventana principal tiene una barra lateral morada con ocho secciones:

| Sección | Qué permite |
| --- | --- |
| Ciudades | Registrar y eliminar ciudades. Agregar o quitar conexiones. La lista de la derecha muestra las salidas de la ciudad elegida. |
| Rutas | Registrar rutas dirigidas, también de ida y vuelta. Eliminarlas, verlas todas y calcular el camino más corto. |
| Pasajeros | Registrar, actualizar, eliminar y consultar pasajeros. Al elegir una fila, los datos pasan al formulario. |
| Cola | Encolar un viaje, atender el primero o cancelarlo. La atención usa el camino más corto. |
| Historial | Ver viajes atendidos, rechazados y cancelados. Se puede filtrar por documento. |
| Búsquedas | Buscar ciudad, pasajero, ruta directa, camino, solicitudes e historial. |
| Reportes | Totales de la red, ciudad con más conexiones, rutas extremas, edades y estado de las solicitudes. |
| Archivos | Guardar o cargar `ciudades.txt`, `rutas.txt`, `pasajeros.txt`, `cola.txt` e `historial.txt`. |

Reglas de operación:

- No se aceptan nombres vacíos ni el carácter `|`, porque ese carácter separa los campos en los archivos.
- Una ciudad no se conecta consigo misma y la distancia debe ser mayor que cero. Use punto decimal (`415.5`).
- Una solicitud exige un pasajero ya registrado y dos ciudades distintas.
- Atender una solicitud la saca de la cola. Si existe camino, queda `ATENDIDA`. Si no hay camino o el pasajero ya no existe, queda `RECHAZADA`. Cancelar la primera la marca como `CANCELADA`.
- Las rutas son dirigidas. El regreso solo se crea si se marca la casilla correspondiente.
- Las conexiones de la pestaña Ciudades y las rutas de la pestaña Rutas pertenecen al mismo grafo.

## Estructura de archivos

```text
SITRAC/
├── SITRAC.dev              Proyecto de Dev-C++ (aplicación Windows)
├── Makefile                Prueba opcional de la lógica con g++
├── README.md
├── .gitignore
├── src/
│   ├── main.cpp            Ventana principal (WinMain)
│   ├── util.h / util.cpp
│   ├── ciudades.h / .cpp   Lista de ciudades y conexiones
│   ├── pasajeros.h / .cpp
│   ├── rutas.h / .cpp      Caminos sobre el grafo
│   ├── cola.h / .cpp       Cola de solicitudes
│   ├── historial.h / .cpp
│   ├── busquedas.h / .cpp
│   ├── reportes.h / .cpp
│   └── archivos.h / .cpp   Guardar y cargar
├── datos/                  Ejemplo listo para cargar
└── pruebas/
    ├── logica_main.cpp     Prueba de la lógica, sin ventana
    └── probar.sh
```

## Estructuras de datos utilizadas

No se usan `vector`, `list`, `queue` ni `map` para el modelo. `std::string` guarda el texto y `fstream` lee y escribe los archivos. La ventana usa la API de Windows (Win32).

- **Lista enlazada simple de ciudades.** Cada nodo guarda el nombre y el inicio de sus conexiones.
- **Grafo dirigido con listas de adyacencia.** Cada conexión tiene destino, distancia y puntero a la siguiente arista.
- **Lista enlazada simple de pasajeros.**
- **Cola FIFO de solicitudes.** Se inserta por el final y se atiende por el frente.
- **Lista enlazada de historial**, agregada al final para conservar el orden.
- **Dijkstra sin cola de prioridad de la STL.** En cada paso se elige la ciudad pendiente con menor distancia. Las distancias del grafo son positivas.

Al cerrar la ventana se liberan todos los nodos.

## Requisitos

- Windows.
- Dev-C++ 5.11 u otra versión con MinGW g++. El código está escrito en C++98 y no necesita librerías externas.
- La prueba de lógica, opcional, pide `g++` y `make`. No abre la ventana.

## Compilar y ejecutar en Dev-C++

1. Descargar o clonar este repositorio en Windows.
2. Abrir `SITRAC.dev` con Dev-C++. El proyecto ya es de tipo **Windows Application** e incluye los archivos de `src/`.
3. Elegir **Ejecutar > Compilar y ejecutar** (F11).
4. Se abre la ventana de SITRAC. No aparece una consola.
5. El ejecutable queda como `SITRAC.exe` en la carpeta del proyecto. La carpeta `datos` debe permanecer junto a ese ejecutable para que el ejemplo cargue al iniciar.

Si al abrir no aparecen las ciudades de ejemplo, use la pestaña **Archivos** y el botón **Cargar**, con el programa ejecutado desde la carpeta del proyecto.

### Comprobar la lógica sin abrir la ventana

Desde la carpeta del proyecto, en un sistema con `g++`:

```bash
make test
```

Eso no sustituye la aplicación. La entrega para presentar es la ventana de Windows.

## Formato de los archivos

| Archivo | Contenido |
| --- | --- |
| `datos/ciudades.txt` | Un nombre por línea. |
| `datos/rutas.txt` | `origen\|destino\|distancia` |
| `datos/pasajeros.txt` | `documento\|nombre\|edad\|telefono` |
| `datos/cola.txt` | Primera línea `#SIGUIENTE id`, luego `id\|documento\|origen\|destino\|fecha`. |
| `datos/historial.txt` | `id\|documento\|nombre\|origen\|destino\|distancia\|fecha\|resultado` |

Las líneas vacías y las que empiezan por `#` son comentarios, excepto `#SIGUIENTE` en la cola.
