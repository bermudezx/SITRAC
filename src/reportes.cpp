#include "reportes.h"
#include "util.h"

#include <limits>

static int contarConexionesSalida(const Ciudad* ciudad) {
    int total = 0;
    const Conexion* actual = ciudad->conexiones;
    while (actual != NULL) {
        total++;
        actual = actual->siguiente;
    }
    return total;
}

static void reporteGeneral(Ciudad* ciudades, Pasajero* pasajeros, const Cola& cola, Historial* historial) {
    double distanciaTotal = 0;
    Ciudad* actual = ciudades;
    while (actual != NULL) {
        Conexion* conexion = actual->conexiones;
        while (conexion != NULL) {
            distanciaTotal += conexion->distancia;
            conexion = conexion->siguiente;
        }
        actual = actual->siguiente;
    }

    cout << "\n===== REPORTE GENERAL =====\n";
    cout << "Ciudades: " << contarCiudades(ciudades) << endl;
    cout << "Rutas dirigidas: " << contarRutas(ciudades) << endl;
    cout << "Pasajeros: " << contarPasajeros(pasajeros) << endl;
    cout << "Solicitudes pendientes: " << contarCola(cola) << endl;
    cout << "Registros de historial: " << contarHistorial(historial) << endl;
    cout << "Distancia acumulada de la red: " << textoDistancia(distanciaTotal) << " km\n";
}

static void reporteCiudadMasConectada(Ciudad* ciudades) {
    if (ciudades == NULL) {
        cout << "\nNo hay ciudades registradas.\n";
        return;
    }

    Ciudad* mejor = NULL;
    int mejorCantidad = -1;
    Ciudad* actual = ciudades;

    while (actual != NULL) {
        int cantidad = contarConexionesSalida(actual);
        if (cantidad > mejorCantidad) {
            mejorCantidad = cantidad;
            mejor = actual;
        }
        actual = actual->siguiente;
    }

    cout << "\nCiudad con mas conexiones de salida: " << mejor->nombre << endl;
    cout << "Conexiones: " << mejorCantidad << endl;

    Conexion* conexion = mejor->conexiones;
    if (conexion == NULL) {
        cout << "Todavia no tiene rutas de salida.\n";
        return;
    }

    while (conexion != NULL) {
        cout << "- " << conexion->destino << " (" << textoDistancia(conexion->distancia) << " km)\n";
        conexion = conexion->siguiente;
    }
}

static void reporteRutasExtremas(Ciudad* ciudades) {
    bool hay = false;
    double menor = 0;
    double mayor = 0;
    string textoMenor = "";
    string textoMayor = "";

    Ciudad* actual = ciudades;
    while (actual != NULL) {
        Conexion* conexion = actual->conexiones;
        while (conexion != NULL) {
            string descripcion = actual->nombre + " -> " + conexion->destino + " (" +
                                 textoDistancia(conexion->distancia) + " km)";
            if (!hay || conexion->distancia < menor) {
                menor = conexion->distancia;
                textoMenor = descripcion;
            }
            if (!hay || conexion->distancia > mayor) {
                mayor = conexion->distancia;
                textoMayor = descripcion;
            }
            hay = true;
            conexion = conexion->siguiente;
        }
        actual = actual->siguiente;
    }

    if (!hay) {
        cout << "\nNo hay rutas registradas.\n";
        return;
    }

    cout << "\nRuta directa mas corta: " << textoMenor << endl;
    cout << "Ruta directa mas larga: " << textoMayor << endl;
}

static void reporteEdades(Pasajero* pasajeros) {
    if (pasajeros == NULL) {
        cout << "\nNo hay pasajeros registrados.\n";
        return;
    }

    int menores = 0;
    int adultos = 0;
    int suma = 0;
    int total = 0;

    while (pasajeros != NULL) {
        suma += pasajeros->edad;
        total++;
        if (pasajeros->edad < 18) {
            menores++;
        } else {
            adultos++;
        }
        pasajeros = pasajeros->siguiente;
    }

    cout << "\n===== PASAJEROS POR EDAD =====\n";
    cout << "Menores de 18: " << menores << endl;
    cout << "Adultos: " << adultos << endl;
    cout << "Edad promedio: " << textoDistancia(static_cast<double>(suma) / total) << endl;
}

static void reporteSolicitudes(const Cola& cola, Historial* historial) {
    cout << "\n===== ESTADO DE SOLICITUDES =====\n";
    cout << "Pendientes en cola: " << contarCola(cola) << endl;
    cout << "Atendidas: " << contarHistorialPorResultado(historial, "ATENDIDA") << endl;
    cout << "Rechazadas: " << contarHistorialPorResultado(historial, "RECHAZADA") << endl;
    cout << "Canceladas: " << contarHistorialPorResultado(historial, "CANCELADA") << endl;
    cout << "Total en historial: " << contarHistorial(historial) << endl;
}

void menuReportes(Ciudad* ciudades, Pasajero* pasajeros, const Cola& cola, Historial* historial) {
    int opcion;

    do {
        cout << "\n===== MODULO DE REPORTES =====\n";
        cout << "1. Reporte general del sistema\n";
        cout << "2. Ciudad con mas conexiones\n";
        cout << "3. Ruta mas larga y ruta mas corta\n";
        cout << "4. Pasajeros por grupo de edad\n";
        cout << "5. Estado de solicitudes\n";
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
                    reporteGeneral(ciudades, pasajeros, cola, historial);
                    break;
                case 2:
                    reporteCiudadMasConectada(ciudades);
                    break;
                case 3:
                    reporteRutasExtremas(ciudades);
                    break;
                case 4:
                    reporteEdades(pasajeros);
                    break;
                case 5:
                    reporteSolicitudes(cola, historial);
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
