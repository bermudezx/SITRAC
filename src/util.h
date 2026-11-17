#ifndef UTIL_H
#define UTIL_H

#include <string>

std::string trim(const std::string& texto);
bool contieneSeparador(const std::string& texto);
bool leerEntero(const std::string& mensaje, int& valor);
bool leerDouble(const std::string& mensaje, double& valor);
std::string leerTexto(const std::string& mensaje);
int separarCampos(const std::string& linea, std::string campos[], int maxCampos);
std::string textoDistancia(double distancia);
bool crearCarpetaDatos(const char* ruta);

#endif
