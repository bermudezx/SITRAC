#ifndef ARCHIVOS_H
#define ARCHIVOS_H

#include "cola.h"

bool guardarInformacion(Ciudad* ciudades, Pasajero* pasajeros, const Cola& cola, Historial* historial);
bool cargarInformacion(Ciudad*& ciudades, Pasajero*& pasajeros, Cola& cola, Historial*& historial);
void menuArchivos(Ciudad*& ciudades, Pasajero*& pasajeros, Cola& cola, Historial*& historial);

#endif
