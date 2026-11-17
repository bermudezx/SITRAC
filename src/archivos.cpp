#include "archivos.h"
#include "util.h"

#include <fstream>
#include <limits>
#include <sstream>

static const char* CARPETA = "datos";
static const char* ARCHIVO_CIUDADES = "datos/ciudades.txt";
static const char* ARCHIVO_RUTAS = "datos/rutas.txt";
static const char* ARCHIVO_PASAJEROS = "datos/pasajeros.txt";
static const char* ARCHIVO_COLA = "datos/cola.txt";
static const char* ARCHIVO_HISTORIAL = "datos/historial.txt";

static bool esComentario(const string& linea) {
    return linea.empty() || linea[0] == '#';
}

static void escribirError(const string& archivo) {
    cout << "No se pudo escribir " << archivo << ".\n";
}

bool guardarInformacion(Ciudad* ciudades, Pasajero* pasajeros, const Cola& cola, Historial* historial) {
    if (!crearCarpetaDatos(CARPETA)) {
        cout << "No se pudo crear la carpeta datos.\n";
        return false;
    }

    ofstream ciudadesOut(ARCHIVO_CIUDADES);
    ofstream rutasOut(ARCHIVO_RUTAS);
    ofstream pasajerosOut(ARCHIVO_PASAJEROS);
    ofstream colaOut(ARCHIVO_COLA);
    ofstream historialOut(ARCHIVO_HISTORIAL);

    if (!ciudadesOut || !rutasOut || !pasajerosOut || !colaOut || !historialOut) {
        cout << "No se pudieron abrir los archivos de salida.\n";
        return false;
    }

    ciudadesOut << "# Una ciudad por linea\n";
    rutasOut << "# origen|destino|distancia\n";
    pasajerosOut << "# documento|nombre|edad|telefono\n";
    colaOut << "#SIGUIENTE " << cola.siguienteId << "\n";
    historialOut << "# id|documento|nombre|origen|destino|distancia|fecha|resultado\n";

    Ciudad* ciudad = ciudades;
    while (ciudad != NULL) {
        ciudadesOut << ciudad->nombre << "\n";
        Conexion* conexion = ciudad->conexiones;
        while (conexion != NULL) {
            rutasOut << ciudad->nombre << "|" << conexion->destino << "|"
                     << textoDistancia(conexion->distancia) << "\n";
            conexion = conexion->siguiente;
        }
        ciudad = ciudad->siguiente;
    }

    // Se recorre la lista tal como esta en memoria para poder recuperarla igual.
    Pasajero* pasajero = pasajeros;
    while (pasajero != NULL) {
        pasajerosOut << pasajero->documento << "|" << pasajero->nombre << "|"
                     << pasajero->edad << "|" << pasajero->telefono << "\n";
        pasajero = pasajero->siguiente;
    }

    Solicitud* solicitud = cola.frente;
    while (solicitud != NULL) {
        colaOut << solicitud->id << "|" << solicitud->documento << "|" << solicitud->origen
                << "|" << solicitud->destino << "|" << solicitud->fecha << "\n";
        solicitud = solicitud->siguiente;
    }

    Historial* registro = historial;
    while (registro != NULL) {
        historialOut << registro->idSolicitud << "|" << registro->documento << "|"
                     << registro->nombrePasajero << "|" << registro->origen << "|"
                     << registro->destino << "|" << textoDistancia(registro->distancia) << "|"
                     << registro->fecha << "|" << registro->resultado << "\n";
        registro = registro->siguiente;
    }

    if (!ciudadesOut || !rutasOut || !pasajerosOut || !colaOut || !historialOut) {
        escribirError(CARPETA);
        return false;
    }

    cout << "Informacion guardada correctamente en la carpeta datos.\n";
    cout << "Ciudades: " << contarCiudades(ciudades)
         << " | Rutas: " << contarRutas(ciudades)
         << " | Pasajeros: " << contarPasajeros(pasajeros)
         << " | Pendientes: " << contarCola(cola)
         << " | Historial: " << contarHistorial(historial) << endl;
    return true;
}

static int leerEnteroTexto(const string& texto, bool& ok) {
    stringstream flujo(texto);
    int valor = 0;
    char sobrante = 0;
    ok = (bool)(flujo >> valor) && !(flujo >> sobrante);
    return valor;
}

static double leerDoubleTexto(const string& texto, bool& ok) {
    stringstream flujo(texto);
    double valor = 0;
    char sobrante = 0;
    ok = (bool)(flujo >> valor) && !(flujo >> sobrante);
    return valor;
}

bool cargarInformacion(Ciudad*& ciudades, Pasajero*& pasajeros, Cola& cola, Historial*& historial) {
    ifstream ciudadesIn(ARCHIVO_CIUDADES);
    ifstream rutasIn(ARCHIVO_RUTAS);
    ifstream pasajerosIn(ARCHIVO_PASAJEROS);
    ifstream colaIn(ARCHIVO_COLA);
    ifstream historialIn(ARCHIVO_HISTORIAL);

    if (!ciudadesIn || !rutasIn || !pasajerosIn || !colaIn || !historialIn) {
        cout << "No se encontro informacion completa en la carpeta datos.\n";
        cout << "Guarde el sistema al menos una vez o conserve los archivos de ejemplo.\n";
        return false;
    }

    liberarCiudades(ciudades);
    liberarPasajeros(pasajeros);
    liberarCola(cola);
    liberarHistorial(historial);
    inicializarCola(cola);

    string linea;
    int avisos = 0;

    while (getline(ciudadesIn, linea)) {
        linea = trim(linea);
        if (esComentario(linea)) {
            continue;
        }
        if (!insertarCiudadDirecta(ciudades, linea)) {
            cout << "Se omitio una ciudad invalida o repetida: " << linea << endl;
            avisos++;
        }
    }

    while (getline(rutasIn, linea)) {
        linea = trim(linea);
        if (esComentario(linea)) {
            continue;
        }

        string campos[3];
        if (separarCampos(linea, campos, 3) != 3) {
            cout << "Ruta con formato invalido: " << linea << endl;
            avisos++;
            continue;
        }

        bool ok = false;
        double distancia = leerDoubleTexto(campos[2], ok);
        if (!ok || agregarConexionDatos(ciudades, campos[0], campos[1], distancia, false) != 0) {
            cout << "Se omitio la ruta: " << linea << endl;
            avisos++;
        }
    }

    while (getline(pasajerosIn, linea)) {
        linea = trim(linea);
        if (esComentario(linea)) {
            continue;
        }

        string campos[4];
        if (separarCampos(linea, campos, 4) != 4) {
            cout << "Pasajero con formato invalido: " << linea << endl;
            avisos++;
            continue;
        }

        bool ok = false;
        int edad = leerEnteroTexto(campos[2], ok);
        if (!ok || !insertarPasajeroDirecto(pasajeros, campos[0], campos[1], edad, campos[3])) {
            cout << "Se omitio el pasajero: " << linea << endl;
            avisos++;
        }
    }

    int siguienteArchivo = 1;
    while (getline(colaIn, linea)) {
        linea = trim(linea);
        if (linea.empty()) {
            continue;
        }

        if (linea.size() >= 11 && linea.substr(0, 11) == "#SIGUIENTE ") {
            bool ok = false;
            int valor = leerEnteroTexto(trim(linea.substr(11)), ok);
            if (ok && valor > siguienteArchivo) {
                siguienteArchivo = valor;
            }
            continue;
        }

        if (linea[0] == '#') {
            continue;
        }

        string campos[5];
        if (separarCampos(linea, campos, 5) != 5) {
            cout << "Solicitud con formato invalido: " << linea << endl;
            avisos++;
            continue;
        }

        bool ok = false;
        int id = leerEnteroTexto(campos[0], ok);
        if (!ok || !encolarExistente(cola, id, campos[1], campos[2], campos[3], campos[4])) {
            cout << "Se omitio la solicitud: " << linea << endl;
            avisos++;
        }
    }

    if (cola.siguienteId < siguienteArchivo) {
        cola.siguienteId = siguienteArchivo;
    }

    while (getline(historialIn, linea)) {
        linea = trim(linea);
        if (esComentario(linea)) {
            continue;
        }

        string campos[8];
        if (separarCampos(linea, campos, 8) != 8) {
            cout << "Historial con formato invalido: " << linea << endl;
            avisos++;
            continue;
        }

        bool okId = false;
        bool okDistancia = false;
        int id = leerEnteroTexto(campos[0], okId);
        double distancia = leerDoubleTexto(campos[5], okDistancia);
        if (!okId || !okDistancia) {
            cout << "Se omitio el historial: " << linea << endl;
            avisos++;
            continue;
        }

        agregarHistorial(historial, id, campos[1], campos[2], campos[3], campos[4],
                         distancia, campos[6], campos[7]);
        if (id >= cola.siguienteId) {
            cola.siguienteId = id + 1;
        }
    }

    cout << "Informacion cargada correctamente desde la carpeta datos.\n";
    cout << "Ciudades: " << contarCiudades(ciudades)
         << " | Rutas: " << contarRutas(ciudades)
         << " | Pasajeros: " << contarPasajeros(pasajeros)
         << " | Pendientes: " << contarCola(cola)
         << " | Historial: " << contarHistorial(historial) << endl;
    if (avisos > 0) {
        cout << "Avisos durante la carga: " << avisos << endl;
    }
    return true;
}

static void mostrarArchivos() {
    cout << "\nLos archivos de SITRAC se guardan junto al programa, en la carpeta datos:\n";
    cout << "- " << ARCHIVO_CIUDADES << endl;
    cout << "- " << ARCHIVO_RUTAS << endl;
    cout << "- " << ARCHIVO_PASAJEROS << endl;
    cout << "- " << ARCHIVO_COLA << endl;
    cout << "- " << ARCHIVO_HISTORIAL << endl;
    cout << "Ejecute el programa desde la carpeta del proyecto para encontrarlos.\n";
}

void menuArchivos(Ciudad*& ciudades, Pasajero*& pasajeros, Cola& cola, Historial*& historial) {
    int opcion;

    do {
        cout << "\n===== GUARDAR / CARGAR =====\n";
        cout << "1. Guardar toda la informacion\n";
        cout << "2. Cargar informacion\n";
        cout << "3. Ver ubicacion de los archivos\n";
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
                    guardarInformacion(ciudades, pasajeros, cola, historial);
                    break;
                case 2:
                    cargarInformacion(ciudades, pasajeros, cola, historial);
                    break;
                case 3:
                    mostrarArchivos();
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
