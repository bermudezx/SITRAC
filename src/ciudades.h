#ifndef CIUDADES_H
#define CIUDADES_H

#include <iostream>
#include <string>

using namespace std;

// Conexion dirigida de una ciudad hacia otra. Forma la lista de adyacencia del grafo.
struct Conexion {
    string destino;
    double distancia;
    Conexion* siguiente;
};

// Nodo de la lista enlazada de ciudades.
struct Ciudad {
    string nombre;
    Conexion* conexiones;
    Ciudad* siguiente;
};

void registrarCiudad(Ciudad*& inicio);
void listarCiudades(Ciudad* inicio);
Ciudad* buscarCiudad(Ciudad* inicio, string nombre);
int eliminarCiudadPorNombre(Ciudad*& inicio, const string& nombre);
void agregarConexion(Ciudad* inicio);
void listarConexiones(Ciudad* inicio);
void menuCiudades(Ciudad*& inicio);

int contarCiudades(Ciudad* inicio);
int contarRutas(Ciudad* inicio);
void liberarCiudades(Ciudad*& inicio);
bool insertarCiudadDirecta(Ciudad*& inicio, const string& nombre);
int agregarConexionDatos(Ciudad* inicio, const string& origen, const string& destino, double distancia, bool insertarAlInicio = true);
bool eliminarConexionDatos(Ciudad* inicio, const string& origen, const string& destino);
Conexion* buscarConexion(Ciudad* origen, const string& destino);

#endif
