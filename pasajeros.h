
#ifndef PASAJEROS_H
#define PASAJEROS_H

#include <iostream>
#include <string>

using namespace std;

// Estructura de un pasajero
struct Pasajero {
    string documento;
    string nombre;
    int edad;
    string telefono;
    Pasajero* siguiente;
};

// Funciones del modulo
void registrarPasajero(Pasajero*& inicio);
void listarPasajeros(Pasajero* inicio);
Pasajero* buscarPasajero(Pasajero* inicio, string documento);
void actualizarPasajero(Pasajero* inicio);
void eliminarPasajero(Pasajero*& inicio);
void menuPasajeros(Pasajero*& inicio);

#endif
