#include "historial.h"
#include "util.h"

#include <limits>

void agregarHistorial(Historial*& inicio, int idSolicitud, const string& documento,
                      const string& nombrePasajero, const string& origen,
                      const string& destino, double distancia, const string& fecha,
                      const string& resultado) {
    Historial* nuevo = new Historial;
    nuevo->idSolicitud = idSolicitud;
    nuevo->documento = documento;
    nuevo->nombrePasajero = nombrePasajero;
    nuevo->origen = origen;
    nuevo->destino = destino;
    nuevo->distancia = distancia;
    nuevo->fecha = fecha;
    nuevo->resultado = resultado;
    nuevo->siguiente = NULL;

    if (inicio == NULL) {
        inicio = nuevo;
        return;
    }

    Historial* actual = inicio;
    while (actual->siguiente != NULL) {
        actual = actual->siguiente;
    }
    actual->siguiente = nuevo;
}

static void imprimirRegistro(const Historial* actual) {
    cout << "\nSolicitud #" << actual->idSolicitud << endl;
    cout << "Pasajero: " << actual->nombrePasajero << " (" << actual->documento << ")\n";
    cout << "Recorrido: " << actual->origen << " -> " << actual->destino << endl;
    cout << "Distancia: " << textoDistancia(actual->distancia) << " km\n";
    cout << "Fecha: " << actual->fecha << endl;
    cout << "Resultado: " << actual->resultado << endl;
}

void listarHistorial(Historial* inicio) {
    if (inicio == NULL) {
        cout << "\nEl historial esta vacio.\n";
        return;
    }

    cout << "\n===== HISTORIAL DE SOLICITUDES =====\n";
    while (inicio != NULL) {
        imprimirRegistro(inicio);
        inicio = inicio->siguiente;
    }
}

void listarHistorialPorDocumento(Historial* inicio, const string& documento) {
    string doc = trim(documento);
    bool encontro = false;

    while (inicio != NULL) {
        if (inicio->documento == doc) {
            if (!encontro) {
                cout << "\n===== HISTORIAL DEL DOCUMENTO " << doc << " =====\n";
            }
            imprimirRegistro(inicio);
            encontro = true;
        }
        inicio = inicio->siguiente;
    }

    if (!encontro) {
        cout << "No hay registros en el historial para ese documento.\n";
    }
}

int contarHistorial(Historial* inicio) {
    int total = 0;
    while (inicio != NULL) {
        total++;
        inicio = inicio->siguiente;
    }
    return total;
}

int contarHistorialPorResultado(Historial* inicio, const string& resultado) {
    int total = 0;
    while (inicio != NULL) {
        if (inicio->resultado == resultado) {
            total++;
        }
        inicio = inicio->siguiente;
    }
    return total;
}

void liberarHistorial(Historial*& inicio) {
    while (inicio != NULL) {
        Historial* temporal = inicio;
        inicio = inicio->siguiente;
        delete temporal;
    }
}

void menuHistorial(Historial* inicio) {
    int opcion;

    do {
        cout << "\n===== MODULO DE HISTORIAL =====\n";
        cout << "1. Listar historial completo\n";
        cout << "2. Consultar historial de un pasajero\n";
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
                    listarHistorial(inicio);
                    break;
                case 2: {
                    string documento = leerTexto("Ingrese el documento: ");
                    listarHistorialPorDocumento(inicio, documento);
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
