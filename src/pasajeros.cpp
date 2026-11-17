#include "pasajeros.h"
#include "util.h"

#include <limits>

// Buscar pasajero por documento
Pasajero* buscarPasajero(Pasajero* inicio, string documento) {
    documento = trim(documento);
    Pasajero* actual = inicio;

    while (actual != NULL) {
        if (actual->documento == documento) {
            return actual;
        }
        actual = actual->siguiente;
    }

    return NULL;
}

bool insertarPasajeroDirecto(Pasajero*& inicio, const string& documento, const string& nombre, int edad, const string& telefono) {
    string doc = trim(documento);
    string nom = trim(nombre);
    string tel = trim(telefono);

    if (doc.empty() || nom.empty() || contieneSeparador(doc) || contieneSeparador(nom) || contieneSeparador(tel)) {
        return false;
    }

    if (edad <= 0 || buscarPasajero(inicio, doc) != NULL) {
        return false;
    }

    Pasajero* nuevo = new Pasajero;
    nuevo->documento = doc;
    nuevo->nombre = nom;
    nuevo->edad = edad;
    nuevo->telefono = tel;
    nuevo->siguiente = NULL;

    if (inicio == NULL) {
        inicio = nuevo;
        return true;
    }

    Pasajero* actual = inicio;
    while (actual->siguiente != NULL) {
        actual = actual->siguiente;
    }
    actual->siguiente = nuevo;
    return true;
}

// Registrar pasajero
void registrarPasajero(Pasajero*& inicio) {
    string documento = leerTexto("\nIngrese el documento: ");

    if (documento.empty()) {
        cout << "El documento no puede estar vacio.\n";
        return;
    }

    if (contieneSeparador(documento)) {
        cout << "El documento no puede contener el caracter |.\n";
        return;
    }

    if (buscarPasajero(inicio, documento) != NULL) {
        cout << "Error: el documento ya esta registrado.\n";
        return;
    }

    string nombre = leerTexto("Ingrese el nombre completo: ");
    if (nombre.empty()) {
        cout << "El nombre no puede estar vacio.\n";
        return;
    }

    if (contieneSeparador(nombre)) {
        cout << "El nombre no puede contener el caracter |.\n";
        return;
    }

    int edad = 0;
    if (!leerEntero("Ingrese la edad: ", edad)) {
        return;
    }

    if (edad <= 0) {
        cout << "La edad debe ser mayor que cero.\n";
        return;
    }

    string telefono = leerTexto("Ingrese el telefono: ");
    if (contieneSeparador(telefono)) {
        cout << "El telefono no puede contener el caracter |.\n";
        return;
    }

    // Igual que el modulo original: el pasajero nuevo queda al inicio de la lista.
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

int contarPasajeros(Pasajero* inicio) {
    int total = 0;
    while (inicio != NULL) {
        total++;
        inicio = inicio->siguiente;
    }
    return total;
}

// Actualizar pasajero. Los cambios se aplican solo si todos los datos son validos.
void actualizarPasajero(Pasajero* inicio) {
    string documento = leerTexto("\nIngrese el documento del pasajero: ");
    Pasajero* actual = buscarPasajero(inicio, documento);

    if (actual == NULL) {
        cout << "El pasajero no existe.\n";
        return;
    }

    cout << "Nombre actual: " << actual->nombre << endl;
    string nombre = leerTexto("Nuevo nombre: ");
    if (nombre.empty()) {
        cout << "El nombre no puede estar vacio.\n";
        return;
    }
    if (contieneSeparador(nombre)) {
        cout << "El nombre no puede contener el caracter |.\n";
        return;
    }

    int edad = 0;
    if (!leerEntero("Nueva edad: ", edad)) {
        return;
    }
    if (edad <= 0) {
        cout << "La edad debe ser mayor que cero.\n";
        return;
    }

    string telefono = leerTexto("Nuevo telefono: ");
    if (contieneSeparador(telefono)) {
        cout << "El telefono no puede contener el caracter |.\n";
        return;
    }

    actual->nombre = nombre;
    actual->edad = edad;
    actual->telefono = telefono;
    cout << "Pasajero actualizado correctamente.\n";
}

// Eliminar pasajero
void eliminarPasajero(Pasajero*& inicio) {
    if (inicio == NULL) {
        cout << "\nNo hay pasajeros para eliminar.\n";
        return;
    }

    string documento = leerTexto("\nIngrese el documento del pasajero: ");
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

void liberarPasajeros(Pasajero*& inicio) {
    while (inicio != NULL) {
        Pasajero* temporal = inicio;
        inicio = inicio->siguiente;
        delete temporal;
    }
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
                    string documento = leerTexto("Ingrese el documento: ");
                    Pasajero* encontrado = buscarPasajero(inicio, documento);

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
