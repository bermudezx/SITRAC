#include "ciudades.h"
#include "util.h"

#include <limits>

// Buscar una ciudad por su nombre
Ciudad* buscarCiudad(Ciudad* inicio, string nombre) {
    nombre = trim(nombre);
    Ciudad* actual = inicio;

    while (actual != NULL) {
        if (actual->nombre == nombre) {
            return actual;
        }
        actual = actual->siguiente;
    }

    return NULL;
}

bool insertarCiudadDirecta(Ciudad*& inicio, const string& nombre) {
    string limpio = trim(nombre);

    if (limpio.empty() || contieneSeparador(limpio)) {
        return false;
    }

    if (buscarCiudad(inicio, limpio) != NULL) {
        return false;
    }

    Ciudad* nueva = new Ciudad;
    nueva->nombre = limpio;
    nueva->conexiones = NULL;
    nueva->siguiente = NULL;

    if (inicio == NULL) {
        inicio = nueva;
        return true;
    }

    Ciudad* actual = inicio;
    while (actual->siguiente != NULL) {
        actual = actual->siguiente;
    }
    actual->siguiente = nueva;
    return true;
}

// Registrar una nueva ciudad
void registrarCiudad(Ciudad*& inicio) {
    string nombre = leerTexto("\nIngrese el nombre de la ciudad: ");

    if (nombre.empty()) {
        cout << "El nombre no puede estar vacio.\n";
        return;
    }

    if (contieneSeparador(nombre)) {
        cout << "El nombre no puede contener el caracter |.\n";
        return;
    }

    if (buscarCiudad(inicio, nombre) != NULL) {
        cout << "Error: la ciudad ya existe.\n";
        return;
    }

    // El registro interactivo conserva el orden original: la ciudad nueva queda al inicio.
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
    int numero = 1;

    cout << "\n--- CIUDADES REGISTRADAS ---\n";
    while (actual != NULL) {
        cout << numero << ". " << actual->nombre << endl;
        actual = actual->siguiente;
        numero++;
    }
}

int contarCiudades(Ciudad* inicio) {
    int total = 0;
    while (inicio != NULL) {
        total++;
        inicio = inicio->siguiente;
    }
    return total;
}

int contarRutas(Ciudad* inicio) {
    int total = 0;
    while (inicio != NULL) {
        Conexion* conexion = inicio->conexiones;
        while (conexion != NULL) {
            total++;
            conexion = conexion->siguiente;
        }
        inicio = inicio->siguiente;
    }
    return total;
}

Conexion* buscarConexion(Ciudad* origen, const string& destino) {
    if (origen == NULL) {
        return NULL;
    }

    Conexion* actual = origen->conexiones;
    while (actual != NULL) {
        if (actual->destino == destino) {
            return actual;
        }
        actual = actual->siguiente;
    }
    return NULL;
}

// 0 creada, 1 sin origen, 2 sin destino, 3 misma ciudad, 4 duplicada, 5 distancia invalida
int agregarConexionDatos(Ciudad* inicio, const string& origen, const string& destino, double distancia, bool insertarAlInicio) {
    Ciudad* ciudadOrigen = buscarCiudad(inicio, origen);
    if (ciudadOrigen == NULL) {
        return 1;
    }

    Ciudad* ciudadDestino = buscarCiudad(inicio, destino);
    if (ciudadDestino == NULL) {
        return 2;
    }

    if (ciudadOrigen->nombre == ciudadDestino->nombre) {
        return 3;
    }

    if (buscarConexion(ciudadOrigen, ciudadDestino->nombre) != NULL) {
        return 4;
    }

    if (distancia <= 0) {
        return 5;
    }

    Conexion* nueva = new Conexion;
    nueva->destino = ciudadDestino->nombre;
    nueva->distancia = distancia;
    nueva->siguiente = NULL;

    if (insertarAlInicio || ciudadOrigen->conexiones == NULL) {
        nueva->siguiente = ciudadOrigen->conexiones;
        ciudadOrigen->conexiones = nueva;
    } else {
        Conexion* ultima = ciudadOrigen->conexiones;
        while (ultima->siguiente != NULL) {
            ultima = ultima->siguiente;
        }
        ultima->siguiente = nueva;
    }

    return 0;
}

bool eliminarConexionDatos(Ciudad* inicio, const string& origen, const string& destino) {
    Ciudad* ciudadOrigen = buscarCiudad(inicio, origen);
    if (ciudadOrigen == NULL) {
        return false;
    }

    string destinoLimpio = trim(destino);
    Conexion* actual = ciudadOrigen->conexiones;
    Conexion* anterior = NULL;

    while (actual != NULL && actual->destino != destinoLimpio) {
        anterior = actual;
        actual = actual->siguiente;
    }

    if (actual == NULL) {
        return false;
    }

    if (anterior == NULL) {
        ciudadOrigen->conexiones = actual->siguiente;
    } else {
        anterior->siguiente = actual->siguiente;
    }

    delete actual;
    return true;
}

// 0 eliminada, 1 no hay ciudades, 2 no existe
int eliminarCiudadPorNombre(Ciudad*& inicio, const string& nombreBuscado) {
    string nombre = trim(nombreBuscado);

    if (inicio == NULL) {
        return 1;
    }

    Ciudad* actual = inicio;
    Ciudad* anterior = NULL;

    while (actual != NULL && actual->nombre != nombre) {
        anterior = actual;
        actual = actual->siguiente;
    }

    if (actual == NULL) {
        return 2;
    }

    if (anterior == NULL) {
        inicio = actual->siguiente;
    } else {
        anterior->siguiente = actual->siguiente;
    }

    Conexion* conexion = actual->conexiones;
    while (conexion != NULL) {
        Conexion* temporal = conexion;
        conexion = conexion->siguiente;
        delete temporal;
    }

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
    return 0;
}

// Eliminar una ciudad y las aristas que la mencionan
void eliminarCiudad(Ciudad*& inicio) {
    if (inicio == NULL) {
        cout << "\nNo hay ciudades para eliminar.\n";
        return;
    }

    string nombre = leerTexto("\nIngrese el nombre de la ciudad que desea eliminar: ");
    int resultado = eliminarCiudadPorNombre(inicio, nombre);

    if (resultado == 2) {
        cout << "La ciudad no existe.\n";
        return;
    }

    cout << "Ciudad y sus conexiones eliminadas correctamente.\n";
}

// Agregar una conexion entre dos ciudades
void agregarConexion(Ciudad* inicio) {
    string origen = leerTexto("\nIngrese la ciudad de origen: ");
    string destino = leerTexto("Ingrese la ciudad de destino: ");

    double distancia = 0;
    if (!leerDouble("Ingrese la distancia en kilometros: ", distancia)) {
        return;
    }

    int resultado = agregarConexionDatos(inicio, origen, destino, distancia);
    if (resultado == 0) {
        cout << "Conexion registrada correctamente.\n";
    } else if (resultado == 1) {
        cout << "La ciudad de origen no existe.\n";
    } else if (resultado == 2) {
        cout << "La ciudad de destino no existe.\n";
    } else if (resultado == 3) {
        cout << "Una ciudad no puede conectarse consigo misma.\n";
    } else if (resultado == 4) {
        cout << "Esta conexion ya existe.\n";
    } else {
        cout << "La distancia debe ser mayor que cero.\n";
    }
}

// Consultar las conexiones de una ciudad
void listarConexiones(Ciudad* inicio) {
    string nombre = leerTexto("\nIngrese la ciudad que desea consultar: ");
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
        cout << "- " << actual->destino << ": " << textoDistancia(actual->distancia) << " km\n";
        actual = actual->siguiente;
    }
}

void liberarCiudades(Ciudad*& inicio) {
    while (inicio != NULL) {
        Conexion* conexion = inicio->conexiones;
        while (conexion != NULL) {
            Conexion* temporal = conexion;
            conexion = conexion->siguiente;
            delete temporal;
        }

        Ciudad* temporal = inicio;
        inicio = inicio->siguiente;
        delete temporal;
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
