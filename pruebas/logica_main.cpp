#include <iostream>
#include <string>

#include "../src/archivos.h"
#include "../src/pasajeros.h"
#include "../src/reportes.h"
#include "../src/rutas.h"
#include "../src/util.h"

static int errores = 0;

static void esperar(bool condicion, const std::string& mensaje) {
    if (!condicion) {
        std::cout << "FALLO: " << mensaje << std::endl;
        errores++;
    }
}

int main() {
    Ciudad* ciudades = NULL;
    Pasajero* pasajeros = NULL;
    Cola cola;
    Historial* historial = NULL;
    inicializarCola(cola);

    esperar(insertarCiudadDirecta(ciudades, "Bogota"), "crear Bogota");
    esperar(insertarCiudadDirecta(ciudades, "Medellin"), "crear Medellin");
    esperar(insertarCiudadDirecta(ciudades, "Cali"), "crear Cali");
    esperar(insertarCiudadDirecta(ciudades, "Pereira"), "crear Pereira");
    esperar(!insertarCiudadDirecta(ciudades, "Bogota"), "no duplicar ciudad");
    esperar(agregarConexionDatos(ciudades, "Bogota", "Medellin", 415, false) == 0, "ruta Bogota Medellin");
    esperar(agregarConexionDatos(ciudades, "Bogota", "Medellin", 100, false) == 4, "ruta duplicada");
    esperar(agregarConexionDatos(ciudades, "Medellin", "Cali", 420, false) == 0, "ruta Medellin Cali");
    esperar(agregarConexionDatos(ciudades, "Bogota", "Pereira", 300, false) == 0, "ruta Bogota Pereira");
    esperar(agregarConexionDatos(ciudades, "Pereira", "Bogota", 300, false) == 0, "ruta Pereira Bogota");
    esperar(agregarConexionDatos(ciudades, "Pereira", "Cali", 200, false) == 0, "ruta Pereira Cali");

    ResultadoCamino camino = calcularCaminoMasCorto(ciudades, "Bogota", "Cali");
    esperar(camino.encontrado, "existe camino");
    esperar(camino.recorrido == "Bogota -> Pereira -> Cali", "recorrido optimo");
    esperar(camino.distancia > 499.9 && camino.distancia < 500.1, "distancia 500");

    esperar(eliminarConexionDatos(ciudades, "Medellin", "Cali"), "eliminar ruta");
    esperar(buscarConexion(buscarCiudad(ciudades, "Medellin"), "Cali") == NULL, "ruta eliminada");

    esperar(insertarPasajeroDirecto(pasajeros, "1001", "Ana Perez", 22, "3001112233"), "pasajero Ana");
    esperar(insertarPasajeroDirecto(pasajeros, "1002", "Luis Gomez", 17, "3015556677"), "pasajero Luis");
    esperar(!insertarPasajeroDirecto(pasajeros, "1001", "Otra", 30, "1"), "documento duplicado");
    esperar(actualizarPasajeroDatos(pasajeros, "1002", "Luis Gomez Actualizado", 18, "3010000000"), "actualizar");
    esperar(buscarPasajero(pasajeros, "1002")->edad == 18, "edad actualizada");
    esperar(insertarPasajeroDirecto(pasajeros, "1003", "Temporal Uno", 30, "300000"), "temporal");
    esperar(eliminarPasajeroPorDocumento(pasajeros, "1003"), "eliminar temporal");
    esperar(buscarPasajero(pasajeros, "1003") == NULL, "temporal ya no existe");

    esperar(encolarExistente(cola, 1, "1001", "Bogota", "Cali", "2026-11-20"), "solicitud 1");
    esperar(encolarExistente(cola, 2, "1002", "Pereira", "Medellin", "2026-11-21"), "solicitud 2");
    esperar(contarCola(cola) == 2, "cola con dos");

    ResultadoSolicitud atendida = procesarSiguienteSolicitud(cola, ciudades, pasajeros, historial, false);
    esperar(atendida.procesada && atendida.resultado == "ATENDIDA", "primera atendida");
    esperar(atendida.detalle == "Bogota -> Pereira -> Cali", "detalle del camino");

    ResultadoSolicitud segunda = procesarSiguienteSolicitud(cola, ciudades, pasajeros, historial, false);
    esperar(segunda.procesada && segunda.resultado == "ATENDIDA", "segunda atendida");
    esperar(segunda.detalle == "Pereira -> Bogota -> Medellin", "camino de regreso");

    esperar(insertarCiudadDirecta(ciudades, "Cartagena"), "Cartagena");
    esperar(encolarExistente(cola, 3, "1001", "Bogota", "Cartagena", "2026-11-22"), "solicitud sin camino");
    ResultadoSolicitud rechazada = procesarSiguienteSolicitud(cola, ciudades, pasajeros, historial, false);
    esperar(rechazada.resultado == "RECHAZADA", "solicitud rechazada");
    esperar(contarHistorial(historial) == 3, "tres registros");
    esperar(contarHistorialPorResultado(historial, "ATENDIDA") == 2, "dos atendidas");
    esperar(contarCola(cola) == 0, "cola vacia");

    std::string reporte = generarReporteCompleto(ciudades, pasajeros, cola, historial);
    esperar(reporte.find("Ciudades: 5") != std::string::npos, "reporte de ciudades");
    esperar(reporte.find("Adultos: 2") != std::string::npos, "reporte de edades");
    esperar(reporte.find("Pereira") != std::string::npos, "ciudad conectada");

    esperar(eliminarCiudadPorNombre(ciudades, "Cartagena") == 0, "eliminar Cartagena");
    esperar(buscarCiudad(ciudades, "Cartagena") == NULL, "Cartagena eliminada");

    establecerCarpetaDatos("/tmp/sitrac-gui-datos");
    esperar(guardarInformacion(ciudades, pasajeros, cola, historial), "guardar");

    Ciudad* cargadas = NULL;
    Pasajero* pasajerosCargados = NULL;
    Cola colaCargada;
    Historial* historialCargado = NULL;
    inicializarCola(colaCargada);
    esperar(cargarInformacion(cargadas, pasajerosCargados, colaCargada, historialCargado), "cargar");
    esperar(contarCiudades(cargadas) == 4, "ciudades cargadas");
    esperar(buscarPasajero(pasajerosCargados, "1002")->nombre == "Luis Gomez Actualizado", "nombre cargado");
    esperar(contarHistorial(historialCargado) == 3, "historial cargado");
    esperar(contarCola(colaCargada) == 0, "cola cargada vacia");

    liberarCiudades(ciudades);
    liberarPasajeros(pasajeros);
    liberarCola(cola);
    liberarHistorial(historial);
    liberarCiudades(cargadas);
    liberarPasajeros(pasajerosCargados);
    liberarCola(colaCargada);
    liberarHistorial(historialCargado);

    if (errores == 0) {
        std::cout << "Pruebas de logica superadas." << std::endl;
        return 0;
    }
    std::cout << "Errores: " << errores << std::endl;
    return 1;
}
