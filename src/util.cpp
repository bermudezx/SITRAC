#include "util.h"

#include <cerrno>
#include <cstdio>
#include <iostream>
#include <limits>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

using namespace std;

string trim(const string& texto) {
    size_t inicio = 0;
    while (inicio < texto.size()) {
        char c = texto[inicio];
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
            break;
        }
        inicio++;
    }

    size_t fin = texto.size();
    while (fin > inicio) {
        char c = texto[fin - 1];
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
            break;
        }
        fin--;
    }

    return texto.substr(inicio, fin - inicio);
}

bool contieneSeparador(const string& texto) {
    return texto.find('|') != string::npos;
}

bool leerEntero(const string& mensaje, int& valor) {
    cout << mensaje;
    if (!(cin >> valor)) {
        cout << "Debe ingresar un numero entero.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return true;
}

bool leerDouble(const string& mensaje, double& valor) {
    cout << mensaje;
    if (!(cin >> valor)) {
        cout << "Debe ingresar un valor numerico.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return true;
}

string leerTexto(const string& mensaje) {
    cout << mensaje;
    string linea;
    getline(cin, linea);
    return trim(linea);
}

int separarCampos(const string& linea, string campos[], int maxCampos) {
    int cantidad = 0;
    string actual = "";

    for (size_t i = 0; i < linea.size(); i++) {
        if (linea[i] == '|') {
            if (cantidad < maxCampos) {
                campos[cantidad] = trim(actual);
            }
            cantidad++;
            actual = "";
        } else if (linea[i] != '\r') {
            actual += linea[i];
        }
    }

    if (cantidad < maxCampos) {
        campos[cantidad] = trim(actual);
    }
    cantidad++;
    return cantidad;
}

string textoDistancia(double distancia) {
    char buffer[64];
    sprintf(buffer, "%.2f", distancia);
    return string(buffer);
}

bool crearCarpetaDatos(const char* ruta) {
#ifdef _WIN32
    if (_mkdir(ruta) == 0 || errno == EEXIST) {
        return true;
    }
#else
    if (mkdir(ruta, 0755) == 0 || errno == EEXIST) {
        return true;
    }
#endif
    return false;
}
