#include "cola.h"
#include "rutas.h"
#include "util.h"

#include <limits>

void inicializarCola(Cola& cola) {
    cola.frente = NULL;
    cola.fin = NULL;
    cola.siguienteId = 1;
}

void liberarCola(Cola& cola) {
    while (cola.frente != NULL) {
        Solicitud* temporal = cola.frente;
        cola.frente = cola.frente->siguiente;
        delete temporal;
    }
    cola.fin = NULL;
    cola.siguienteId = 1;
}

int contarCola(const Cola& cola) {
    int total = 0;
    Solicitud* actual = cola.frente;
    while (actual != NULL) {
        total++;
        actual = actual->siguiente;
    }
    return total;
}

bool encolarExistente(Cola& cola, int id, const string& documento, const string& origen,
                      const string& destino, const string& fecha) {
    if (id <= 0 || documento.empty() || origen.empty() || destino.empty() || fecha.empty()) {
        return false;
    }

    Solicitud* nueva = new Solicitud;
    nueva->id = id;
    nueva->documento = documento;
    nueva->origen = origen;
    nueva->destino = destino;
    nueva->fecha = fecha;
    nueva->siguiente = NULL;

    if (cola.fin == NULL) {
        cola.frente = nueva;
        cola.fin = nueva;
    } else {
        cola.fin->siguiente = nueva;
        cola.fin = nueva;
    }

    if (id >= cola.siguienteId) {
        cola.siguienteId = id + 1;
    }

    return true;
}

static bool encolarNueva(Cola& cola, const string& documento, const string& origen,
                         const string& destino, const string& fecha) {
    return encolarExistente(cola, cola.siguienteId, documento, origen, destino, fecha);
}

static void imprimirSolicitud(const Solicitud* actual) {
    cout << "\nSolicitud #" << actual->id << endl;
    cout << "Documento: " << actual->documento << endl;
    cout << "Origen: " << actual->origen << endl;
    cout << "Destino: " << actual->destino << endl;
    cout << "Fecha: " << actual->fecha << endl;
}

void listarCola(const Cola& cola) {
    if (cola.frente == NULL) {
        cout << "\nNo hay solicitudes pendientes.\n";
        return;
    }

    cout << "\n===== COLA DE SOLICITUDES =====\n";
    cout << "El primer registro es el siguiente en ser atendido.\n";

    Solicitud* actual = cola.frente;
    int posicion = 1;
    while (actual != NULL) {
        cout << "\nPosicion " << posicion << endl;
        imprimirSolicitud(actual);
        actual = actual->siguiente;
        posicion++;
    }
}

void listarColaPorDocumento(const Cola& cola, const string& documento) {
    string doc = trim(documento);
    bool encontro = false;
    Solicitud* actual = cola.frente;

    while (actual != NULL) {
        if (actual->documento == doc) {
            if (!encontro) {
                cout << "\nSolicitudes pendientes del documento " << doc << ":\n";
            }
            imprimirSolicitud(actual);
            encontro = true;
        }
        actual = actual->siguiente;
    }

    if (!encontro) {
        cout << "Ese documento no tiene solicitudes pendientes.\n";
    }
}

static bool sacarFrente(Cola& cola, Solicitud& copia) {
    if (cola.frente == NULL) {
        return false;
    }

    Solicitud* nodo = cola.frente;
    copia = *nodo;
    copia.siguiente = NULL;

    cola.frente = nodo->siguiente;
    if (cola.frente == NULL) {
        cola.fin = NULL;
    }

    delete nodo;
    return true;
}

static void registrarSolicitud(Cola& cola, Ciudad* ciudades, Pasajero* pasajeros) {
    string documento = leerTexto("\nDocumento del pasajero: ");
    Pasajero* pasajero = buscarPasajero(pasajeros, documento);
    if (pasajero == NULL) {
        cout << "El pasajero no esta registrado.\n";
        return;
    }

    string origen = leerTexto("Ciudad de origen: ");
    string destino = leerTexto("Ciudad de destino: ");

    if (buscarCiudad(ciudades, origen) == NULL) {
        cout << "La ciudad de origen no existe.\n";
        return;
    }
    if (buscarCiudad(ciudades, destino) == NULL) {
        cout << "La ciudad de destino no existe.\n";
        return;
    }
    if (trim(origen) == trim(destino)) {
        cout << "El origen y el destino deben ser diferentes.\n";
        return;
    }

    string fecha = leerTexto("Fecha del viaje (ejemplo 2026-11-17): ");
    if (fecha.empty()) {
        cout << "La fecha no puede estar vacia.\n";
        return;
    }
    if (contieneSeparador(fecha)) {
        cout << "La fecha no puede contener el caracter |.\n";
        return;
    }

    if (!encolarNueva(cola, pasajero->documento, trim(origen), trim(destino), fecha)) {
        cout << "No se pudo registrar la solicitud.\n";
        return;
    }

    cout << "Solicitud #" << (cola.siguienteId - 1) << " agregada al final de la cola.\n";

    ResultadoCamino camino = calcularCaminoMasCorto(ciudades, origen, destino);
    if (!camino.encontrado) {
        cout << "Aviso: todavia no existe un camino entre esas ciudades.\n";
    }
}

static void atenderSiguiente(Cola& cola, Ciudad* ciudades, Pasajero* pasajeros, Historial*& historial, bool cancelar) {
    if (cola.frente == NULL) {
        cout << "\nNo hay solicitudes pendientes.\n";
        return;
    }

    Solicitud solicitud;
    if (!sacarFrente(cola, solicitud)) {
        cout << "\nNo hay solicitudes pendientes.\n";
        return;
    }

    Pasajero* pasajero = buscarPasajero(pasajeros, solicitud.documento);
    string nombre = (pasajero != NULL) ? pasajero->nombre : "Pasajero no registrado";
    string resultado = "RECHAZADA";
    double distancia = 0;
    string detalle = "No existe un camino entre las ciudades.";

    cout << "\nProcesando solicitud #" << solicitud.id << endl;
    cout << "Pasajero: " << nombre << " (" << solicitud.documento << ")\n";
    cout << "Recorrido solicitado: " << solicitud.origen << " -> " << solicitud.destino << endl;

    if (cancelar) {
        resultado = "CANCELADA";
        detalle = "La solicitud fue cancelada antes de asignar una ruta.";
    } else if (pasajero == NULL) {
        detalle = "El pasajero ya no esta registrado.";
    } else {
        ResultadoCamino camino = calcularCaminoMasCorto(ciudades, solicitud.origen, solicitud.destino);
        if (camino.encontrado) {
            resultado = "ATENDIDA";
            distancia = camino.distancia;
            detalle = camino.recorrido;
        }
    }

    agregarHistorial(historial, solicitud.id, solicitud.documento, nombre, solicitud.origen,
                     solicitud.destino, distancia, solicitud.fecha, resultado);

    cout << "Detalle: " << detalle << endl;
    if (resultado == "ATENDIDA") {
        cout << "Distancia asignada: " << textoDistancia(distancia) << " km\n";
    }
    cout << "Resultado: " << resultado << endl;
}

void menuCola(Cola& cola, Ciudad* ciudades, Pasajero* pasajeros, Historial*& historial) {
    int opcion;

    do {
        cout << "\n===== COLA DE SOLICITUDES =====\n";
        cout << "Pendientes: " << contarCola(cola) << endl;
        cout << "1. Registrar solicitud\n";
        cout << "2. Ver siguiente solicitud\n";
        cout << "3. Atender siguiente solicitud\n";
        cout << "4. Listar solicitudes pendientes\n";
        cout << "5. Cancelar siguiente solicitud\n";
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
                    registrarSolicitud(cola, ciudades, pasajeros);
                    break;
                case 2:
                    if (cola.frente == NULL) {
                        cout << "\nNo hay solicitudes pendientes.\n";
                    } else {
                        cout << "\nSiguiente solicitud por atender:\n";
                        imprimirSolicitud(cola.frente);
                    }
                    break;
                case 3:
                    atenderSiguiente(cola, ciudades, pasajeros, historial, false);
                    break;
                case 4:
                    listarCola(cola);
                    break;
                case 5:
                    atenderSiguiente(cola, ciudades, pasajeros, historial, true);
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
