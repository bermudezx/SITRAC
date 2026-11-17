#ifndef HISTORIAL_H
#define HISTORIAL_H

#include <iostream>
#include <string>

using namespace std;

// Registro historico. La lista se recorre del mas antiguo al mas reciente.
struct Historial {
    int idSolicitud;
    string documento;
    string nombrePasajero;
    string origen;
    string destino;
    double distancia;
    string fecha;
    string resultado;
    Historial* siguiente;
};

void agregarHistorial(Historial*& inicio, int idSolicitud, const string& documento,
                      const string& nombrePasajero, const string& origen,
                      const string& destino, double distancia, const string& fecha,
                      const string& resultado);
void listarHistorial(Historial* inicio);
void listarHistorialPorDocumento(Historial* inicio, const string& documento);
int contarHistorial(Historial* inicio);
int contarHistorialPorResultado(Historial* inicio, const string& resultado);
void liberarHistorial(Historial*& inicio);
void menuHistorial(Historial* inicio);

#endif
