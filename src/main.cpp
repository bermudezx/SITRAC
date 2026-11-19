#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <commctrl.h>

#include <cstdio>
#include <sstream>
#include <string>

#include "archivos.h"
#include "pasajeros.h"
#include "reportes.h"
#include "rutas.h"
#include "util.h"

#ifndef SB_SETTEXTW
#define SB_SETTEXTW (WM_USER + 11)
#endif

#ifndef LVS_EX_DOUBLEBUFFER
#define LVS_EX_DOUBLEBUFFER 0x00010000
#endif

#ifndef EM_SETCUEBANNER
#define EM_SETCUEBANNER 0x1501
#endif

namespace {

enum {
    TAB_CIU = 0,
    TAB_RUT,
    TAB_PAS,
    TAB_COL,
    TAB_HIS,
    TAB_BUS,
    TAB_REP,
    TAB_ARC
};

enum {
    IDC_TAB = 1000,
    IDC_LISTA = 1001,
    IDC_LISTA2 = 1002,
    IDC_RESULTADO = 1003,
    IDC_CIU_NOMBRE = 1101,
    IDC_CIU_REG = 1102,
    IDC_CIU_ELI = 1103,
    IDC_CIU_ORI = 1104,
    IDC_CIU_DES = 1105,
    IDC_CIU_KM = 1106,
    IDC_CIU_ADD = 1107,
    IDC_CIU_QUIT = 1108,
    IDC_RUT_ORI = 1201,
    IDC_RUT_DES = 1202,
    IDC_RUT_KM = 1203,
    IDC_RUT_IDA = 1204,
    IDC_RUT_REG = 1205,
    IDC_RUT_ELI = 1206,
    IDC_RUT_CAM = 1207,
    IDC_RUT_RES = 1208,
    IDC_PAS_DOC = 1301,
    IDC_PAS_NOM = 1302,
    IDC_PAS_EDAD = 1303,
    IDC_PAS_TEL = 1304,
    IDC_PAS_REG = 1305,
    IDC_PAS_ACT = 1306,
    IDC_PAS_ELI = 1307,
    IDC_COL_DOC = 1401,
    IDC_COL_ORI = 1402,
    IDC_COL_DES = 1403,
    IDC_COL_FEC = 1404,
    IDC_COL_ADD = 1405,
    IDC_COL_ATE = 1406,
    IDC_COL_CAN = 1407,
    IDC_HIS_DOC = 1501,
    IDC_HIS_FIL = 1502,
    IDC_HIS_TODO = 1503,
    IDC_BUS_TIPO = 1601,
    IDC_BUS_A = 1602,
    IDC_BUS_B = 1603,
    IDC_BUS_GO = 1604,
    IDC_REP_ACT = 1701,
    IDC_ARC_GUA = 1801,
    IDC_ARC_CAR = 1802
};

struct Pieza {
    HWND hwnd;
    int tab;
};

HINSTANCE gInst = NULL;
HWND gVentana = NULL;
HWND gEncabezado = NULL;
HWND gTabs = NULL;
HWND gLista = NULL;
HWND gLista2 = NULL;
HWND gResultado = NULL;
HWND gEstado = NULL;
HWND gLblCiudad = NULL;
HWND gLblConexion = NULL;
HWND gLblRuta = NULL;
HWND gLblPasajero = NULL;
HWND gLblCola = NULL;
HFONT gFuente = NULL;
HFONT gFuenteTitulo = NULL;
int gPestana = TAB_CIU;
bool gCargando = false;
string gFiltroHistorial = "";

Ciudad* gCiudades = NULL;
Pasajero* gPasajeros = NULL;
Cola gCola;
Historial* gHistorial = NULL;

Pieza gPiezas[80];
int gNPiezas = 0;

HWND anotar(HWND hwnd, int tab) {
    if (hwnd != NULL && gNPiezas < 80) {
        gPiezas[gNPiezas].hwnd = hwnd;
        gPiezas[gNPiezas].tab = tab;
        gNPiezas++;
    }
    return hwnd;
}

wstring ampliar(const string& texto) {
    if (texto.empty()) {
        return L"";
    }
    int n = MultiByteToWideChar(CP_UTF8, 0, texto.c_str(), (int)texto.size(), NULL, 0);
    if (n <= 0) {
        return L"";
    }
    wstring salida(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, texto.c_str(), (int)texto.size(), &salida[0], n);
    return salida;
}

string reducir(const wstring& texto) {
    if (texto.empty()) {
        return "";
    }
    int n = WideCharToMultiByte(CP_UTF8, 0, texto.c_str(), (int)texto.size(), NULL, 0, NULL, NULL);
    if (n <= 0) {
        return "";
    }
    string salida(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, texto.c_str(), (int)texto.size(), &salida[0], n, NULL, NULL);
    return salida;
}

string leerCaja(HWND hwnd) {
    wchar_t buffer[256];
    GetWindowTextW(hwnd, buffer, 256);
    return trim(reducir(buffer));
}

void ponerTexto(HWND hwnd, const wstring& texto) {
    SetWindowTextW(hwnd, texto.c_str());
}

void ponerTexto(HWND hwnd, const string& texto) {
    ponerTexto(hwnd, ampliar(texto));
}

void estado(const wstring& texto) {
    SendMessageW(gEstado, SB_SETTEXTW, 0, (LPARAM)texto.c_str());
}

void estado(const string& texto) {
    estado(ampliar(texto));
}

void avisar(const wchar_t* titulo, const string& mensaje, bool error) {
    MessageBoxW(gVentana, ampliar(mensaje).c_str(), titulo, MB_OK | (error ? MB_ICONWARNING : MB_ICONINFORMATION));
}

bool leerDoubleTexto(const string& texto, double& valor) {
    stringstream flujo(texto);
    char sobrante = 0;
    return (flujo >> valor) && !(flujo >> sobrante);
}

bool leerEnteroTexto(const string& texto, int& valor) {
    stringstream flujo(texto);
    char sobrante = 0;
    return (flujo >> valor) && !(flujo >> sobrante);
}

string enteroATexto(int valor) {
    char buffer[32];
    sprintf(buffer, "%d", valor);
    return buffer;
}

HWND etiqueta(int tab, const wchar_t* texto, int id) {
    HWND hwnd = CreateWindowExW(0, L"STATIC", texto, WS_CHILD | WS_VISIBLE,
                                0, 0, 10, 10, gVentana, (HMENU)(INT_PTR)id, gInst, NULL);
    return anotar(hwnd, tab);
}

HWND caja(int tab, int id, bool multilinea) {
    DWORD estilo = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL;
    if (multilinea) {
        estilo = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY;
    }
    HWND hwnd = CreateWindowExW(0, L"EDIT", L"", estilo, 0, 0, 10, 10, gVentana, (HMENU)(INT_PTR)id, gInst, NULL);
    return anotar(hwnd, tab);
}

HWND boton(int tab, int id, const wchar_t* texto) {
    HWND hwnd = CreateWindowExW(0, L"BUTTON", texto, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                0, 0, 10, 10, gVentana, (HMENU)(INT_PTR)id, gInst, NULL);
    return anotar(hwnd, tab);
}

void pista(HWND hwnd, const wchar_t* texto) {
    SendMessageW(hwnd, EM_SETCUEBANNER, TRUE, (LPARAM)texto);
}

void limpiarColumnas(HWND lista) {
    while (SendMessageW(lista, LVM_DELETECOLUMN, 0, 0)) {
    }
    SendMessageW(lista, LVM_DELETEALLITEMS, 0, 0);
}

void columna(HWND lista, int indice, const wchar_t* texto, int ancho) {
    LVCOLUMNW col;
    ZeroMemory(&col, sizeof(col));
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    col.pszText = (LPWSTR)texto;
    col.cx = ancho;
    col.iSubItem = indice;
    SendMessageW(lista, LVM_INSERTCOLUMNW, (WPARAM)indice, (LPARAM)&col);
}

void subcelda(HWND lista, int indice, int sub, const wstring& texto) {
    if (texto.empty()) {
        return;
    }
    LVITEMW item;
    ZeroMemory(&item, sizeof(item));
    item.iSubItem = sub;
    item.pszText = (LPWSTR)texto.c_str();
    SendMessageW(lista, LVM_SETITEMTEXTW, (WPARAM)indice, (LPARAM)&item);
}

void fila(HWND lista, int indice, const wstring& c0, const wstring& c1, const wstring& c2, const wstring& c3) {
    LVITEMW item;
    ZeroMemory(&item, sizeof(item));
    item.mask = LVIF_TEXT;
    item.iItem = indice;
    item.iSubItem = 0;
    item.pszText = (LPWSTR)c0.c_str();
    SendMessageW(lista, LVM_INSERTITEMW, 0, (LPARAM)&item);
    subcelda(lista, indice, 1, c1);
    subcelda(lista, indice, 2, c2);
    subcelda(lista, indice, 3, c3);
}

string celdaSeleccionada(HWND lista, int subindice) {
    int indice = (int)SendMessageW(lista, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
    if (indice < 0) {
        return "";
    }
    wchar_t buffer[256];
    LVITEMW item;
    ZeroMemory(&item, sizeof(item));
    item.iSubItem = subindice;
    item.pszText = buffer;
    item.cchTextMax = 256;
    SendMessageW(lista, LVM_GETITEMTEXTW, (WPARAM)indice, (LPARAM)&item);
    return trim(reducir(buffer));
}

void fijarCarpetaDatos() {
    char ruta[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, ruta, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) {
        establecerCarpetaDatos("datos");
        return;
    }
    for (int i = (int)n - 1; i >= 0; --i) {
        if (ruta[i] == '\\' || ruta[i] == '/') {
            ruta[i] = '\0';
            break;
        }
    }
    establecerCarpetaDatos(string(ruta) + "\\datos");
}

void cargarConexiones(const string& nombre) {
    gCargando = true;
    limpiarColumnas(gLista2);
    columna(gLista2, 0, L"Destino", 220);
    columna(gLista2, 1, L"Distancia", 120);
    Ciudad* ciudad = buscarCiudad(gCiudades, nombre);
    int filaIndice = 0;
    if (ciudad != NULL) {
        Conexion* conexion = ciudad->conexiones;
        while (conexion != NULL) {
            fila(gLista2, filaIndice, ampliar(conexion->destino), ampliar(textoDistancia(conexion->distancia) + " km"), L"", L"");
            filaIndice++;
            conexion = conexion->siguiente;
        }
    }
    gCargando = false;
}

void cargarLista() {
    gCargando = true;
    limpiarColumnas(gLista);

    if (gPestana == TAB_CIU) {
        columna(gLista, 0, L"Ciudad", 240);
        columna(gLista, 1, L"Conexiones de salida", 180);
        int i = 0;
        for (Ciudad* actual = gCiudades; actual != NULL; actual = actual->siguiente) {
            int salidas = 0;
            for (Conexion* conexion = actual->conexiones; conexion != NULL; conexion = conexion->siguiente) {
                salidas++;
            }
            fila(gLista, i, ampliar(actual->nombre), ampliar(enteroATexto(salidas)), L"", L"");
            i++;
        }
        cargarConexiones(leerCaja(GetDlgItem(gVentana, IDC_CIU_NOMBRE)));
    } else if (gPestana == TAB_RUT) {
        columna(gLista, 0, L"Origen", 180);
        columna(gLista, 1, L"Destino", 180);
        columna(gLista, 2, L"Distancia", 120);
        int i = 0;
        for (Ciudad* actual = gCiudades; actual != NULL; actual = actual->siguiente) {
            for (Conexion* conexion = actual->conexiones; conexion != NULL; conexion = conexion->siguiente) {
                fila(gLista, i, ampliar(actual->nombre), ampliar(conexion->destino),
                     ampliar(textoDistancia(conexion->distancia) + " km"), L"");
                i++;
            }
        }
    } else if (gPestana == TAB_PAS) {
        columna(gLista, 0, L"Documento", 140);
        columna(gLista, 1, L"Nombre", 220);
        columna(gLista, 2, L"Edad", 70);
        columna(gLista, 3, L"Telefono", 140);
        int i = 0;
        for (Pasajero* actual = gPasajeros; actual != NULL; actual = actual->siguiente) {
            fila(gLista, i, ampliar(actual->documento), ampliar(actual->nombre),
                 ampliar(enteroATexto(actual->edad)), ampliar(actual->telefono));
            i++;
        }
    } else if (gPestana == TAB_COL) {
        columna(gLista, 0, L"Turno", 70);
        columna(gLista, 1, L"Documento", 130);
        columna(gLista, 2, L"Viaje", 260);
        columna(gLista, 3, L"Fecha", 120);
        int i = 0;
        int turno = 1;
        for (Solicitud* actual = gCola.frente; actual != NULL; actual = actual->siguiente) {
            fila(gLista, i, ampliar(enteroATexto(turno)), ampliar(actual->documento),
                 ampliar(actual->origen + " -> " + actual->destino), ampliar(actual->fecha));
            i++;
            turno++;
        }
    } else if (gPestana == TAB_HIS) {
        columna(gLista, 0, L"Id", 60);
        columna(gLista, 1, L"Pasajero", 200);
        columna(gLista, 2, L"Viaje", 240);
        columna(gLista, 3, L"Resultado", 120);
        int i = 0;
        for (Historial* actual = gHistorial; actual != NULL; actual = actual->siguiente) {
            if (!gFiltroHistorial.empty() && actual->documento != gFiltroHistorial) {
                continue;
            }
            fila(gLista, i, ampliar(enteroATexto(actual->idSolicitud)),
                 ampliar(actual->nombrePasajero + " (" + actual->documento + ")"),
                 ampliar(actual->origen + " -> " + actual->destino), ampliar(actual->resultado));
            i++;
        }
    }

    gCargando = false;
}

void mostrarResumenArchivos() {
    string texto = "Carpeta de datos:\r\n" + obtenerCarpetaDatos() + "\r\n\r\n";
    texto += "Ciudades: " + enteroATexto(contarCiudades(gCiudades)) + "\r\n";
    texto += "Rutas dirigidas: " + enteroATexto(contarRutas(gCiudades)) + "\r\n";
    texto += "Pasajeros: " + enteroATexto(contarPasajeros(gPasajeros)) + "\r\n";
    texto += "Solicitudes pendientes: " + enteroATexto(contarCola(gCola)) + "\r\n";
    texto += "Historial: " + enteroATexto(contarHistorial(gHistorial)) + "\r\n\r\n";
    texto += "Guardar reemplaza ciudades.txt, rutas.txt, pasajeros.txt, cola.txt e historial.txt.\r\n";
    texto += "Cargar vacia la memoria y reconstruye las listas desde esos archivos.";
    ponerTexto(gResultado, texto);
}

void mostrarPestana(int pestana) {
    gPestana = pestana;
    for (int i = 0; i < gNPiezas; i++) {
        ShowWindow(gPiezas[i].hwnd, gPiezas[i].tab == pestana ? SW_SHOW : SW_HIDE);
    }

    bool usaLista = pestana != TAB_BUS && pestana != TAB_REP && pestana != TAB_ARC;
    bool usaTexto = pestana == TAB_BUS || pestana == TAB_REP || pestana == TAB_ARC;
    ShowWindow(gLista, usaLista ? SW_SHOW : SW_HIDE);
    ShowWindow(gLista2, pestana == TAB_CIU ? SW_SHOW : SW_HIDE);
    ShowWindow(gResultado, usaTexto ? SW_SHOW : SW_HIDE);

    if (pestana == TAB_REP) {
        ponerTexto(gResultado, generarReporteCompleto(gCiudades, gPasajeros, gCola, gHistorial));
    } else if (pestana == TAB_ARC) {
        mostrarResumenArchivos();
    } else if (pestana == TAB_BUS) {
        ponerTexto(gResultado, "");
    }

    if (usaLista) {
        cargarLista();
    }
    estado(L"SITRAC listo");
}

void alSeleccionar() {
    if (gCargando) {
        return;
    }
    if (gPestana == TAB_CIU) {
        string nombre = celdaSeleccionada(gLista, 0);
        if (!nombre.empty()) {
            ponerTexto(GetDlgItem(gVentana, IDC_CIU_NOMBRE), nombre);
            ponerTexto(GetDlgItem(gVentana, IDC_CIU_ORI), nombre);
            cargarConexiones(nombre);
        }
    } else if (gPestana == TAB_RUT) {
        ponerTexto(GetDlgItem(gVentana, IDC_RUT_ORI), celdaSeleccionada(gLista, 0));
        ponerTexto(GetDlgItem(gVentana, IDC_RUT_DES), celdaSeleccionada(gLista, 1));
    } else if (gPestana == TAB_PAS) {
        ponerTexto(GetDlgItem(gVentana, IDC_PAS_DOC), celdaSeleccionada(gLista, 0));
        ponerTexto(GetDlgItem(gVentana, IDC_PAS_NOM), celdaSeleccionada(gLista, 1));
        ponerTexto(GetDlgItem(gVentana, IDC_PAS_EDAD), celdaSeleccionada(gLista, 2));
        ponerTexto(GetDlgItem(gVentana, IDC_PAS_TEL), celdaSeleccionada(gLista, 3));
    } else if (gPestana == TAB_COL) {
        string viaje = celdaSeleccionada(gLista, 2);
        string origen = viaje;
        string destino = "";
        size_t marca = viaje.find(" -> ");
        if (marca != string::npos) {
            origen = viaje.substr(0, marca);
            destino = viaje.substr(marca + 4);
        }
        ponerTexto(GetDlgItem(gVentana, IDC_COL_DOC), celdaSeleccionada(gLista, 1));
        ponerTexto(GetDlgItem(gVentana, IDC_COL_ORI), origen);
        ponerTexto(GetDlgItem(gVentana, IDC_COL_DES), destino);
        ponerTexto(GetDlgItem(gVentana, IDC_COL_FEC), celdaSeleccionada(gLista, 3));
    }
}

string textoErrorConexion(int codigo) {
    if (codigo == 1) return "La ciudad de origen no existe.";
    if (codigo == 2) return "La ciudad de destino no existe.";
    if (codigo == 3) return "Una ciudad no puede conectarse consigo misma.";
    if (codigo == 4) return "Esa ruta ya existe.";
    if (codigo == 5) return "La distancia debe ser mayor que cero.";
    return "No se pudo registrar la ruta.";
}

void registrarCiudadGui() {
    string nombre = leerCaja(GetDlgItem(gVentana, IDC_CIU_NOMBRE));
    if (nombre.empty()) {
        avisar(L"Ciudades", "Escriba el nombre de la ciudad.", true);
        return;
    }
    if (contieneSeparador(nombre)) {
        avisar(L"Ciudades", "El nombre no puede contener el caracter |.", true);
        return;
    }
    if (buscarCiudad(gCiudades, nombre) != NULL) {
        avisar(L"Ciudades", "Esa ciudad ya esta registrada.", true);
        return;
    }
    if (!insertarCiudadDirecta(gCiudades, nombre)) {
        avisar(L"Ciudades", "No se pudo registrar la ciudad.", true);
        return;
    }
    ponerTexto(GetDlgItem(gVentana, IDC_CIU_NOMBRE), "");
    cargarLista();
    estado("Ciudad registrada: " + nombre);
}

void eliminarCiudadGui() {
    string nombre = leerCaja(GetDlgItem(gVentana, IDC_CIU_NOMBRE));
    if (nombre.empty()) {
        nombre = celdaSeleccionada(gLista, 0);
    }
    int resultado = eliminarCiudadPorNombre(gCiudades, nombre);
    if (resultado != 0) {
        avisar(L"Ciudades", resultado == 1 ? "No hay ciudades para eliminar." : "La ciudad no existe.", true);
        return;
    }
    ponerTexto(GetDlgItem(gVentana, IDC_CIU_NOMBRE), "");
    cargarLista();
    estado("Ciudad eliminada: " + nombre);
}

void agregarRutaGui(bool desdeCiudades) {
    int idOrigen = desdeCiudades ? IDC_CIU_ORI : IDC_RUT_ORI;
    int idDestino = desdeCiudades ? IDC_CIU_DES : IDC_RUT_DES;
    int idKm = desdeCiudades ? IDC_CIU_KM : IDC_RUT_KM;
    string origen = leerCaja(GetDlgItem(gVentana, idOrigen));
    string destino = leerCaja(GetDlgItem(gVentana, idDestino));
    string textoKm = leerCaja(GetDlgItem(gVentana, idKm));
    double distancia = 0;
    if (!leerDoubleTexto(textoKm, distancia)) {
        avisar(L"Rutas", "Escriba una distancia numerica. Use punto decimal.", true);
        return;
    }

    int codigo = agregarConexionDatos(gCiudades, origen, destino, distancia, false);
    if (codigo != 0) {
        avisar(L"Rutas", textoErrorConexion(codigo), true);
        return;
    }

    bool idaVuelta = !desdeCiudades && SendMessageW(GetDlgItem(gVentana, IDC_RUT_IDA), BM_GETCHECK, 0, 0) == BST_CHECKED;
    if (idaVuelta) {
        int regreso = agregarConexionDatos(gCiudades, destino, origen, distancia, false);
        if (regreso != 0 && regreso != 4) {
            avisar(L"Rutas", "La ida se registro, pero el regreso no: " + textoErrorConexion(regreso), true);
        }
    }
    cargarLista();
    estado(idaVuelta ? "Ruta de ida y vuelta registrada." : "Ruta registrada.");
}

void quitarRutaGui(bool desdeCiudades) {
    int idOrigen = desdeCiudades ? IDC_CIU_ORI : IDC_RUT_ORI;
    int idDestino = desdeCiudades ? IDC_CIU_DES : IDC_RUT_DES;
    string origen = leerCaja(GetDlgItem(gVentana, idOrigen));
    string destino = leerCaja(GetDlgItem(gVentana, idDestino));
    if (!eliminarConexionDatos(gCiudades, origen, destino)) {
        avisar(L"Rutas", "La ruta indicada no existe.", true);
        return;
    }
    bool idaVuelta = !desdeCiudades && SendMessageW(GetDlgItem(gVentana, IDC_RUT_IDA), BM_GETCHECK, 0, 0) == BST_CHECKED;
    if (idaVuelta) {
        eliminarConexionDatos(gCiudades, destino, origen);
    }
    cargarLista();
    estado("Ruta eliminada.");
}

void caminoGui() {
    string origen = leerCaja(GetDlgItem(gVentana, IDC_RUT_ORI));
    string destino = leerCaja(GetDlgItem(gVentana, IDC_RUT_DES));
    if (buscarCiudad(gCiudades, origen) == NULL || buscarCiudad(gCiudades, destino) == NULL) {
        avisar(L"Rutas", "Indique dos ciudades registradas.", true);
        return;
    }
    ResultadoCamino camino = calcularCaminoMasCorto(gCiudades, origen, destino);
    if (!camino.encontrado) {
        ponerTexto(GetDlgItem(gVentana, IDC_RUT_RES), "No existe un camino entre esas ciudades.");
        return;
    }
    ponerTexto(GetDlgItem(gVentana, IDC_RUT_RES), "Camino mas corto: " + camino.recorrido + "\r\nDistancia: " + textoDistancia(camino.distancia) + " km");
}

void registrarPasajeroGui() {
    string documento = leerCaja(GetDlgItem(gVentana, IDC_PAS_DOC));
    string nombre = leerCaja(GetDlgItem(gVentana, IDC_PAS_NOM));
    string telefono = leerCaja(GetDlgItem(gVentana, IDC_PAS_TEL));
    int edad = 0;
    if (!leerEnteroTexto(leerCaja(GetDlgItem(gVentana, IDC_PAS_EDAD)), edad)) {
        avisar(L"Pasajeros", "La edad debe ser un numero entero.", true);
        return;
    }
    if (documento.empty() || nombre.empty()) {
        avisar(L"Pasajeros", "Documento y nombre son obligatorios.", true);
        return;
    }
    if (contieneSeparador(documento) || contieneSeparador(nombre) || contieneSeparador(telefono)) {
        avisar(L"Pasajeros", "Los datos no pueden contener el caracter |.", true);
        return;
    }
    if (buscarPasajero(gPasajeros, documento) != NULL) {
        avisar(L"Pasajeros", "Ese documento ya esta registrado.", true);
        return;
    }
    if (edad <= 0 || !insertarPasajeroDirecto(gPasajeros, documento, nombre, edad, telefono)) {
        avisar(L"Pasajeros", "No se pudo registrar el pasajero.", true);
        return;
    }
    cargarLista();
    estado("Pasajero registrado: " + nombre);
}

void actualizarPasajeroGui() {
    string documento = leerCaja(GetDlgItem(gVentana, IDC_PAS_DOC));
    int edad = 0;
    if (!leerEnteroTexto(leerCaja(GetDlgItem(gVentana, IDC_PAS_EDAD)), edad)) {
        avisar(L"Pasajeros", "La edad debe ser un numero entero.", true);
        return;
    }
    if (!actualizarPasajeroDatos(gPasajeros, documento, leerCaja(GetDlgItem(gVentana, IDC_PAS_NOM)), edad, leerCaja(GetDlgItem(gVentana, IDC_PAS_TEL)))) {
        avisar(L"Pasajeros", "No se pudo actualizar. Verifique el documento, el nombre y la edad.", true);
        return;
    }
    cargarLista();
    estado("Pasajero actualizado.");
}

void eliminarPasajeroGui() {
    string documento = leerCaja(GetDlgItem(gVentana, IDC_PAS_DOC));
    if (!eliminarPasajeroPorDocumento(gPasajeros, documento)) {
        avisar(L"Pasajeros", "El pasajero no existe.", true);
        return;
    }
    cargarLista();
    estado("Pasajero eliminado.");
}

void encolarGui() {
    string documento = leerCaja(GetDlgItem(gVentana, IDC_COL_DOC));
    string origen = leerCaja(GetDlgItem(gVentana, IDC_COL_ORI));
    string destino = leerCaja(GetDlgItem(gVentana, IDC_COL_DES));
    string fecha = leerCaja(GetDlgItem(gVentana, IDC_COL_FEC));
    Pasajero* pasajero = buscarPasajero(gPasajeros, documento);
    if (pasajero == NULL) {
        avisar(L"Cola", "El pasajero no esta registrado.", true);
        return;
    }
    if (buscarCiudad(gCiudades, origen) == NULL || buscarCiudad(gCiudades, destino) == NULL) {
        avisar(L"Cola", "El origen y el destino deben ser ciudades registradas.", true);
        return;
    }
    if (trim(origen) == trim(destino)) {
        avisar(L"Cola", "El origen y el destino deben ser diferentes.", true);
        return;
    }
    if (fecha.empty() || contieneSeparador(fecha)) {
        avisar(L"Cola", "Escriba una fecha sin el caracter |.", true);
        return;
    }
    int id = gCola.siguienteId;
    if (!encolarExistente(gCola, id, pasajero->documento, trim(origen), trim(destino), fecha)) {
        avisar(L"Cola", "No se pudo encolar la solicitud.", true);
        return;
    }
    ResultadoCamino camino = calcularCaminoMasCorto(gCiudades, origen, destino);
    cargarLista();
    if (!camino.encontrado) {
        estado("Solicitud #" + enteroATexto(id) + " en cola. Aun no hay camino.");
        avisar(L"Cola", "La solicitud quedo en cola, pero todavia no existe un camino entre esas ciudades.", false);
    } else {
        estado("Solicitud #" + enteroATexto(id) + " agregada al final de la cola.");
    }
}

void procesarGui(bool cancelar) {
    ResultadoSolicitud salida = procesarSiguienteSolicitud(gCola, gCiudades, gPasajeros, gHistorial, cancelar);
    if (!salida.procesada) {
        avisar(L"Cola", salida.texto, true);
        return;
    }
    cargarLista();
    avisar(cancelar ? L"Solicitud cancelada" : L"Solicitud atendida", salida.texto, salida.resultado != "ATENDIDA");
    estado(salida.resultado);
}

void buscarGui() {
    int tipo = (int)SendMessageW(GetDlgItem(gVentana, IDC_BUS_TIPO), CB_GETCURSEL, 0, 0);
    string a = leerCaja(GetDlgItem(gVentana, IDC_BUS_A));
    string b = leerCaja(GetDlgItem(gVentana, IDC_BUS_B));
    string texto = "";

    if (tipo == 0) {
        Ciudad* ciudad = buscarCiudad(gCiudades, a);
        if (ciudad == NULL) {
            texto = "No se encontro la ciudad.";
        } else {
            texto = "Ciudad: " + ciudad->nombre + "\r\n";
            if (ciudad->conexiones == NULL) {
                texto += "No tiene rutas de salida.";
            }
            for (Conexion* conexion = ciudad->conexiones; conexion != NULL; conexion = conexion->siguiente) {
                texto += "- " + conexion->destino + " (" + textoDistancia(conexion->distancia) + " km)\r\n";
            }
        }
    } else if (tipo == 1) {
        Pasajero* pasajero = buscarPasajero(gPasajeros, a);
        if (pasajero == NULL) {
            texto = "Pasajero no encontrado.";
        } else {
            texto = "Documento: " + pasajero->documento + "\r\nNombre: " + pasajero->nombre +
                    "\r\nEdad: " + enteroATexto(pasajero->edad) + "\r\nTelefono: " + pasajero->telefono;
        }
    } else if (tipo == 2) {
        bool encontro = false;
        for (Pasajero* actual = gPasajeros; actual != NULL; actual = actual->siguiente) {
            if (actual->nombre.find(a) != string::npos) {
                texto += actual->documento + " - " + actual->nombre + " (" + enteroATexto(actual->edad) + ")\r\n";
                encontro = true;
            }
        }
        if (!encontro) {
            texto = "No se encontraron pasajeros con ese nombre.";
        }
    } else if (tipo == 3) {
        Ciudad* origen = buscarCiudad(gCiudades, a);
        Conexion* conexion = (origen != NULL) ? buscarConexion(origen, trim(b)) : NULL;
        if (conexion == NULL) {
            texto = "No existe una ruta directa.";
        } else {
            texto = origen->nombre + " -> " + conexion->destino + " (" + textoDistancia(conexion->distancia) + " km)";
        }
    } else if (tipo == 4) {
        ResultadoCamino camino = calcularCaminoMasCorto(gCiudades, a, b);
        if (!camino.encontrado) {
            texto = "No existe un camino entre esas ciudades.";
        } else {
            texto = camino.recorrido + "\r\nDistancia: " + textoDistancia(camino.distancia) + " km";
        }
    } else if (tipo == 5) {
        bool encontro = false;
        for (Solicitud* actual = gCola.frente; actual != NULL; actual = actual->siguiente) {
            if (actual->documento == trim(a)) {
                texto += "#" + enteroATexto(actual->id) + "  " + actual->origen + " -> " + actual->destino + "  " + actual->fecha + "\r\n";
                encontro = true;
            }
        }
        if (!encontro) {
            texto = "Ese documento no tiene solicitudes pendientes.";
        }
    } else {
        bool encontro = false;
        for (Historial* actual = gHistorial; actual != NULL; actual = actual->siguiente) {
            if (actual->documento == trim(a)) {
                texto += "#" + enteroATexto(actual->idSolicitud) + "  " + actual->origen + " -> " + actual->destino +
                         "  " + actual->resultado + "\r\n";
                encontro = true;
            }
        }
        if (!encontro) {
            texto = "No hay historial para ese documento.";
        }
    }

    ponerTexto(gResultado, texto);
}

void organizar(int ancho, int alto) {
    int estadoAlto = 24;
    MoveWindow(gEncabezado, 0, 0, ancho, 72, TRUE);
    MoveWindow(gTabs, 12, 80, ancho - 24, alto - 108, TRUE);
    SendMessageW(gEstado, WM_SIZE, 0, 0);

    RECT pagina;
    GetClientRect(gTabs, &pagina);
    TabCtrl_AdjustRect(gTabs, FALSE, &pagina);
    int x = 12 + pagina.left + 8;
    int y = 80 + pagina.top + 8;
    int anchoPagina = pagina.right - pagina.left - 16;
    int altoPagina = pagina.bottom - pagina.top - 16;
    if (anchoPagina < 200) {
        anchoPagina = 200;
    }
    if (altoPagina < 120) {
        altoPagina = 120;
    }

    int formulario = 330;
    int listaX = x + formulario + 12;
    int listaAncho = ancho - listaX - 20;
    if (listaAncho < 180) {
        listaAncho = 180;
    }

    MoveWindow(gLblCiudad, x, y, formulario - 8, 20, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_NOMBRE), x, y + 24, formulario - 8, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_REG), x, y + 56, 150, 28, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_ELI), x + 158, y + 56, 160, 28, TRUE);
    MoveWindow(gLblConexion, x, y + 98, formulario - 8, 20, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_ORI), x, y + 124, formulario - 8, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_DES), x, y + 156, formulario - 8, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_KM), x, y + 188, formulario - 8, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_ADD), x, y + 220, 150, 28, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_QUIT), x + 158, y + 220, 160, 28, TRUE);

    MoveWindow(gLblRuta, x, y, formulario - 8, 20, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_ORI), x, y + 24, formulario - 8, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_DES), x, y + 56, formulario - 8, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_KM), x, y + 88, formulario - 8, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_IDA), x, y + 118, formulario - 8, 22, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_REG), x, y + 148, 150, 28, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_ELI), x + 158, y + 148, 160, 28, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_CAM), x, y + 184, formulario - 8, 28, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_RES), x, y + 220, formulario - 8, 90, TRUE);

    MoveWindow(gLblPasajero, x, y, formulario - 8, 20, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_PAS_DOC), x, y + 24, formulario - 8, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_PAS_NOM), x, y + 56, formulario - 8, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_PAS_EDAD), x, y + 88, 100, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_PAS_TEL), x + 108, y + 88, formulario - 116, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_PAS_REG), x, y + 122, 150, 28, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_PAS_ACT), x + 158, y + 122, 160, 28, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_PAS_ELI), x, y + 156, 150, 28, TRUE);

    MoveWindow(gLblCola, x, y, formulario - 8, 20, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_COL_DOC), x, y + 24, formulario - 8, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_COL_ORI), x, y + 56, formulario - 8, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_COL_DES), x, y + 88, formulario - 8, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_COL_FEC), x, y + 120, formulario - 8, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_COL_ADD), x, y + 154, formulario - 8, 28, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_COL_ATE), x, y + 190, 150, 28, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_COL_CAN), x + 158, y + 190, 160, 28, TRUE);

    MoveWindow(GetDlgItem(gVentana, IDC_HIS_DOC), x, y + 4, 220, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_HIS_FIL), x + 228, y + 2, 120, 28, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_HIS_TODO), x + 356, y + 2, 140, 28, TRUE);

    MoveWindow(GetDlgItem(gVentana, IDC_BUS_TIPO), x, y + 4, 280, 200, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_BUS_A), x + 292, y + 6, 220, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_BUS_B), x + 520, y + 6, 220, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_BUS_GO), x + 750, y + 4, 120, 28, TRUE);

    MoveWindow(GetDlgItem(gVentana, IDC_REP_ACT), x, y + 4, 180, 28, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_ARC_GUA), x, y + 8, 180, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_ARC_CAR), x + 192, y + 8, 180, 32, TRUE);

    int listaY = (gPestana == TAB_HIS) ? y + 40 : y;
    int listaAlto = (gPestana == TAB_CIU) ? (altoPagina - 8) / 2 : alto - listaY - estadoAlto - 16;
    if (gPestana == TAB_HIS) {
        listaAlto = alto - listaY - estadoAlto - 16;
    }
    MoveWindow(gLista, (gPestana == TAB_HIS || gPestana == TAB_BUS) ? x : listaX, listaY,
               (gPestana == TAB_HIS) ? anchoPagina : listaAncho, listaAlto, TRUE);
    MoveWindow(gLista2, listaX, listaY + listaAlto + 8, listaAncho, altoPagina - listaAlto - 8, TRUE);

    int resultadoY = (gPestana == TAB_BUS || gPestana == TAB_REP) ? y + 42 : y + 52;
    MoveWindow(gResultado, x, resultadoY, anchoPagina, alto - resultadoY - estadoAlto - 16, TRUE);
    (void)estadoAlto;
    (void)altoPagina;
}

void crearControles() {
    gEncabezado = CreateWindowExW(0, L"SITRACEncabezado", L"", WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, gVentana, NULL, gInst, NULL);
    gTabs = CreateWindowExW(0, WC_TABCONTROLW, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0, 10, 10, gVentana, (HMENU)IDC_TAB, gInst, NULL);

    const wchar_t* titulos[8];
    titulos[0] = L"Ciudades";
    titulos[1] = L"Rutas";
    titulos[2] = L"Pasajeros";
    titulos[3] = L"Cola";
    titulos[4] = L"Historial";
    titulos[5] = L"B\u00fasquedas";
    titulos[6] = L"Reportes";
    titulos[7] = L"Archivos";
    for (int i = 0; i < 8; i++) {
        TCITEMW item;
        ZeroMemory(&item, sizeof(item));
        item.mask = TCIF_TEXT;
        item.pszText = (LPWSTR)titulos[i];
        SendMessageW(gTabs, TCM_INSERTITEMW, (WPARAM)i, (LPARAM)&item);
    }

    DWORD estiloLista = WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS;
    gLista = CreateWindowExW(0, WC_LISTVIEWW, L"", estiloLista, 0, 0, 10, 10, gVentana, (HMENU)IDC_LISTA, gInst, NULL);
    gLista2 = CreateWindowExW(0, WC_LISTVIEWW, L"", estiloLista, 0, 0, 10, 10, gVentana, (HMENU)IDC_LISTA2, gInst, NULL);
    SendMessageW(gLista, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
    SendMessageW(gLista2, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

    gResultado = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                                 0, 0, 10, 10, gVentana, (HMENU)IDC_RESULTADO, gInst, NULL);
    gEstado = CreateWindowExW(0, STATUSCLASSNAMEW, L"", WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 10, 10, gVentana, NULL, gInst, NULL);

    gLblCiudad = etiqueta(TAB_CIU, L"Nombre de la ciudad", 0);
    caja(TAB_CIU, IDC_CIU_NOMBRE, false);
    boton(TAB_CIU, IDC_CIU_REG, L"Registrar ciudad");
    boton(TAB_CIU, IDC_CIU_ELI, L"Eliminar ciudad");
    gLblConexion = etiqueta(TAB_CIU, L"Conexi\u00f3n entre ciudades", 0);
    caja(TAB_CIU, IDC_CIU_ORI, false);
    caja(TAB_CIU, IDC_CIU_DES, false);
    caja(TAB_CIU, IDC_CIU_KM, false);
    boton(TAB_CIU, IDC_CIU_ADD, L"Agregar conexi\u00f3n");
    boton(TAB_CIU, IDC_CIU_QUIT, L"Quitar conexi\u00f3n");
    pista(GetDlgItem(gVentana, IDC_CIU_ORI), L"Ciudad de origen");
    pista(GetDlgItem(gVentana, IDC_CIU_DES), L"Ciudad de destino");
    pista(GetDlgItem(gVentana, IDC_CIU_KM), L"Distancia en km");

    gLblRuta = etiqueta(TAB_RUT, L"Ruta dirigida", 0);
    caja(TAB_RUT, IDC_RUT_ORI, false);
    caja(TAB_RUT, IDC_RUT_DES, false);
    caja(TAB_RUT, IDC_RUT_KM, false);
    anotar(CreateWindowExW(0, L"BUTTON", L"Registrar tambi\u00e9n el regreso", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                           0, 0, 10, 10, gVentana, (HMENU)(INT_PTR)IDC_RUT_IDA, gInst, NULL), TAB_RUT);
    boton(TAB_RUT, IDC_RUT_REG, L"Registrar ruta");
    boton(TAB_RUT, IDC_RUT_ELI, L"Eliminar ruta");
    boton(TAB_RUT, IDC_RUT_CAM, L"Calcular camino m\u00e1s corto");
    caja(TAB_RUT, IDC_RUT_RES, true);
    pista(GetDlgItem(gVentana, IDC_RUT_ORI), L"Origen");
    pista(GetDlgItem(gVentana, IDC_RUT_DES), L"Destino");
    pista(GetDlgItem(gVentana, IDC_RUT_KM), L"Distancia en km");

    gLblPasajero = etiqueta(TAB_PAS, L"Datos del pasajero", 0);
    caja(TAB_PAS, IDC_PAS_DOC, false);
    caja(TAB_PAS, IDC_PAS_NOM, false);
    caja(TAB_PAS, IDC_PAS_EDAD, false);
    caja(TAB_PAS, IDC_PAS_TEL, false);
    boton(TAB_PAS, IDC_PAS_REG, L"Registrar");
    boton(TAB_PAS, IDC_PAS_ACT, L"Actualizar");
    boton(TAB_PAS, IDC_PAS_ELI, L"Eliminar");
    pista(GetDlgItem(gVentana, IDC_PAS_DOC), L"Documento");
    pista(GetDlgItem(gVentana, IDC_PAS_NOM), L"Nombre completo");
    pista(GetDlgItem(gVentana, IDC_PAS_EDAD), L"Edad");
    pista(GetDlgItem(gVentana, IDC_PAS_TEL), L"Tel\u00e9fono");

    gLblCola = etiqueta(TAB_COL, L"Nueva solicitud", 0);
    caja(TAB_COL, IDC_COL_DOC, false);
    caja(TAB_COL, IDC_COL_ORI, false);
    caja(TAB_COL, IDC_COL_DES, false);
    caja(TAB_COL, IDC_COL_FEC, false);
    boton(TAB_COL, IDC_COL_ADD, L"Agregar al final de la cola");
    boton(TAB_COL, IDC_COL_ATE, L"Atender siguiente");
    boton(TAB_COL, IDC_COL_CAN, L"Cancelar siguiente");
    pista(GetDlgItem(gVentana, IDC_COL_DOC), L"Documento del pasajero");
    pista(GetDlgItem(gVentana, IDC_COL_ORI), L"Ciudad de origen");
    pista(GetDlgItem(gVentana, IDC_COL_DES), L"Ciudad de destino");
    pista(GetDlgItem(gVentana, IDC_COL_FEC), L"Fecha, por ejemplo 2026-11-17");

    caja(TAB_HIS, IDC_HIS_DOC, false);
    boton(TAB_HIS, IDC_HIS_FIL, L"Filtrar");
    boton(TAB_HIS, IDC_HIS_TODO, L"Ver todos");
    pista(GetDlgItem(gVentana, IDC_HIS_DOC), L"Documento");

    HWND combo = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
                                 0, 0, 10, 10, gVentana, (HMENU)IDC_BUS_TIPO, gInst, NULL);
    anotar(combo, TAB_BUS);
    const wchar_t* opciones[7];
    opciones[0] = L"Ciudad";
    opciones[1] = L"Pasajero por documento";
    opciones[2] = L"Pasajero por nombre";
    opciones[3] = L"Ruta directa";
    opciones[4] = L"Camino m\u00e1s corto";
    opciones[5] = L"Solicitudes por documento";
    opciones[6] = L"Historial por documento";
    for (int i = 0; i < 7; i++) {
        SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)opciones[i]);
    }
    SendMessageW(combo, CB_SETCURSEL, 0, 0);
    caja(TAB_BUS, IDC_BUS_A, false);
    caja(TAB_BUS, IDC_BUS_B, false);
    boton(TAB_BUS, IDC_BUS_GO, L"Buscar");
    pista(GetDlgItem(gVentana, IDC_BUS_A), L"Primer dato");
    pista(GetDlgItem(gVentana, IDC_BUS_B), L"Destino, si aplica");

    boton(TAB_REP, IDC_REP_ACT, L"Actualizar reporte");
    boton(TAB_ARC, IDC_ARC_GUA, L"Guardar");
    boton(TAB_ARC, IDC_ARC_CAR, L"Cargar");

    SendMessageW(gVentana, WM_SETFONT, (WPARAM)gFuente, TRUE);
    SendMessageW(gTabs, WM_SETFONT, (WPARAM)gFuente, TRUE);
    SendMessageW(gLista, WM_SETFONT, (WPARAM)gFuente, TRUE);
    SendMessageW(gLista2, WM_SETFONT, (WPARAM)gFuente, TRUE);
    SendMessageW(gResultado, WM_SETFONT, (WPARAM)gFuente, TRUE);
    SendMessageW(gEstado, WM_SETFONT, (WPARAM)gFuente, TRUE);
    for (int i = 0; i < gNPiezas; i++) {
        SendMessageW(gPiezas[i].hwnd, WM_SETFONT, (WPARAM)gFuente, TRUE);
    }
}

void ejecutarComando(int id) {
    switch (id) {
        case IDC_CIU_REG: registrarCiudadGui(); break;
        case IDC_CIU_ELI: eliminarCiudadGui(); break;
        case IDC_CIU_ADD: agregarRutaGui(true); break;
        case IDC_CIU_QUIT: quitarRutaGui(true); break;
        case IDC_RUT_REG: agregarRutaGui(false); break;
        case IDC_RUT_ELI: quitarRutaGui(false); break;
        case IDC_RUT_CAM: caminoGui(); break;
        case IDC_PAS_REG: registrarPasajeroGui(); break;
        case IDC_PAS_ACT: actualizarPasajeroGui(); break;
        case IDC_PAS_ELI: eliminarPasajeroGui(); break;
        case IDC_COL_ADD: encolarGui(); break;
        case IDC_COL_ATE: procesarGui(false); break;
        case IDC_COL_CAN: procesarGui(true); break;
        case IDC_HIS_FIL:
            gFiltroHistorial = leerCaja(GetDlgItem(gVentana, IDC_HIS_DOC));
            cargarLista();
            break;
        case IDC_HIS_TODO:
            gFiltroHistorial = "";
            ponerTexto(GetDlgItem(gVentana, IDC_HIS_DOC), "");
            cargarLista();
            break;
        case IDC_BUS_GO: buscarGui(); break;
        case IDC_REP_ACT:
            ponerTexto(gResultado, generarReporteCompleto(gCiudades, gPasajeros, gCola, gHistorial));
            break;
        case IDC_ARC_GUA:
            if (guardarInformacion(gCiudades, gPasajeros, gCola, gHistorial)) {
                mostrarResumenArchivos();
                estado(obtenerMensajeArchivo());
                avisar(L"Archivos", obtenerMensajeArchivo(), false);
            } else {
                avisar(L"Archivos", obtenerMensajeArchivo(), true);
            }
            break;
        case IDC_ARC_CAR:
            if (cargarInformacion(gCiudades, gPasajeros, gCola, gHistorial)) {
                mostrarPestana(gPestana);
                estado(obtenerMensajeArchivo());
                avisar(L"Archivos", obtenerMensajeArchivo(), false);
            } else {
                avisar(L"Archivos", obtenerMensajeArchivo(), true);
            }
            break;
        default:
            break;
    }
}

LRESULT CALLBACK EncabezadoProc(HWND hwnd, UINT mensaje, WPARAM wParam, LPARAM lParam) {
    if (mensaje == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);
        HBRUSH brocha = CreateSolidBrush(RGB(14, 48, 92));
        FillRect(dc, &rect, brocha);
        DeleteObject(brocha);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(255, 255, 255));
        HFONT anterior = (HFONT)SelectObject(dc, gFuenteTitulo);
        RECT titulo = rect;
        titulo.left = 18;
        titulo.top = 12;
        DrawTextW(dc, L"SITRAC", -1, &titulo, DT_LEFT | DT_TOP | DT_SINGLELINE);
        SelectObject(dc, gFuente);
        SetTextColor(dc, RGB(214, 226, 242));
        titulo.top = 40;
        DrawTextW(dc, L"Sistema Inteligente de Transporte y Gesti\u00f3n de Rutas   \u00b7   Universidad Santo Tom\u00e1s",
                  -1, &titulo, DT_LEFT | DT_TOP | DT_SINGLELINE);
        SelectObject(dc, anterior);
        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProcW(hwnd, mensaje, wParam, lParam);
}

LRESULT CALLBACK VentanaProc(HWND hwnd, UINT mensaje, WPARAM wParam, LPARAM lParam) {
    switch (mensaje) {
        case WM_CREATE:
            gVentana = hwnd;
            crearControles();
            fijarCarpetaDatos();
            if (cargarInformacion(gCiudades, gPasajeros, gCola, gHistorial)) {
                estado("Se cargo la informacion de " + obtenerCarpetaDatos());
            } else {
                establecerCarpetaDatos("datos");
                if (cargarInformacion(gCiudades, gPasajeros, gCola, gHistorial)) {
                    estado("Se cargo la informacion de " + obtenerCarpetaDatos());
                } else {
                    fijarCarpetaDatos();
                    estado(L"Sin datos previos. Puede registrar informacion y guardarla.");
                }
            }
            mostrarPestana(TAB_CIU);
            return 0;
        case WM_SIZE: {
            int ancho = LOWORD(lParam);
            int alto = HIWORD(lParam);
            if (ancho > 0 && alto > 0) {
                organizar(ancho, alto);
            }
            return 0;
        }
        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == 0) {
                ejecutarComando(LOWORD(wParam));
            }
            return 0;
        case WM_NOTIFY: {
            NMHDR* aviso = (NMHDR*)lParam;
            if (aviso->idFrom == IDC_TAB && aviso->code == TCN_SELCHANGE) {
                mostrarPestana((int)SendMessageW(gTabs, TCM_GETCURSEL, 0, 0));
                RECT rect;
                GetClientRect(hwnd, &rect);
                organizar(rect.right, rect.bottom);
            } else if (aviso->idFrom == IDC_LISTA && aviso->code == LVN_ITEMCHANGED) {
                NMLISTVIEW* vista = (NMLISTVIEW*)lParam;
                if ((vista->uNewState & LVIS_SELECTED) != 0) {
                    alSeleccionar();
                }
            }
            return 0;
        }
        case WM_GETMINMAXINFO: {
            MINMAXINFO* info = (MINMAXINFO*)lParam;
            info->ptMinTrackSize.x = 980;
            info->ptMinTrackSize.y = 640;
            return 0;
        }
        case WM_DESTROY:
            liberarCiudades(gCiudades);
            liberarPasajeros(gPasajeros);
            liberarCola(gCola);
            liberarHistorial(gHistorial);
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd, mensaje, wParam, lParam);
}

}  // namespace

extern "C" int WINAPI WinMain(HINSTANCE instancia, HINSTANCE, LPSTR, int mostrar) {
    gInst = instancia;
    inicializarCola(gCola);

    INITCOMMONCONTROLSEX controles;
    controles.dwSize = sizeof(controles);
    controles.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES | ICC_BAR_CLASSES | ICC_WIN95_CLASSES;
    InitCommonControlsEx(&controles);

    gFuente = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    gFuenteTitulo = CreateFontW(-28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");

    WNDCLASSW encabezadoClase;
    ZeroMemory(&encabezadoClase, sizeof(encabezadoClase));
    encabezadoClase.lpfnWndProc = EncabezadoProc;
    encabezadoClase.hInstance = instancia;
    encabezadoClase.lpszClassName = L"SITRACEncabezado";
    encabezadoClase.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&encabezadoClase);

    WNDCLASSW clase;
    ZeroMemory(&clase, sizeof(clase));
    clase.lpfnWndProc = VentanaProc;
    clase.hInstance = instancia;
    clase.lpszClassName = L"SITRACVentana";
    clase.hCursor = LoadCursor(NULL, IDC_ARROW);
    clase.hbrBackground = CreateSolidBrush(RGB(241, 245, 249));
    clase.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassW(&clase);

    HWND ventana = CreateWindowExW(0, L"SITRACVentana",
                                   L"SITRAC - Sistema Inteligente de Transporte y Gesti\u00f3n de Rutas",
                                   WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
                                   1100, 700, NULL, NULL, instancia, NULL);
    if (ventana == NULL) {
        return 1;
    }

    ShowWindow(ventana, mostrar);
    UpdateWindow(ventana);

    MSG mensaje;
    while (GetMessageW(&mensaje, NULL, 0, 0) > 0) {
        if (!IsDialogMessageW(ventana, &mensaje)) {
            TranslateMessage(&mensaje);
            DispatchMessageW(&mensaje);
        }
    }

    if (gFuente != NULL) {
        DeleteObject(gFuente);
    }
    if (gFuenteTitulo != NULL) {
        DeleteObject(gFuenteTitulo);
    }
    return (int)mensaje.wParam;
}
