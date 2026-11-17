#include "busquedas.h"
#include "rutas.h"
#include "util.h"

#include <limits>

static void buscarCiudadInteractivo(Ciudad* ciudades) {
    string nombre = leerTexto("\nNombre de la ciudad: ");
    if (nombre.empty()) {
        cout << "Debe escribir un nombre.\n";
        return;
    }

    Ciudad* exacta = buscarCiudad(ciudades, nombre);
    if (exacta != NULL) {
        cout << "Ciudad encontrada: " << exacta->nombre << endl;
        Conexion* conexion = exacta->conexiones;
        if (conexion == NULL) {
            cout << "No tiene rutas de salida.\n";
        } else {
            cout << "Rutas de salida:\n";
            while (conexion != NULL) {
                cout << "- " << conexion->destino << " (" << textoDistancia(conexion->distancia) << " km)\n";
                conexion = conexion->siguiente;
            }
        }
        return;
    }

    bool alguna = false;
    Ciudad* actual = ciudades;
    while (actual != NULL) {
        if (actual->nombre.find(nombre) != string::npos) {
            if (!alguna) {
                cout << "No hay una coincidencia exacta. Coincidencias parciales:\n";
            }
            cout << "- " << actual->nombre << endl;
            alguna = true;
        }
        actual = actual->siguiente;
    }

    if (!alguna) {
        cout << "No se encontro la ciudad.\n";
    }
}

static void buscarPasajeroPorNombre(Pasajero* pasajeros) {
    string nombre = leerTexto("\nNombre o parte del nombre: ");
    if (nombre.empty()) {
        cout << "Debe escribir un nombre.\n";
        return;
    }

    bool encontro = false;
    Pasajero* actual = pasajeros;
    while (actual != NULL) {
        if (actual->nombre.find(nombre) != string::npos) {
            cout << "\nDocumento: " << actual->documento << endl;
            cout << "Nombre: " << actual->nombre << endl;
            cout << "Edad: " << actual->edad << endl;
            cout << "Telefono: " << actual->telefono << endl;
            encontro = true;
        }
        actual = actual->siguiente;
    }

    if (!encontro) {
        cout << "No se encontraron pasajeros con ese nombre.\n";
    }
}

static void buscarPasajeroPorDocumento(Pasajero* pasajeros) {
    string documento = leerTexto("\nDocumento: ");
    Pasajero* encontrado = buscarPasajero(pasajeros, documento);

    if (encontrado == NULL) {
        cout << "Pasajero no encontrado.\n";
        return;
    }

    cout << "Nombre: " << encontrado->nombre << endl;
    cout << "Edad: " << encontrado->edad << endl;
    cout << "Telefono: " << encontrado->telefono << endl;
}

static void buscarRutaDirecta(Ciudad* ciudades) {
    string origen = leerTexto("\nCiudad de origen: ");
    string destino = leerTexto("Ciudad de destino: ");
    Ciudad* ciudadOrigen = buscarCiudad(ciudades, origen);

    if (ciudadOrigen == NULL || buscarCiudad(ciudades, destino) == NULL) {
        cout << "Debe indicar dos ciudades registradas.\n";
        return;
    }

    Conexion* conexion = buscarConexion(ciudadOrigen, trim(destino));
    if (conexion == NULL) {
        cout << "No existe una ruta directa.\n";
        return;
    }

    cout << "Ruta encontrada: " << ciudadOrigen->nombre << " -> " << conexion->destino
         << " (" << textoDistancia(conexion->distancia) << " km)\n";
}

static void buscarCamino(Ciudad* ciudades) {
    string origen = leerTexto("\nCiudad de origen: ");
    string destino = leerTexto("Ciudad de destino: ");

    if (buscarCiudad(ciudades, origen) == NULL || buscarCiudad(ciudades, destino) == NULL) {
        cout << "Debe indicar dos ciudades registradas.\n";
        return;
    }

    ResultadoCamino camino = calcularCaminoMasCorto(ciudades, origen, destino);
    if (!camino.encontrado) {
        cout << "No existe un camino entre esas ciudades.\n";
        return;
    }

    cout << "Camino encontrado: " << camino.recorrido << endl;
    cout << "Distancia total: " << textoDistancia(camino.distancia) << " km\n";
}

void menuBusquedas(Ciudad* ciudades, Pasajero* pasajeros, const Cola& cola, Historial* historial) {
    int opcion;

    do {
        cout << "\n===== MODULO DE BUSQUEDAS =====\n";
        cout << "1. Buscar ciudad\n";
        cout << "2. Buscar pasajero por documento\n";
        cout << "3. Buscar pasajero por nombre\n";
        cout << "4. Buscar ruta directa\n";
        cout << "5. Buscar camino mas corto\n";
        cout << "6. Buscar solicitudes por documento\n";
        cout << "7. Buscar historial por documento\n";
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
                    buscarCiudadInteractivo(ciudades);
                    break;
                case 2:
                    buscarPasajeroPorDocumento(pasajeros);
                    break;
                case 3:
                    buscarPasajeroPorNombre(pasajeros);
                    break;
                case 4:
                    buscarRutaDirecta(ciudades);
                    break;
                case 5:
                    buscarCamino(ciudades);
                    break;
                case 6: {
                    string documento = leerTexto("Documento: ");
                    listarColaPorDocumento(cola, documento);
                    break;
                }
                case 7: {
                    string documento = leerTexto("Documento: ");
                    listarHistorialPorDocumento(historial, documento);
                    break;
                }
                case 0:
                    cout << "Regresando al menu principal...\n";
                    break;
                default:
                    cout << "Opcion no valida.\n";
            }
        }
    } while (opcion != 0);
}
