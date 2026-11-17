#include <iostream>
#include <limits>

#include "archivos.h"
#include "busquedas.h"
#include "ciudades.h"
#include "cola.h"
#include "historial.h"
#include "pasajeros.h"
#include "reportes.h"
#include "rutas.h"

using namespace std;

static void liberarSistema(Ciudad*& ciudades, Pasajero*& pasajeros, Cola& cola, Historial*& historial) {
    liberarCiudades(ciudades);
    liberarPasajeros(pasajeros);
    liberarCola(cola);
    liberarHistorial(historial);
}

int main() {
    Ciudad* listaCiudades = NULL;
    Pasajero* listaPasajeros = NULL;
    Cola cola;
    Historial* historial = NULL;
    inicializarCola(cola);

    int opcion;

    do {
        cout << "\n========================================" << endl;
        cout << "                 SITRAC                 " << endl;
        cout << " Sistema Inteligente de Transporte     " << endl;
        cout << "          y Gestion de Rutas           " << endl;
        cout << "       Universidad Santo Tomas         " << endl;
        cout << "========================================" << endl;

        cout << "\n1. Ciudades" << endl;
        cout << "2. Rutas" << endl;
        cout << "3. Pasajeros" << endl;
        cout << "4. Cola de solicitudes" << endl;
        cout << "5. Historial" << endl;
        cout << "6. Busquedas" << endl;
        cout << "7. Reportes" << endl;
        cout << "8. Guardar / Cargar archivos" << endl;
        cout << "0. Salir" << endl;
        cout << "\nSeleccione una opcion: ";

        if (!(cin >> opcion)) {
            cout << "\nDebe ingresar un numero." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            opcion = -1;
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            switch (opcion) {
                case 1:
                    menuCiudades(listaCiudades);
                    break;
                case 2:
                    menuRutas(listaCiudades);
                    break;
                case 3:
                    menuPasajeros(listaPasajeros);
                    break;
                case 4:
                    menuCola(cola, listaCiudades, listaPasajeros, historial);
                    break;
                case 5:
                    menuHistorial(historial);
                    break;
                case 6:
                    menuBusquedas(listaCiudades, listaPasajeros, cola, historial);
                    break;
                case 7:
                    menuReportes(listaCiudades, listaPasajeros, cola, historial);
                    break;
                case 8:
                    menuArchivos(listaCiudades, listaPasajeros, cola, historial);
                    break;
                case 0:
                    cout << "\nCerrando SITRAC..." << endl;
                    break;
                default:
                    cout << "\nOpcion no valida." << endl;
            }
        }
    } while (opcion != 0);

    liberarSistema(listaCiudades, listaPasajeros, cola, historial);
    cout << "\nGracias por utilizar SITRAC." << endl;
    return 0;
}
