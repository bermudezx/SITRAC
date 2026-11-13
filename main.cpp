#include <iostream>
#include <limits>
#include "ciudades.h"
#include "pasajeros.h"

using namespace std;

int main() {
	Ciudad* listaCiudades = NULL;
	Pasajero* listaPasajeros = NULL;

    int opcion;

    do {

        cout << "\n========================================" << endl;
        cout << "                 SITRAC                 " << endl;
        cout << " Sistema Inteligente de Transporte     " << endl;
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

            switch (opcion) {

                case 1:
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    menuCiudades(listaCiudades);
    break;
                case 2:
    menuPasajeros(listaPasajeros);
    break;

                case 3:
                    cout << "\nModulo de Pasajeros." << endl;
                    break;

                case 4:
                    cout << "\nModulo de Cola." << endl;
                    break;

                case 5:
                    cout << "\nModulo de Historial." << endl;
                    break;

                case 6:
                    cout << "\nModulo de Busquedas." << endl;
                    break;

                case 7:
                    cout << "\nModulo de Reportes." << endl;
                    break;

                case 8:
                    cout << "\nModulo de Archivos." << endl;
                    break;

                case 0:
                    cout << "\nCerrando SITRAC..." << endl;
                    break;

                default:
                    cout << "\nOpcion no valida." << endl;
            }
        }

    } while (opcion != 0);

    cout << "\nGracias por utilizar SITRAC." << endl;

    return 0;
}
