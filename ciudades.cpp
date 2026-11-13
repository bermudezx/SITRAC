
#include "ciudades.h"
#include <limits>

// Buscar una ciudad por su nombre
Ciudad* buscarCiudad(Ciudad* inicio, string nombre) {
    Ciudad* actual = inicio;

    while (actual != NULL) {
        if (actual->nombre == nombre) {
            return actual;
        }

        actual = actual->siguiente;
    }

    return NULL;
}

// Registrar una nueva ciudad
void registrarCiudad(Ciudad*& inicio) {
    string nombre;

    cout << "\nIngrese el nombre de la ciudad: ";
    getline(cin, nombre);

    if (nombre.empty()) {
        cout << "El nombre no puede estar vacio.\n";
        return;
    }

    if (buscarCiudad(inicio, nombre) != NULL) {
        cout << "Error: la ciudad ya existe.\n";
        return;
    }

    Ciudad* nueva = new Ciudad;

    nueva->nombre = nombre;
    nueva->conexiones = NULL;
    nueva->siguiente = inicio;

    inicio = nueva;

    cout << "Ciudad registrada correctamente.\n";
}

// Mostrar todas las ciudades
void listarCiudades(Ciudad* inicio) {
    if (inicio == NULL) {
        cout << "\nNo hay ciudades registradas.\n";
        return;
    }

    Ciudad* actual = inicio;

    cout << "\n--- CIUDADES REGISTRADAS ---\n";

    while (actual != NULL) {
        cout << "- " << actual->nombre << endl;
        actual = actual->siguiente;
    }
}

// Eliminar una ciudad
void eliminarCiudad(Ciudad*& inicio) {
    if (inicio == NULL) {
        cout << "\nNo hay ciudades para eliminar.\n";
        return;
    }

    string nombre;

    cout << "\nIngrese el nombre de la ciudad que desea eliminar: ";
    getline(cin, nombre);

    Ciudad* actual = inicio;
    Ciudad* anterior = NULL;

    while (actual != NULL && actual->nombre != nombre) {
        anterior = actual;
        actual = actual->siguiente;
    }

    if (actual == NULL) {
        cout << "La ciudad no existe.\n";
        return;
    }

    if (anterior == NULL) {
        inicio = actual->siguiente;
    } else {
        anterior->siguiente = actual->siguiente;
    }

    // Liberar las conexiones de la ciudad eliminada
    Conexion* conexion = actual->conexiones;

    while (conexion != NULL) {
        Conexion* temporal = conexion;
        conexion = conexion->siguiente;
        delete temporal;
    }

    // Eliminar referencias hacia la ciudad desde otras ciudades
    Ciudad* otraCiudad = inicio;

    while (otraCiudad != NULL) {
        Conexion* actualConexion = otraCiudad->conexiones;
        Conexion* anteriorConexion = NULL;

        while (actualConexion != NULL) {
            if (actualConexion->destino == nombre) {
                Conexion* temporal = actualConexion;

                if (anteriorConexion == NULL) {
                    otraCiudad->conexiones = actualConexion->siguiente;
                } else {
                    anteriorConexion->siguiente = actualConexion->siguiente;
                }

                actualConexion = actualConexion->siguiente;
                delete temporal;
            } else {
                anteriorConexion = actualConexion;
                actualConexion = actualConexion->siguiente;
            }
        }

        otraCiudad = otraCiudad->siguiente;
    }

    delete actual;

    cout << "Ciudad y sus conexiones eliminadas correctamente.\n";
}

// Agregar una conexion entre dos ciudades
void agregarConexion(Ciudad* inicio) {
    string origen;
    string destino;
    double distancia;

    cout << "\nIngrese la ciudad de origen: ";
    getline(cin, origen);

    Ciudad* ciudadOrigen = buscarCiudad(inicio, origen);

    if (ciudadOrigen == NULL) {
        cout << "La ciudad de origen no existe.\n";
        return;
    }

    cout << "Ingrese la ciudad de destino: ";
    getline(cin, destino);

    Ciudad* ciudadDestino = buscarCiudad(inicio, destino);

    if (ciudadDestino == NULL) {
        cout << "La ciudad de destino no existe.\n";
        return;
    }

    if (origen == destino) {
        cout << "Una ciudad no puede conectarse consigo misma.\n";
        return;
    }

    // Evitar conexiones duplicadas
    Conexion* existente = ciudadOrigen->conexiones;

    while (existente != NULL) {
        if (existente->destino == destino) {
            cout << "Esta conexion ya existe.\n";
            return;
        }

        existente = existente->siguiente;
    }

    cout << "Ingrese la distancia en kilometros: ";

    if (!(cin >> distancia)) {
        cout << "Debe ingresar un valor numerico.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (distancia <= 0) {
        cout << "La distancia debe ser mayor que cero.\n";
        return;
    }

    Conexion* nueva = new Conexion;

    nueva->destino = destino;
    nueva->distancia = distancia;
    nueva->siguiente = ciudadOrigen->conexiones;

    ciudadOrigen->conexiones = nueva;

    cout << "Conexion registrada correctamente.\n";
}

// Consultar las conexiones de una ciudad
void listarConexiones(Ciudad* inicio) {
    string nombre;

    cout << "\nIngrese la ciudad que desea consultar: ";
    getline(cin, nombre);

    Ciudad* ciudad = buscarCiudad(inicio, nombre);

    if (ciudad == NULL) {
        cout << "La ciudad no existe.\n";
        return;
    }

    cout << "\nConexiones de " << ciudad->nombre << ":\n";

    Conexion* actual = ciudad->conexiones;

    if (actual == NULL) {
        cout << "Esta ciudad no tiene conexiones.\n";
        return;
    }

    while (actual != NULL) {
        cout << "- " << actual->destino
             << ": " << actual->distancia << " km\n";

        actual = actual->siguiente;
    }
}

// Menu del modulo de ciudades
void menuCiudades(Ciudad*& inicio) {
    int opcion;

    do {
        cout << "\n===== MODULO DE CIUDADES =====\n";
        cout << "1. Registrar ciudad\n";
        cout << "2. Listar ciudades\n";
        cout << "3. Eliminar ciudad\n";
        cout << "4. Agregar conexion\n";
        cout << "5. Consultar conexiones\n";
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
                    registrarCiudad(inicio);
                    break;

                case 2:
                    listarCiudades(inicio);
                    break;

                case 3:
                    eliminarCiudad(inicio);
                    break;

                case 4:
                    agregarConexion(inicio);
                    break;

                case 5:
                    listarConexiones(inicio);
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
