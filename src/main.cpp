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

#ifndef IDC_HAND
#define IDC_HAND MAKEINTRESOURCE(32649)
#endif

#ifndef FW_SEMIBOLD
#define FW_SEMIBOLD 600
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
HWND gLista = NULL;
HWND gLista2 = NULL;
HWND gResultado = NULL;
HWND gEstado = NULL;
HWND gMetricas = NULL;
HWND gTarjetaForm = NULL;
HWND gTarjetaLista = NULL;
HWND gNav[8];
HWND gLblCiudad = NULL;
HWND gLblConexion = NULL;
HWND gLblRuta = NULL;
HWND gLblPasajero = NULL;
HWND gLblCola = NULL;
HFONT gFuente = NULL;
HFONT gFuenteTitulo = NULL;
HFONT gFuenteMarca = NULL;
HFONT gFuenteMini = NULL;
HFONT gFuenteBoton = NULL;
HBRUSH gBrochaFondo = NULL;
HBRUSH gBrochaBlanca = NULL;
WNDPROC gProcEdit = NULL;
int gPestana = TAB_CIU;
bool gCargando = false;
string gFiltroHistorial = "";

Ciudad* gCiudades = NULL;
Pasajero* gPasajeros = NULL;
Cola gCola;
Historial* gHistorial = NULL;

Pieza gPiezas[80];
int gNPiezas = 0;

const int NAV_ANCHO = 252;
const int PIE_ALTO = 42;
const int IDC_NAV = 1900;

struct Cromo {
    int modo;
    int tab;
    int hover;
    int down;
};

COLORREF mezclar(COLORREF a, COLORREF b, int t) {
    if (t < 0) {
        t = 0;
    }
    if (t > 255) {
        t = 255;
    }
    int rojo = GetRValue(a) + (GetRValue(b) - GetRValue(a)) * t / 255;
    int verde = GetGValue(a) + (GetGValue(b) - GetGValue(a)) * t / 255;
    int azul = GetBValue(a) + (GetBValue(b) - GetBValue(a)) * t / 255;
    return RGB(rojo, verde, azul);
}

COLORREF colorBarra(int y, int alto) {
    if (alto < 1) {
        alto = 1;
    }
    if (y < 0) {
        y = 0;
    }
    if (y > alto) {
        y = alto;
    }
    return mezclar(RGB(96, 32, 168), RGB(28, 8, 52), y * 255 / alto);
}

void rellenarDegradado(HDC dc, RECT rect, COLORREF arriba, COLORREF abajo) {
    int alto = rect.bottom - rect.top;
    if (alto < 1) {
        return;
    }
    for (int y = 0; y < alto; y++) {
        HBRUSH brocha = CreateSolidBrush(mezclar(arriba, abajo, y * 255 / alto));
        RECT linea;
        linea.left = rect.left;
        linea.right = rect.right;
        linea.top = rect.top + y;
        linea.bottom = linea.top + 1;
        FillRect(dc, &linea, brocha);
        DeleteObject(brocha);
    }
}

void redondear(HDC dc, RECT rect, COLORREF relleno, COLORREF borde, int curva) {
    HBRUSH brocha = CreateSolidBrush(relleno);
    HPEN pluma = CreatePen(PS_SOLID, 1, borde);
    HGDIOBJ viejaBrocha = SelectObject(dc, brocha);
    HGDIOBJ viejaPluma = SelectObject(dc, pluma);
    RoundRect(dc, rect.left, rect.top, rect.right, rect.bottom, curva, curva);
    SelectObject(dc, viejaBrocha);
    SelectObject(dc, viejaPluma);
    DeleteObject(brocha);
    DeleteObject(pluma);
}

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
    if (gEstado != NULL) {
        SetWindowTextW(gEstado, texto.c_str());
        InvalidateRect(gEstado, NULL, FALSE);
    }
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

LRESULT CALLBACK EditTemaProc(HWND hwnd, UINT mensaje, WPARAM wParam, LPARAM lParam) {
    if (mensaje == WM_NCPAINT && gProcEdit != NULL) {
        CallWindowProcW(gProcEdit, hwnd, mensaje, wParam, lParam);
        HDC dc = GetWindowDC(hwnd);
        RECT rect;
        GetWindowRect(hwnd, &rect);
        OffsetRect(&rect, -rect.left, -rect.top);
        HBRUSH brocha = CreateSolidBrush(RGB(186, 156, 224));
        FrameRect(dc, &rect, brocha);
        DeleteObject(brocha);
        InflateRect(&rect, -1, -1);
        HBRUSH interior = CreateSolidBrush(RGB(255, 255, 255));
        FrameRect(dc, &rect, interior);
        DeleteObject(interior);
        ReleaseDC(hwnd, dc);
        return 0;
    }
    return CallWindowProcW(gProcEdit, hwnd, mensaje, wParam, lParam);
}

HWND caja(int tab, int id, bool multilinea) {
    DWORD estilo = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL;
    if (multilinea) {
        estilo = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY;
    }
    HWND hwnd = CreateWindowExW(0, L"EDIT", L"", estilo, 0, 0, 10, 10, gVentana, (HMENU)(INT_PTR)id, gInst, NULL);
    if (gProcEdit == NULL) {
        gProcEdit = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    }
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)EditTemaProc);
    return anotar(hwnd, tab);
}

HWND boton(int tab, int id, const wchar_t* texto) {
    Cromo* cromo = new Cromo();
    cromo->modo = 0;
    cromo->tab = tab;
    cromo->hover = 0;
    cromo->down = 0;
    if (id == IDC_CIU_ELI || id == IDC_CIU_QUIT || id == IDC_RUT_ELI || id == IDC_PAS_ELI || id == IDC_COL_CAN) {
        cromo->modo = 2;
    } else if (id == IDC_HIS_TODO || id == IDC_ARC_CAR || id == IDC_PAS_ACT) {
        cromo->modo = 1;
    }
    HWND hwnd = CreateWindowExW(0, L"SITRACCromo", texto, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                0, 0, 10, 10, gVentana, (HMENU)(INT_PTR)id, gInst, cromo);
    if (hwnd == NULL) {
        delete cromo;
    }
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
    if (gMetricas != NULL) {
        InvalidateRect(gMetricas, NULL, FALSE);
    }
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
        ShowWindow(gPiezas[i].hwnd, (gPiezas[i].tab == pestana || gPiezas[i].tab < 0) ? SW_SHOW : SW_HIDE);
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
    if (gMetricas != NULL) {
        InvalidateRect(gMetricas, NULL, FALSE);
    }
    for (int n = 0; n < 8; n++) {
        if (gNav[n] != NULL) {
            InvalidateRect(gNav[n], NULL, FALSE);
        }
    }
    if (gVentana != NULL) {
        InvalidateRect(gVentana, NULL, FALSE);
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
    if (gNav[0] == NULL || gTarjetaForm == NULL || gLista == NULL || gEstado == NULL || gMetricas == NULL) {
        return;
    }

    int margen = 22;
    int x = NAV_ANCHO + margen;
    int anchoUtil = ancho - x - margen;
    if (anchoUtil < 280) {
        anchoUtil = 280;
    }

    MoveWindow(gEstado, 0, alto - PIE_ALTO, ancho, PIE_ALTO, TRUE);
    for (int i = 0; i < 8; i++) {
        MoveWindow(gNav[i], 14, 128 + i * 48, NAV_ANCHO - 28, 42, TRUE);
    }

    int metricY = 78;
    int metricH = 82;
    MoveWindow(gMetricas, x, metricY, anchoUtil, metricH, TRUE);

    int cuerpoY = metricY + metricH + 16;
    int cuerpoH = alto - PIE_ALTO - cuerpoY - 16;
    if (cuerpoH < 180) {
        cuerpoH = 180;
    }

    bool formulario = gPestana == TAB_CIU || gPestana == TAB_RUT || gPestana == TAB_PAS || gPestana == TAB_COL;
    int formW = formulario ? 390 : 0;
    if (formulario && formW > anchoUtil - 220) {
        formW = anchoUtil - 220;
    }
    int listaX = formulario ? x + formW + 16 : x;
    int listaW = anchoUtil - (formulario ? formW + 16 : 0);

    ShowWindow(gTarjetaForm, formulario ? SW_SHOW : SW_HIDE);
    ShowWindow(gTarjetaLista, SW_SHOW);
    if (formulario) {
        MoveWindow(gTarjetaForm, x, cuerpoY, formW, cuerpoH, TRUE);
    }
    MoveWindow(gTarjetaLista, listaX, cuerpoY, listaW, cuerpoH, TRUE);

    int fx = x + 22;
    int fy = cuerpoY + 20;
    int fw = formW - 44;
    if (fw < 120) {
        fw = 120;
    }

    MoveWindow(gLblCiudad, fx, fy, fw, 22, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_NOMBRE), fx, fy + 26, fw, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_REG), fx, fy + 68, (fw - 10) / 2, 36, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_ELI), fx + (fw - 10) / 2 + 10, fy + 68, (fw - 10) / 2, 36, TRUE);
    MoveWindow(gLblConexion, fx, fy + 122, fw, 22, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_ORI), fx, fy + 148, fw, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_DES), fx, fy + 188, fw, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_KM), fx, fy + 228, fw, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_ADD), fx, fy + 272, (fw - 10) / 2, 36, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_CIU_QUIT), fx + (fw - 10) / 2 + 10, fy + 272, (fw - 10) / 2, 36, TRUE);

    MoveWindow(gLblRuta, fx, fy, fw, 22, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_ORI), fx, fy + 26, fw, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_DES), fx, fy + 66, fw, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_KM), fx, fy + 106, fw, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_IDA), fx, fy + 146, fw, 24, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_REG), fx, fy + 178, (fw - 10) / 2, 36, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_ELI), fx + (fw - 10) / 2 + 10, fy + 178, (fw - 10) / 2, 36, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_CAM), fx, fy + 224, fw, 36, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_RUT_RES), fx, fy + 270, fw, cuerpoH - 300, TRUE);

    MoveWindow(gLblPasajero, fx, fy, fw, 22, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_PAS_DOC), fx, fy + 26, fw, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_PAS_NOM), fx, fy + 66, fw, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_PAS_EDAD), fx, fy + 106, 110, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_PAS_TEL), fx + 120, fy + 106, fw - 120, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_PAS_REG), fx, fy + 152, (fw - 10) / 2, 36, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_PAS_ACT), fx + (fw - 10) / 2 + 10, fy + 152, (fw - 10) / 2, 36, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_PAS_ELI), fx, fy + 198, (fw - 10) / 2, 36, TRUE);

    MoveWindow(gLblCola, fx, fy, fw, 22, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_COL_DOC), fx, fy + 26, fw, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_COL_ORI), fx, fy + 66, fw, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_COL_DES), fx, fy + 106, fw, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_COL_FEC), fx, fy + 146, fw, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_COL_ADD), fx, fy + 190, fw, 36, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_COL_ATE), fx, fy + 236, (fw - 10) / 2, 36, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_COL_CAN), fx + (fw - 10) / 2 + 10, fy + 236, (fw - 10) / 2, 36, TRUE);

    int lx = listaX + 16;
    int ly = cuerpoY + 16;
    int lw = listaW - 32;
    int lh = cuerpoH - 32;
    if (lw < 80) {
        lw = 80;
    }

    MoveWindow(GetDlgItem(gVentana, IDC_HIS_DOC), lx, ly, 230, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_HIS_FIL), lx + 242, ly, 120, 34, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_HIS_TODO), lx + 372, ly, 140, 34, TRUE);

    MoveWindow(GetDlgItem(gVentana, IDC_BUS_TIPO), lx, ly, 250, 220, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_BUS_A), lx + 264, ly + 2, 200, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_BUS_B), lx + 474, ly + 2, 200, 32, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_BUS_GO), lx + 686, ly, 130, 36, TRUE);

    MoveWindow(GetDlgItem(gVentana, IDC_REP_ACT), lx, ly, 210, 36, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_ARC_GUA), lx, ly, 170, 36, TRUE);
    MoveWindow(GetDlgItem(gVentana, IDC_ARC_CAR), lx + 182, ly, 170, 36, TRUE);

    int listaY = ly;
    int listaAlto = lh;
    if (gPestana == TAB_HIS) {
        listaY = ly + 48;
        listaAlto = lh - 48;
    }
    if (gPestana == TAB_CIU) {
        int mitad = (listaAlto - 12) / 2;
        MoveWindow(gLista, lx, listaY, lw, mitad, TRUE);
        MoveWindow(gLista2, lx, listaY + mitad + 12, lw, listaAlto - mitad - 12, TRUE);
    } else {
        MoveWindow(gLista, lx, listaY, lw, listaAlto, TRUE);
    }

    int resultadoY = ly + 50;
    MoveWindow(gResultado, lx, resultadoY, lw, lh - 50, TRUE);
}

void crearControles() {
    gTarjetaForm = CreateWindowExW(0, L"SITRACTarjeta", L"", WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, gVentana, NULL, gInst, NULL);
    gTarjetaLista = CreateWindowExW(0, L"SITRACTarjeta", L"", WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, gVentana, NULL, gInst, NULL);
    gMetricas = CreateWindowExW(0, L"SITRACMetricas", L"", WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, gVentana, NULL, gInst, NULL);

    DWORD estiloLista = WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS;
    gLista = CreateWindowExW(0, WC_LISTVIEWW, L"", estiloLista, 0, 0, 10, 10, gVentana, (HMENU)IDC_LISTA, gInst, NULL);
    gLista2 = CreateWindowExW(0, WC_LISTVIEWW, L"", estiloLista, 0, 0, 10, 10, gVentana, (HMENU)IDC_LISTA2, gInst, NULL);
    SendMessageW(gLista, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    SendMessageW(gLista2, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    SendMessageW(gLista, LVM_SETBKCOLOR, 0, RGB(255, 255, 255));
    SendMessageW(gLista2, LVM_SETBKCOLOR, 0, RGB(255, 255, 255));
    SendMessageW(gLista, LVM_SETTEXTBKCOLOR, 0, RGB(255, 255, 255));
    SendMessageW(gLista2, LVM_SETTEXTBKCOLOR, 0, RGB(255, 255, 255));
    SendMessageW(gLista, LVM_SETTEXTCOLOR, 0, RGB(48, 22, 84));
    SendMessageW(gLista2, LVM_SETTEXTCOLOR, 0, RGB(48, 22, 84));

    HMODULE tema = LoadLibraryA("uxtheme.dll");
    if (tema != NULL) {
        typedef HRESULT (WINAPI *TemaFn)(HWND, LPCWSTR, LPCWSTR);
        TemaFn quitar = (TemaFn)GetProcAddress(tema, "SetWindowTheme");
        if (quitar != NULL) {
            quitar(gLista, L"", L"");
            quitar(gLista2, L"", L"");
        }
    }

    gResultado = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                                 0, 0, 10, 10, gVentana, (HMENU)IDC_RESULTADO, gInst, NULL);
    if (gProcEdit == NULL) {
        gProcEdit = (WNDPROC)GetWindowLongPtrW(gResultado, GWLP_WNDPROC);
    }
    SetWindowLongPtrW(gResultado, GWLP_WNDPROC, (LONG_PTR)EditTemaProc);

    gLblCiudad = etiqueta(TAB_CIU, L"Nombre de la ciudad", 0);
    caja(TAB_CIU, IDC_CIU_NOMBRE, false);
    boton(TAB_CIU, IDC_CIU_REG, L"Registrar ciudad");
    boton(TAB_CIU, IDC_CIU_ELI, L"Eliminar ciudad");
    gLblConexion = etiqueta(TAB_CIU, L"Conexion entre ciudades", 0);
    caja(TAB_CIU, IDC_CIU_ORI, false);
    caja(TAB_CIU, IDC_CIU_DES, false);
    caja(TAB_CIU, IDC_CIU_KM, false);
    boton(TAB_CIU, IDC_CIU_ADD, L"Agregar conexion");
    boton(TAB_CIU, IDC_CIU_QUIT, L"Quitar conexion");
    pista(GetDlgItem(gVentana, IDC_CIU_ORI), L"Ciudad de origen");
    pista(GetDlgItem(gVentana, IDC_CIU_DES), L"Ciudad de destino");
    pista(GetDlgItem(gVentana, IDC_CIU_KM), L"Distancia en km");

    gLblRuta = etiqueta(TAB_RUT, L"Ruta dirigida", 0);
    caja(TAB_RUT, IDC_RUT_ORI, false);
    caja(TAB_RUT, IDC_RUT_DES, false);
    caja(TAB_RUT, IDC_RUT_KM, false);
    anotar(CreateWindowExW(0, L"BUTTON", L"Registrar tambien el regreso", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                           0, 0, 10, 10, gVentana, (HMENU)(INT_PTR)IDC_RUT_IDA, gInst, NULL), TAB_RUT);
    boton(TAB_RUT, IDC_RUT_REG, L"Registrar ruta");
    boton(TAB_RUT, IDC_RUT_ELI, L"Eliminar ruta");
    boton(TAB_RUT, IDC_RUT_CAM, L"Calcular camino mas corto");
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
    pista(GetDlgItem(gVentana, IDC_PAS_TEL), L"Telefono");

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
    opciones[4] = L"Camino mas corto";
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

    const wchar_t* navTexto[8];
    navTexto[0] = L"Ciudades";
    navTexto[1] = L"Rutas";
    navTexto[2] = L"Pasajeros";
    navTexto[3] = L"Cola";
    navTexto[4] = L"Historial";
    navTexto[5] = L"Busquedas";
    navTexto[6] = L"Reportes";
    navTexto[7] = L"Archivos";
    for (int i = 0; i < 8; i++) {
        Cromo* cromo = new Cromo();
        cromo->modo = 3;
        cromo->tab = i;
        cromo->hover = 0;
        cromo->down = 0;
        gNav[i] = CreateWindowExW(0, L"SITRACCromo", navTexto[i], WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                  0, 0, 10, 10, gVentana, (HMENU)(INT_PTR)(IDC_NAV + i), gInst, cromo);
        anotar(gNav[i], -1);
    }

    gEstado = CreateWindowExW(0, L"SITRACPie", L"SITRAC listo", WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, gVentana, NULL, gInst, NULL);

    SendMessageW(gLista, WM_SETFONT, (WPARAM)gFuente, TRUE);
    SendMessageW(gLista2, WM_SETFONT, (WPARAM)gFuente, TRUE);
    SendMessageW(gResultado, WM_SETFONT, (WPARAM)gFuente, TRUE);
    for (int i = 0; i < gNPiezas; i++) {
        HFONT fuente = gFuente;
        wchar_t clase[32];
        GetClassNameW(gPiezas[i].hwnd, clase, 32);
        if (lstrcmpW(clase, L"STATIC") == 0) {
            fuente = gFuenteBoton;
        }
        SendMessageW(gPiezas[i].hwnd, WM_SETFONT, (WPARAM)fuente, TRUE);
    }

    RECT rect;
    GetClientRect(gVentana, &rect);
    organizar(rect.right, rect.bottom);
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

void pintarFondoBarra(HDC dc, HWND hwnd) {
    RECT local;
    GetClientRect(hwnd, &local);
    RECT pantalla;
    GetWindowRect(hwnd, &pantalla);
    POINT origen;
    origen.x = pantalla.left;
    origen.y = pantalla.top;
    ScreenToClient(GetParent(hwnd), &origen);
    RECT padre;
    GetClientRect(GetParent(hwnd), &padre);
    int alto = padre.bottom - PIE_ALTO;
    if (alto < 1) {
        alto = 1;
    }
    for (int y = 0; y < local.bottom; y++) {
        HBRUSH brocha = CreateSolidBrush(colorBarra(origen.y + y, alto));
        RECT linea;
        linea.left = 0;
        linea.right = local.right;
        linea.top = y;
        linea.bottom = y + 1;
        FillRect(dc, &linea, brocha);
        DeleteObject(brocha);
    }
}

void pulsarCromo(HWND hwnd) {
    int id = GetDlgCtrlID(hwnd);
    SendMessageW(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(id, BN_CLICKED), (LPARAM)hwnd);
}

LRESULT CALLBACK CromoProc(HWND hwnd, UINT mensaje, WPARAM wParam, LPARAM lParam) {
    if (mensaje == WM_NCCREATE) {
        CREATESTRUCTW* creado = (CREATESTRUCTW*)lParam;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)creado->lpCreateParams);
        return DefWindowProcW(hwnd, mensaje, wParam, lParam);
    }
    Cromo* cromo = (Cromo*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (mensaje == WM_NCDESTROY) {
        delete cromo;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        return DefWindowProcW(hwnd, mensaje, wParam, lParam);
    }
    if (cromo == NULL) {
        return DefWindowProcW(hwnd, mensaje, wParam, lParam);
    }

    switch (mensaje) {
        case WM_ERASEBKGND:
            return 1;
        case WM_SETCURSOR:
            SetCursor(LoadCursor(NULL, IDC_HAND));
            return TRUE;
        case WM_GETDLGCODE:
            return DLGC_BUTTON;
        case WM_MOUSEMOVE:
            if (!cromo->hover) {
                cromo->hover = 1;
                TRACKMOUSEEVENT rastreo;
                rastreo.cbSize = sizeof(rastreo);
                rastreo.dwFlags = TME_LEAVE;
                rastreo.hwndTrack = hwnd;
                rastreo.dwHoverTime = 0;
                TrackMouseEvent(&rastreo);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        case WM_MOUSELEAVE:
            cromo->hover = 0;
            cromo->down = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        case WM_LBUTTONDOWN:
            cromo->down = 1;
            SetCapture(hwnd);
            SetFocus(hwnd);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        case WM_LBUTTONUP:
            if (cromo->down) {
                cromo->down = 0;
                ReleaseCapture();
                POINT punto;
                GetCursorPos(&punto);
                ScreenToClient(hwnd, &punto);
                RECT rect;
                GetClientRect(hwnd, &rect);
                InvalidateRect(hwnd, NULL, FALSE);
                if (PtInRect(&rect, punto)) {
                    pulsarCromo(hwnd);
                }
            }
            return 0;
        case WM_KEYDOWN:
            if (wParam == VK_SPACE || wParam == VK_RETURN) {
                pulsarCromo(hwnd);
                return 0;
            }
            break;
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            bool activo = cromo->modo == 3 && cromo->tab == gPestana;
            if (cromo->modo == 3) {
                pintarFondoBarra(dc, hwnd);
                if (activo || cromo->hover) {
                    RECT pastilla = rect;
                    InflateRect(&pastilla, -4, -3);
                    COLORREF relleno = activo ? RGB(248, 242, 255) : RGB(124, 64, 186);
                    redondear(dc, pastilla, relleno, relleno, 16);
                }
            } else {
                COLORREF fondo = RGB(255, 255, 255);
                HBRUSH blanca = CreateSolidBrush(fondo);
                FillRect(dc, &rect, blanca);
                DeleteObject(blanca);
                RECT pastilla = rect;
                COLORREF relleno = RGB(109, 45, 214);
                COLORREF borde = relleno;
                COLORREF texto = RGB(255, 255, 255);
                if (cromo->modo == 1) {
                    relleno = cromo->hover ? RGB(244, 236, 255) : RGB(255, 255, 255);
                    borde = RGB(109, 45, 214);
                    texto = RGB(88, 32, 176);
                } else if (cromo->modo == 2) {
                    relleno = cromo->hover ? RGB(255, 236, 244) : RGB(255, 255, 255);
                    borde = RGB(176, 64, 112);
                    texto = RGB(146, 36, 82);
                } else if (cromo->hover) {
                    relleno = RGB(88, 28, 184);
                    borde = relleno;
                }
                if (cromo->down && cromo->modo == 0) {
                    relleno = RGB(70, 20, 150);
                    borde = relleno;
                }
                redondear(dc, pastilla, relleno, borde, 16);
                SetTextColor(dc, texto);
            }

            wchar_t texto[160];
            GetWindowTextW(hwnd, texto, 160);
            SetBkMode(dc, TRANSPARENT);
            HFONT fuente = (HFONT)SelectObject(dc, cromo->modo == 3 ? gFuente : gFuenteBoton);
            if (cromo->modo == 3) {
                SetTextColor(dc, activo ? RGB(72, 24, 138) : RGB(236, 226, 252));
                RECT zona = rect;
                zona.left += 22;
                DrawTextW(dc, texto, -1, &zona, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                wchar_t numero[8];
                wsprintfW(numero, L"%02d", cromo->tab + 1);
                zona.right -= 16;
                SetTextColor(dc, activo ? RGB(146, 92, 214) : RGB(214, 176, 132));
                SelectObject(dc, gFuenteMini);
                DrawTextW(dc, numero, -1, &zona, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
            } else {
                DrawTextW(dc, texto, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            if (GetFocus() == hwnd) {
                RECT foco = rect;
                InflateRect(&foco, -4, -4);
                DrawFocusRect(dc, &foco);
            }
            SelectObject(dc, fuente);
            EndPaint(hwnd, &ps);
            return 0;
        }
        default:
            break;
    }
    return DefWindowProcW(hwnd, mensaje, wParam, lParam);
}

LRESULT CALLBACK TarjetaProc(HWND hwnd, UINT mensaje, WPARAM wParam, LPARAM lParam) {
    if (mensaje == WM_ERASEBKGND) {
        return 1;
    }
    if (mensaje == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);
        HBRUSH fondo = CreateSolidBrush(RGB(244, 239, 250));
        FillRect(dc, &rect, fondo);
        DeleteObject(fondo);
        RECT sombra = rect;
        sombra.left += 5;
        sombra.top += 7;
        sombra.right -= 2;
        sombra.bottom -= 2;
        redondear(dc, sombra, RGB(214, 198, 232), RGB(214, 198, 232), 22);
        RECT cuerpo = rect;
        cuerpo.right -= 8;
        cuerpo.bottom -= 10;
        redondear(dc, cuerpo, RGB(255, 255, 255), RGB(226, 214, 242), 22);
        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProcW(hwnd, mensaje, wParam, lParam);
}

void pintarMetrica(HDC dc, RECT rect, const wchar_t* titulo, const wchar_t* valor) {
    redondear(dc, rect, RGB(255, 255, 255), RGB(226, 214, 242), 18);
    RECT etiqueta = rect;
    etiqueta.left += 16;
    etiqueta.top += 12;
    etiqueta.right -= 10;
    etiqueta.bottom = etiqueta.top + 20;
    SetTextColor(dc, RGB(132, 104, 168));
    HFONT anterior = (HFONT)SelectObject(dc, gFuenteMini);
    DrawTextW(dc, titulo, -1, &etiqueta, DT_LEFT | DT_SINGLELINE);
    RECT numero = rect;
    numero.left += 16;
    numero.top += 32;
    numero.right -= 10;
    SetTextColor(dc, RGB(78, 28, 156));
    SelectObject(dc, gFuenteMarca);
    DrawTextW(dc, valor, -1, &numero, DT_LEFT | DT_SINGLELINE);
    SelectObject(dc, anterior);
}

LRESULT CALLBACK MetricasProc(HWND hwnd, UINT mensaje, WPARAM wParam, LPARAM lParam) {
    if (mensaje == WM_ERASEBKGND) {
        return 1;
    }
    if (mensaje == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);
        HBRUSH fondo = CreateSolidBrush(RGB(244, 239, 250));
        FillRect(dc, &rect, fondo);
        DeleteObject(fondo);
        SetBkMode(dc, TRANSPARENT);

        const wchar_t* titulos[4];
        titulos[0] = L"CIUDADES";
        titulos[1] = L"RUTAS";
        titulos[2] = L"PASAJEROS";
        titulos[3] = L"EN COLA";
        wstring valores[4];
        valores[0] = ampliar(enteroATexto(contarCiudades(gCiudades)));
        valores[1] = ampliar(enteroATexto(contarRutas(gCiudades)));
        valores[2] = ampliar(enteroATexto(contarPasajeros(gPasajeros)));
        valores[3] = ampliar(enteroATexto(contarCola(gCola)));

        int separacion = 12;
        int ancho = (rect.right - separacion * 3) / 4;
        if (ancho < 40) {
            ancho = 40;
        }
        for (int i = 0; i < 4; i++) {
            RECT pieza;
            pieza.left = i * (ancho + separacion);
            pieza.top = 2;
            pieza.right = pieza.left + ancho;
            pieza.bottom = rect.bottom - 4;
            pintarMetrica(dc, pieza, titulos[i], valores[i].c_str());
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProcW(hwnd, mensaje, wParam, lParam);
}

LRESULT CALLBACK PieProc(HWND hwnd, UINT mensaje, WPARAM wParam, LPARAM lParam) {
    if (mensaje == WM_ERASEBKGND) {
        return 1;
    }
    if (mensaje == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);
        HBRUSH brocha = CreateSolidBrush(RGB(24, 8, 46));
        FillRect(dc, &rect, brocha);
        DeleteObject(brocha);
        RECT acento = rect;
        acento.bottom = acento.top + 2;
        HBRUSH linea = CreateSolidBrush(RGB(168, 96, 232));
        FillRect(dc, &acento, linea);
        DeleteObject(linea);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(236, 226, 252));
        HFONT anterior = (HFONT)SelectObject(dc, gFuenteMini);
        wchar_t texto[256];
        GetWindowTextW(hwnd, texto, 256);
        RECT zona = rect;
        zona.left = 22;
        zona.top += 2;
        DrawTextW(dc, L"SITRAC", -1, &zona, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        zona.left = 92;
        SetTextColor(dc, RGB(196, 176, 224));
        DrawTextW(dc, texto, -1, &zona, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        zona.left = 0;
        zona.right -= 18;
        SetTextColor(dc, RGB(214, 176, 132));
        DrawTextW(dc, L"Universidad Santo Tomas", -1, &zona, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        SelectObject(dc, anterior);
        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProcW(hwnd, mensaje, wParam, lParam);
}

const wchar_t* tituloPestana(int pestana) {
    if (pestana == TAB_RUT) return L"Rutas";
    if (pestana == TAB_PAS) return L"Pasajeros";
    if (pestana == TAB_COL) return L"Cola de solicitudes";
    if (pestana == TAB_HIS) return L"Historial";
    if (pestana == TAB_BUS) return L"Busquedas";
    if (pestana == TAB_REP) return L"Reportes";
    if (pestana == TAB_ARC) return L"Archivos";
    return L"Ciudades";
}

const wchar_t* subtituloPestana(int pestana) {
    if (pestana == TAB_RUT) return L"Rutas dirigidas y el camino mas corto entre dos ciudades";
    if (pestana == TAB_PAS) return L"Personas que pueden pedir un viaje";
    if (pestana == TAB_COL) return L"La primera solicitud es la siguiente en ser atendida";
    if (pestana == TAB_HIS) return L"Viajes atendidos, rechazados y cancelados";
    if (pestana == TAB_BUS) return L"Consulte la red, los pasajeros y los recorridos";
    if (pestana == TAB_REP) return L"Resumen de la operacion del sistema";
    if (pestana == TAB_ARC) return L"Guardar o volver a cargar la carpeta de datos";
    return L"Red de ciudades y conexiones de salida";
}

void pintarVentana(HDC dc, RECT rect) {
    HBRUSH fondo = CreateSolidBrush(RGB(244, 239, 250));
    FillRect(dc, &rect, fondo);
    DeleteObject(fondo);

    RECT barra;
    barra.left = 0;
    barra.top = 0;
    barra.right = NAV_ANCHO;
    barra.bottom = rect.bottom - PIE_ALTO;
    rellenarDegradado(dc, barra, RGB(96, 32, 168), RGB(28, 8, 52));

    RECT filo;
    filo.left = NAV_ANCHO - 1;
    filo.top = 0;
    filo.right = NAV_ANCHO;
    filo.bottom = barra.bottom;
    HBRUSH filoBrocha = CreateSolidBrush(RGB(214, 176, 132));
    FillRect(dc, &filo, filoBrocha);
    DeleteObject(filoBrocha);

    HBRUSH medalla = CreateSolidBrush(RGB(244, 236, 255));
    HPEN pluma = CreatePen(PS_SOLID, 1, RGB(244, 236, 255));
    HGDIOBJ viejaBrocha = SelectObject(dc, medalla);
    HGDIOBJ viejaPluma = SelectObject(dc, pluma);
    Ellipse(dc, 26, 28, 74, 76);
    SelectObject(dc, viejaBrocha);
    SelectObject(dc, viejaPluma);
    DeleteObject(medalla);
    DeleteObject(pluma);

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(88, 32, 168));
    HFONT anterior = (HFONT)SelectObject(dc, gFuenteMarca);
    RECT marca = {26, 36, 74, 72};
    DrawTextW(dc, L"S", -1, &marca, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SetTextColor(dc, RGB(255, 255, 255));
    SelectObject(dc, gFuenteTitulo);
    RECT nombre = {88, 26, NAV_ANCHO - 12, 58};
    DrawTextW(dc, L"SITRAC", -1, &nombre, DT_LEFT | DT_SINGLELINE);
    SetTextColor(dc, RGB(226, 208, 248));
    SelectObject(dc, gFuenteMini);
    RECT detalle = {88, 56, NAV_ANCHO - 16, 78};
    DrawTextW(dc, L"Transporte y rutas", -1, &detalle, DT_LEFT | DT_SINGLELINE);

    int x = NAV_ANCHO + 28;
    SetTextColor(dc, RGB(48, 16, 92));
    SelectObject(dc, gFuenteTitulo);
    RECT titulo = {x, 16, rect.right - 24, 52};
    DrawTextW(dc, tituloPestana(gPestana), -1, &titulo, DT_LEFT | DT_SINGLELINE);
    SetTextColor(dc, RGB(122, 96, 156));
    SelectObject(dc, gFuente);
    RECT sub = {x, 48, rect.right - 24, 74};
    DrawTextW(dc, subtituloPestana(gPestana), -1, &sub, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
    SelectObject(dc, anterior);
}

LRESULT pintarLista(NMHDR* aviso, LPARAM lParam) {
    HWND cabecera1 = (HWND)SendMessageW(gLista, LVM_GETHEADER, 0, 0);
    HWND cabecera2 = (HWND)SendMessageW(gLista2, LVM_GETHEADER, 0, 0);
    if (aviso->hwndFrom == cabecera1 || aviso->hwndFrom == cabecera2) {
        NMCUSTOMDRAW* dibujo = (NMCUSTOMDRAW*)lParam;
        if (dibujo->dwDrawStage == CDDS_PREPAINT) {
            return CDRF_NOTIFYITEMDRAW;
        }
        if (dibujo->dwDrawStage == CDDS_ITEMPREPAINT) {
            HBRUSH brocha = CreateSolidBrush(RGB(74, 32, 128));
            FillRect(dibujo->hdc, &dibujo->rc, brocha);
            DeleteObject(brocha);
            SetBkMode(dibujo->hdc, TRANSPARENT);
            SetTextColor(dibujo->hdc, RGB(255, 255, 255));
            HFONT anterior = (HFONT)SelectObject(dibujo->hdc, gFuenteMini);
            wchar_t texto[128];
            HDITEMW item;
            ZeroMemory(&item, sizeof(item));
            item.mask = HDI_TEXT;
            item.pszText = texto;
            item.cchTextMax = 128;
            texto[0] = 0;
            SendMessageW(aviso->hwndFrom, HDM_GETITEMW, dibujo->dwItemSpec, (LPARAM)&item);
            RECT zona = dibujo->rc;
            zona.left += 10;
            DrawTextW(dibujo->hdc, texto, -1, &zona, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            SelectObject(dibujo->hdc, anterior);
            return CDRF_SKIPDEFAULT;
        }
        return CDRF_DODEFAULT;
    }

    if (aviso->hwndFrom == gLista || aviso->hwndFrom == gLista2) {
        NMLVCUSTOMDRAW* dibujo = (NMLVCUSTOMDRAW*)lParam;
        if (dibujo->nmcd.dwDrawStage == CDDS_PREPAINT) {
            return CDRF_NOTIFYITEMDRAW;
        }
        if (dibujo->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
            bool seleccionado = (dibujo->nmcd.uItemState & CDIS_SELECTED) != 0;
            if (seleccionado) {
                dibujo->clrText = RGB(255, 255, 255);
                dibujo->clrTextBk = RGB(109, 45, 214);
            } else if ((dibujo->nmcd.dwItemSpec % 2) == 1) {
                dibujo->clrText = RGB(48, 22, 84);
                dibujo->clrTextBk = RGB(246, 240, 252);
            } else {
                dibujo->clrText = RGB(48, 22, 84);
                dibujo->clrTextBk = RGB(255, 255, 255);
            }
            return CDRF_NEWFONT;
        }
    }
    return CDRF_DODEFAULT;
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
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            if (rect.right > 0 && rect.bottom > 0) {
                HDC memoria = CreateCompatibleDC(dc);
                HBITMAP mapa = CreateCompatibleBitmap(dc, rect.right, rect.bottom);
                HGDIOBJ anterior = SelectObject(memoria, mapa);
                pintarVentana(memoria, rect);
                BitBlt(dc, 0, 0, rect.right, rect.bottom, memoria, 0, 0, SRCCOPY);
                SelectObject(memoria, anterior);
                DeleteObject(mapa);
                DeleteDC(memoria);
            }
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_SIZE: {
            int ancho = LOWORD(lParam);
            int alto = HIWORD(lParam);
            if (ancho > 0 && alto > 0) {
                organizar(ancho, alto);
            }
            return 0;
        }
        case WM_CTLCOLORSTATIC: {
            HDC dc = (HDC)wParam;
            SetTextColor(dc, RGB(86, 42, 148));
            SetBkColor(dc, RGB(255, 255, 255));
            return (LRESULT)gBrochaBlanca;
        }
        case WM_CTLCOLOREDIT: {
            HDC dc = (HDC)wParam;
            SetTextColor(dc, RGB(42, 16, 78));
            SetBkColor(dc, RGB(255, 255, 255));
            return (LRESULT)gBrochaBlanca;
        }
        case WM_CTLCOLORBTN: {
            HDC dc = (HDC)wParam;
            SetTextColor(dc, RGB(72, 32, 132));
            SetBkColor(dc, RGB(255, 255, 255));
            return (LRESULT)gBrochaBlanca;
        }
        case WM_CTLCOLORLISTBOX: {
            HDC dc = (HDC)wParam;
            SetTextColor(dc, RGB(42, 16, 78));
            SetBkColor(dc, RGB(255, 255, 255));
            return (LRESULT)gBrochaBlanca;
        }
        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == 0) {
                int id = LOWORD(wParam);
                if (id >= IDC_NAV && id < IDC_NAV + 8) {
                    mostrarPestana(id - IDC_NAV);
                    RECT rect;
                    GetClientRect(hwnd, &rect);
                    organizar(rect.right, rect.bottom);
                } else {
                    ejecutarComando(id);
                }
            }
            return 0;
        case WM_NOTIFY: {
            NMHDR* aviso = (NMHDR*)lParam;
            if (aviso->code == NM_CUSTOMDRAW) {
                return pintarLista(aviso, lParam);
            }
            if (aviso->idFrom == IDC_LISTA && aviso->code == LVN_ITEMCHANGED) {
                NMLISTVIEW* vista = (NMLISTVIEW*)lParam;
                if ((vista->uNewState & LVIS_SELECTED) != 0) {
                    alSeleccionar();
                }
            }
            return 0;
        }
        case WM_GETMINMAXINFO: {
            MINMAXINFO* info = (MINMAXINFO*)lParam;
            info->ptMinTrackSize.x = 1100;
            info->ptMinTrackSize.y = 700;
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
    controles.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_WIN95_CLASSES;
    InitCommonControlsEx(&controles);

    gBrochaFondo = CreateSolidBrush(RGB(244, 239, 250));
    gBrochaBlanca = CreateSolidBrush(RGB(255, 255, 255));
    gFuente = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    gFuenteBoton = CreateFontW(-15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    gFuenteMini = CreateFontW(-12, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                              OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    gFuenteTitulo = CreateFontW(-28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    gFuenteMarca = CreateFontW(-22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");

    WNDCLASSW cromoClase;
    ZeroMemory(&cromoClase, sizeof(cromoClase));
    cromoClase.lpfnWndProc = CromoProc;
    cromoClase.hInstance = instancia;
    cromoClase.lpszClassName = L"SITRACCromo";
    cromoClase.hCursor = LoadCursor(NULL, IDC_HAND);
    RegisterClassW(&cromoClase);

    WNDCLASSW tarjetaClase;
    ZeroMemory(&tarjetaClase, sizeof(tarjetaClase));
    tarjetaClase.lpfnWndProc = TarjetaProc;
    tarjetaClase.hInstance = instancia;
    tarjetaClase.lpszClassName = L"SITRACTarjeta";
    tarjetaClase.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&tarjetaClase);

    WNDCLASSW metricasClase;
    ZeroMemory(&metricasClase, sizeof(metricasClase));
    metricasClase.lpfnWndProc = MetricasProc;
    metricasClase.hInstance = instancia;
    metricasClase.lpszClassName = L"SITRACMetricas";
    RegisterClassW(&metricasClase);

    WNDCLASSW pieClase;
    ZeroMemory(&pieClase, sizeof(pieClase));
    pieClase.lpfnWndProc = PieProc;
    pieClase.hInstance = instancia;
    pieClase.lpszClassName = L"SITRACPie";
    RegisterClassW(&pieClase);

    WNDCLASSW clase;
    ZeroMemory(&clase, sizeof(clase));
    clase.lpfnWndProc = VentanaProc;
    clase.hInstance = instancia;
    clase.lpszClassName = L"SITRACVentana";
    clase.hCursor = LoadCursor(NULL, IDC_ARROW);
    clase.hbrBackground = NULL;
    clase.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassW(&clase);

    HWND ventana = CreateWindowExW(0, L"SITRACVentana",
                                   L"SITRAC  -  Sistema Inteligente de Transporte y Gestion de Rutas",
                                   WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
                                   1240, 780, NULL, NULL, instancia, NULL);
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

    if (gFuente != NULL) DeleteObject(gFuente);
    if (gFuenteTitulo != NULL) DeleteObject(gFuenteTitulo);
    if (gFuenteMarca != NULL) DeleteObject(gFuenteMarca);
    if (gFuenteMini != NULL) DeleteObject(gFuenteMini);
    if (gFuenteBoton != NULL) DeleteObject(gFuenteBoton);
    if (gBrochaFondo != NULL) DeleteObject(gBrochaFondo);
    if (gBrochaBlanca != NULL) DeleteObject(gBrochaBlanca);
    return (int)mensaje.wParam;
}
