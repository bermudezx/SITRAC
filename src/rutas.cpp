#include "rutas.h"
#include "util.h"

#include <limits>

static const int MAX_CIUDADES = 300;
static const double INFINITO = 1e15;

void listarTodasLasRutas(Ciudad* inicio) {
    if (contarRutas(inicio) == 0) {
        cout << "\nNo hay rutas registradas.\n";
        return;
    }

    cout << "\n===== RUTAS REGISTRADAS =====\n";
    Ciudad* actual = inicio;
    while (actual != NULL) {
        Conexion* conexion = actual->conexiones;
        while (conexion != NULL) {
            cout << "- " << actual->nombre << " -> " << conexion->destino
                 << " (" << textoDistancia(conexion->distancia) << " km)\n";
            conexion = conexion->siguiente;
        }
        actual = actual->siguiente;
    }
}

static void informarResultadoConexion(int resultado, bool esRegreso) {
    string prefijo = esRegreso ? "Regreso: " : "";

    if (resultado == 0) {
        cout << prefijo << "Ruta registrada correctamente.\n";
    } else if (resultado == 1) {
        cout << "La ciudad de origen no existe.\n";
    } else if (resultado == 2) {
        cout << "La ciudad de destino no existe.\n";
    } else if (resultado == 3) {
        cout << "Una ciudad no puede conectarse consigo misma.\n";
    } else if (resultado == 4) {
        cout << prefijo << "Esta ruta ya existe.\n";
    } else {
        cout << "La distancia debe ser mayor que cero.\n";
    }
}

static void registrarRuta(Ciudad* inicio) {
    string origen = leerTexto("\nCiudad de origen: ");
    string destino = leerTexto("Ciudad de destino: ");

    double distancia = 0;
    if (!leerDouble("Distancia en kilometros: ", distancia)) {
        return;
    }

    int resultado = agregarConexionDatos(inicio, origen, destino, distancia);
    informarResultadoConexion(resultado, false);
    if (resultado != 0) {
        return;
    }

    string respuesta = leerTexto("Desea registrar tambien el regreso con la misma distancia? (s/n): ");
    if (respuesta == "s" || respuesta == "S") {
        int regreso = agregarConexionDatos(inicio, destino, origen, distancia);
        informarResultadoConexion(regreso, true);
    }
}

static void eliminarRuta(Ciudad* inicio) {
    string origen = leerTexto("\nCiudad de origen: ");
    string destino = leerTexto("Ciudad de destino: ");

    if (!eliminarConexionDatos(inicio, origen, destino)) {
        cout << "La ruta indicada no existe.\n";
        return;
    }

    cout << "Ruta eliminada correctamente.\n";

    Ciudad* ciudadDestino = buscarCiudad(inicio, destino);
    if (ciudadDestino != NULL && buscarConexion(ciudadDestino, trim(origen)) != NULL) {
        string respuesta = leerTexto("Tambien existe el regreso. Desea eliminarlo? (s/n): ");
        if (respuesta == "s" || respuesta == "S") {
            if (eliminarConexionDatos(inicio, destino, origen)) {
                cout << "Ruta de regreso eliminada correctamente.\n";
            }
        }
    }
}

static void consultarRutaDirecta(Ciudad* inicio) {
    string origen = leerTexto("\nCiudad de origen: ");
    string destino = leerTexto("Ciudad de destino: ");

    Ciudad* ciudadOrigen = buscarCiudad(inicio, origen);
    if (ciudadOrigen == NULL) {
        cout << "La ciudad de origen no existe.\n";
        return;
    }

    if (buscarCiudad(inicio, destino) == NULL) {
        cout << "La ciudad de destino no existe.\n";
        return;
    }

    Conexion* conexion = buscarConexion(ciudadOrigen, trim(destino));
    if (conexion == NULL) {
        cout << "No existe una ruta directa entre esas ciudades.\n";
        return;
    }

    cout << "Ruta directa: " << ciudadOrigen->nombre << " -> " << conexion->destino
         << " (" << textoDistancia(conexion->distancia) << " km)\n";
}

ResultadoCamino calcularCaminoMasCorto(Ciudad* inicio, const string& origen, const string& destino) {
    ResultadoCamino resultado;
    resultado.encontrado = false;
    resultado.distancia = 0;
    resultado.recorrido = "";

    int total = contarCiudades(inicio);
    if (total <= 0 || total > MAX_CIUDADES) {
        return resultado;
    }

    string nombres[MAX_CIUDADES];
    double dist[MAX_CIUDADES];
    int previo[MAX_CIUDADES];
    bool visitado[MAX_CIUDADES];

    int n = 0;
    Ciudad* actual = inicio;
    while (actual != NULL && n < MAX_CIUDADES) {
        nombres[n] = actual->nombre;
        dist[n] = INFINITO;
        previo[n] = -1;
        visitado[n] = false;
        n++;
        actual = actual->siguiente;
    }

    int indiceOrigen = -1;
    int indiceDestino = -1;
    string origenLimpio = trim(origen);
    string destinoLimpio = trim(destino);

    for (int i = 0; i < n; i++) {
        if (nombres[i] == origenLimpio) {
            indiceOrigen = i;
        }
        if (nombres[i] == destinoLimpio) {
            indiceDestino = i;
        }
    }

    if (indiceOrigen == -1 || indiceDestino == -1) {
        return resultado;
    }

    dist[indiceOrigen] = 0;

    for (int paso = 0; paso < n; paso++) {
        int seleccionado = -1;
        double mejor = INFINITO;

        for (int i = 0; i < n; i++) {
            if (!visitado[i] && dist[i] < mejor) {
                mejor = dist[i];
                seleccionado = i;
            }
        }

        if (seleccionado == -1 || dist[seleccionado] >= INFINITO) {
            break;
        }

        visitado[seleccionado] = true;
        Ciudad* ciudad = buscarCiudad(inicio, nombres[seleccionado]);
        Conexion* arista = (ciudad != NULL) ? ciudad->conexiones : NULL;

        while (arista != NULL) {
            int vecino = -1;
            for (int i = 0; i < n; i++) {
                if (nombres[i] == arista->destino) {
                    vecino = i;
                    break;
                }
            }

            if (vecino != -1 && !visitado[vecino] && dist[seleccionado] + arista->distancia < dist[vecino]) {
                dist[vecino] = dist[seleccionado] + arista->distancia;
                previo[vecino] = seleccionado;
            }

            arista = arista->siguiente;
        }
    }

    if (dist[indiceDestino] >= INFINITO) {
        return resultado;
    }

    int cadena[MAX_CIUDADES];
    int longitud = 0;
    int cursor = indiceDestino;

    while (cursor != -1 && longitud < MAX_CIUDADES) {
        cadena[longitud] = cursor;
        longitud++;
        if (cursor == indiceOrigen) {
            break;
        }
        cursor = previo[cursor];
    }

    string recorrido = "";
    for (int i = longitud - 1; i >= 0; i--) {
        if (!recorrido.empty()) {
            recorrido += " -> ";
        }
        recorrido += nombres[cadena[i]];
    }

    resultado.encontrado = true;
    resultado.distancia = dist[indiceDestino];
    resultado.recorrido = recorrido;
    return resultado;
}

static void mostrarCaminoMasCorto(Ciudad* inicio) {
    string origen = leerTexto("\nCiudad de origen: ");
    string destino = leerTexto("Ciudad de destino: ");

    if (buscarCiudad(inicio, origen) == NULL || buscarCiudad(inicio, destino) == NULL) {
        cout << "Debe indicar dos ciudades registradas.\n";
        return;
    }

    ResultadoCamino camino = calcularCaminoMasCorto(inicio, origen, destino);
    if (!camino.encontrado) {
        cout << "No existe un camino entre esas ciudades.\n";
        return;
    }

    cout << "Camino mas corto: " << camino.recorrido << endl;
    cout << "Distancia total: " << textoDistancia(camino.distancia) << " km\n";
}

void menuRutas(Ciudad* inicio) {
    int opcion;

    do {
        cout << "\n===== MODULO DE RUTAS =====\n";
        cout << "El mapa es un grafo dirigido. Cada ruta sale de una ciudad y llega a otra.\n";
        cout << "1. Registrar ruta\n";
        cout << "2. Listar todas las rutas\n";
        cout << "3. Eliminar ruta\n";
        cout << "4. Consultar ruta directa\n";
        cout << "5. Calcular camino mas corto\n";
        cout << "0. Volver al menu principal\n";
        cout << "Seleccione una opcion: ";

        if (!(cin >> opcion)) {
            cout << "Debe ingresar un numero.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            opcion = -1;
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            switch (opcion) {
                case 1:
                    registrarRuta(inicio);
                    break;
                case 2:
                    listarTodasLasRutas(inicio);
                    break;
                case 3:
                    eliminarRuta(inicio);
                    break;
                case 4:
                    consultarRutaDirecta(inicio);
                    break;
                case 5:
                    mostrarCaminoMasCorto(inicio);
                    break;
                case 0:
                    cout << "Regresando al menu principal...\n";
                    break;
                default:
                    cout << "Opcion no valida.\n";
            }
        }
    } while (opcion != 0);
}
