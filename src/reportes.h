#ifndef REPORTES_H
#define REPORTES_H

#include "cola.h"

void menuReportes(Ciudad* ciudades, Pasajero* pasajeros, const Cola& cola, Historial* historial);
string generarReporteCompleto(Ciudad* ciudades, Pasajero* pasajeros, const Cola& cola, Historial* historial);

#endif
