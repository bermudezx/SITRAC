#ifndef COLA_H
#define COLA_H

#include "ciudades.h"
#include "historial.h"
#include "pasajeros.h"

// Nodo de la cola FIFO de solicitudes de viaje.
struct Solicitud {
    int id;
    string documento;
    string origen;
    string destino;
    string fecha;
    Solicitud* siguiente;
};

struct Cola {
    Solicitud* frente;
    Solicitud* fin;
    int siguienteId;
};

void inicializarCola(Cola& cola);
void liberarCola(Cola& cola);
int contarCola(const Cola& cola);
bool encolarExistente(Cola& cola, int id, const string& documento, const string& origen,
                      const string& destino, const string& fecha);
void listarCola(const Cola& cola);
void listarColaPorDocumento(const Cola& cola, const string& documento);
void menuCola(Cola& cola, Ciudad* ciudades, Pasajero* pasajeros, Historial*& historial);
struct ResultadoSolicitud {
    bool procesada;
    string resultado;
    string detalle;
    string texto;
};

ResultadoSolicitud procesarSiguienteSolicitud(Cola& cola, Ciudad* ciudades, Pasajero* pasajeros, Historial*& historial, bool cancelar);

#endif
