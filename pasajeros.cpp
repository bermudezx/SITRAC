
#include "pasajeros.h"
#include <limits>

// Buscar pasajero por documento
Pasajero* buscarPasajero(Pasajero* inicio, string documento) {
    Pasajero* actual = inicio;

    while (actual != NULL) {
        if (actual->documento == documento) {
            return actual;
        }

        actual = actual->siguiente;
    }

    return NULL;
}

// Registrar pasajero
void registrarPasajero(Pasajero*& inicio) {
    string documento;
    string nombre;
    string telefono;
    int edad;

    cout << "\nIngrese el documento: ";
    getline(cin, documento);

    if (documento.empty()) {
        cout << "El documento no puede estar vacio.\n";
        return;
    }

    if (buscarPasajero(inicio, documento) != NULL) {
        cout << "Error: el documento ya esta registrado.\n";
        return;
    }

    cout << "Ingrese el nombre completo: ";
    getline(cin, nombre);

    cout << "Ingrese la edad: ";

    if (!(cin >> edad)) {
        cout << "Debe ingresar una edad numerica.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (edad <= 0) {
        cout << "La edad debe ser mayor que cero.\n";
        return;
    }

    cout << "Ingrese el telefono: ";
    getline(cin, telefono);

    Pasajero* nuevo = new Pasajero;

    nuevo->documento = documento;
    nuevo->nombre = nombre;
    nuevo->edad = edad;
    nuevo->telefono = telefono;
    nuevo->siguiente = inicio;

    inicio = nuevo;

    cout << "Pasajero registrado correctamente.\n";
}

// Listar pasajeros
void listarPasajeros(Pasajero* inicio) {
    if (inicio == NULL) {
        cout << "\nNo hay pasajeros registrados.\n";
        return;
    }

    Pasajero* actual = inicio;

    cout << "\n===== LISTA DE PASAJEROS =====\n";

    while (actual != NULL) {
        cout << "\nDocumento: " << actual->documento << endl;
        cout << "Nombre: " << actual->nombre << endl;
        cout << "Edad: " << actual->edad << endl;
        cout << "Telefono: " << actual->telefono << endl;

        actual = actual->siguiente;
    }
}

// Actualizar pasajero
void actualizarPasajero(Pasajero* inicio) {
    string documento;

    cout << "\nIngrese el documento del pasajero: ";
    getline(cin, documento);

    Pasajero* actual = buscarPasajero(inicio, documento);

    if (actual == NULL) {
        cout << "El pasajero no existe.\n";
        return;
    }

    cout << "Nuevo nombre: ";
    getline(cin, actual->nombre);

    cout << "Nueva edad: ";

    if (!(cin >> actual->edad)) {
        cout << "Debe ingresar una edad numerica.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (actual->edad <= 0) {
        cout << "La edad debe ser mayor que cero.\n";
        return;
    }

    cout << "Nuevo telefono: ";
    getline(cin, actual->telefono);

    cout << "Pasajero actualizado correctamente.\n";
}

// Eliminar pasajero
void eliminarPasajero(Pasajero*& inicio) {
    string documento;

    cout << "\nIngrese el documento del pasajero: ";
    getline(cin, documento);

    Pasajero* actual = inicio;
    Pasajero* anterior = NULL;

    while (actual != NULL && actual->documento != documento) {
        anterior = actual;
        actual = actual->siguiente;
    }

    if (actual == NULL) {
        cout << "El pasajero no existe.\n";
        return;
    }

    if (anterior == NULL) {
        inicio = actual->siguiente;
    } else {
        anterior->siguiente = actual->siguiente;
    }

    delete actual;

    cout << "Pasajero eliminado correctamente.\n";
}

// Menu de pasajeros
void menuPasajeros(Pasajero*& inicio) {
    int opcion;

    do {
        cout << "\n===== MODULO DE PASAJEROS =====\n";
        cout << "1. Registrar pasajero\n";
        cout << "2. Listar pasajeros\n";
        cout << "3. Buscar pasajero\n";
        cout << "4. Actualizar pasajero\n";
        cout << "5. Eliminar pasajero\n";
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
                    registrarPasajero(inicio);
                    break;

                case 2:
                    listarPasajeros(inicio);
                    break;

                case 3: {
                    string documento;

                    cout << "Ingrese el documento: ";
                    getline(cin, documento);

                    Pasajero* encontrado =
                        buscarPasajero(inicio, documento);

                    if (encontrado != NULL) {
                        cout << "Nombre: " << encontrado->nombre << endl;
                        cout << "Edad: " << encontrado->edad << endl;
                        cout << "Telefono: " << encontrado->telefono << endl;
                    } else {
                        cout << "Pasajero no encontrado.\n";
                    }

                    break;
                }

                case 4:
                    actualizarPasajero(inicio);
                    break;

                case 5:
                    eliminarPasajero(inicio);
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
