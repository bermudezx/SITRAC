#ifndef PASAJEROS_H
#define PASAJEROS_H

#include <iostream>
#include <string>

using namespace std;

// Nodo de la lista enlazada de pasajeros.
struct Pasajero {
    string documento;
    string nombre;
    int edad;
    string telefono;
    Pasajero* siguiente;
};

void registrarPasajero(Pasajero*& inicio);
void listarPasajeros(Pasajero* inicio);
Pasajero* buscarPasajero(Pasajero* inicio, string documento);
void actualizarPasajero(Pasajero* inicio);
void eliminarPasajero(Pasajero*& inicio);
void menuPasajeros(Pasajero*& inicio);

int contarPasajeros(Pasajero* inicio);
void liberarPasajeros(Pasajero*& inicio);
bool insertarPasajeroDirecto(Pasajero*& inicio, const string& documento, const string& nombre, int edad, const string& telefono);

#endif
