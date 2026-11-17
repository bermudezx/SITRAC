#ifndef RUTAS_H
#define RUTAS_H

#include "ciudades.h"

// Resultado de un recorrido sobre el grafo de ciudades.
struct ResultadoCamino {
    bool encontrado;
    double distancia;
    string recorrido;
};

void menuRutas(Ciudad* inicio);
ResultadoCamino calcularCaminoMasCorto(Ciudad* inicio, const string& origen, const string& destino);
void listarTodasLasRutas(Ciudad* inicio);

#endif
