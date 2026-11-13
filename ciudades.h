
#ifndef CIUDADES_H
#define CIUDADES_H

#include <iostream>
#include <string>

using namespace std;

// Estructura de una conexion entre ciudades
struct Conexion {
    string destino;
    double distancia;
    Conexion* siguiente;
};

// Estructura de una ciudad
struct Ciudad {
    string nombre;
    Conexion* conexiones;
    Ciudad* siguiente;
};

// Funciones del modulo
void registrarCiudad(Ciudad*& inicio);
void listarCiudades(Ciudad* inicio);
Ciudad* buscarCiudad(Ciudad* inicio, string nombre);
void eliminarCiudad(Ciudad*& inicio);
void agregarConexion(Ciudad* inicio);
void listarConexiones(Ciudad* inicio);
void menuCiudades(Ciudad*& inicio);

#endif
