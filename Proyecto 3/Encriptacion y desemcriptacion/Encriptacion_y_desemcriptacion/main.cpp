#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>

using namespace std;

// ---------- CONFIG ----------
const int MAX_USERS = 200;      // capacidad fija para usuarios en memoria
const int MAX_LINE = 1024;      // buffer para leer líneas
const char INPUT_FILE[] = "Informacion_Usuarios_c.txt";
const char TRANS_FILE[] = "transacciones.txt";
const int COSTO_OPERACION = 1000;

// ---------- UTILIDADES BINARIO <-> TEXTO ----------
// Comprueba si una cadena contiene solo '0' y '1'
bool esBinario(const char* s) {
    if (!s) return false;
    int i = 0;
    while (s[i] != '\0') {
        if (s[i] != '0' && s[i] != '1') return false;
        i++;
    }
    // longitud debe ser múltiplo de 8 para decodificar en ASCII
    return (i % 8) == 0 && i > 0;
}

// Decodifica una cadena de '0'/'1' (agrupada en bytes) a texto C (nuevo buffer, liberar con delete[])
char* binToText(const char* bin) {
    if (!bin) return NULL;
    int len = strlen(bin);
    if (len == 0 || (len % 8) != 0) return NULL;
    int bytes = len / 8;
    char* out = new char[bytes + 1];
    for (int b = 0; b < bytes; b++) {
        int val = 0;
        for (int j = 0; j < 8; j++) {
            val = (val << 1) + (bin[b*8 + j] - '0');
        }
        out[b] = (char)val;
    }
    out[bytes] = '\0';
    return out;
}

// Codifica texto a '0'/'1' (8 bits por char). Devuelve nuevo buffer (liberar con delete[]).
char* textToBin(const char* text) {
    if (!text) return NULL;
    int len = strlen(text);
    int outLen = len * 8;
    char* out = new char[outLen + 1];
    int p = 0;
    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)text[i];
        // convertir a 8 bits MSB primero
        for (int bit = 7; bit >= 0; bit--) {
            out[p++] = ((c >> bit) & 1) ? '1' : '0';
        }
    }
    out[p] = '\0';
    return out;
}

// ---------- PARSEO DE UNA LÍNEA CSV CODIFICADA ----------
// Tokeniza línea por coma, devuelve número de tokens y rellena tokens[] con new char[] (liberar después)
int splitCommaAlloc(const char* linea, char** tokens, int maxTokens) {
    if (!linea) return 0;
    int len = strlen(linea);
    char* buf = new char[len + 1];
    strcpy(buf, linea);

    int count = 0;
    char* cur = buf;
    char* start = cur;
    for (int i = 0; ; i++) {
        if (buf[i] == ',' || buf[i] == '\0') {
            buf[i] = '\0';
            if (count < maxTokens) {
                int tlen = strlen(start);
                tokens[count] = new char[tlen + 1];
                strcpy(tokens[count], start);
                count++;
            }
            if (buf[i] == '\0') break;
            start = &buf[i+1];
        }
    }
    delete[] buf;
    return count;
}

// libera tokens creados por splitCommaAlloc
void freeTokens(char** tokens, int n) {
    for (int i = 0; i < n; i++) {
        if (tokens[i]) delete[] tokens[i];
    }
}

// ---------- MEMORIA PARA USUARIOS (PARALELA) ----------
// rol[i] = "Administrador" o "Usuario"
// name[i], pass[i] son punteros a cadenas (dinámicas). saldo[i] almacena saldo (0 si no aplica)
char* rolArr[MAX_USERS];
char* nameArr[MAX_USERS];
char* passArr[MAX_USERS];
long saldoArr[MAX_USERS];
int userCount = 0;

// Inicializa arreglos vacíos
void initUsers() {
    for (int i = 0; i < MAX_USERS; i++) {
        rolArr[i] = NULL;
        nameArr[i] = NULL;
        passArr[i] = NULL;
        saldoArr[i] = 0;
    }
    userCount = 0;
}

// Agrega usuario en memoria (duplica las cadenas)
bool addUserMemory(const char* rol, const char* name, const char* pass, long saldo) {
    if (userCount >= MAX_USERS) return false;
    rolArr[userCount] = new char[strlen(rol) + 1];
    strcpy(rolArr[userCount], rol);
    nameArr[userCount] = new char[strlen(name) + 1];
    strcpy(nameArr[userCount], name);
    passArr[userCount] = new char[strlen(pass) + 1];
    strcpy(passArr[userCount], pass);
    saldoArr[userCount] = saldo;
    userCount++;
    return true;
}

// Liberar memoria de usuarios
void freeAllUsers() {
    for (int i = 0; i < userCount; i++) {
        if (rolArr[i]) delete[] rolArr[i];
        if (nameArr[i]) delete[] nameArr[i];
        if (passArr[i]) delete[] passArr[i];
    }
    userCount = 0;
}

// Buscar índice de usuario por nombre, -1 si no existe
int findUserIndex(const char* name) {
    if (!name) return -1;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(nameArr[i], name) == 0) return i;
    }
    return -1;
}

// ---------- LECTURA Y DECODIFICACIÓN DEL ARCHIVO Informacion_Usuarios_c.txt ----------
void cargarUsuariosDesdeArchivo() {
    ifstream fin(INPUT_FILE);
    if (!fin.is_open()) {
        cout << "No se encontro el archivo codificado '" << INPUT_FILE << "'.\n";
        return;
    }

    char linea[MAX_LINE];
    initUsers();

    while (fin.getline(linea, MAX_LINE)) {
        // saltar líneas vacías
        if (linea[0] == '\0') continue;

        // dividir por comas (asumiremos como máximo 10 tokens por línea)
        char* tokens[10];
        for (int t = 0; t < 10; t++) tokens[t] = NULL;
        int nTok = splitCommaAlloc(linea, tokens, 10);

        // interpretamos: tokens pueden mezclar texto plano y campos binarios
        // convertir tokens que son binarios (múltiplos de 8, solo 0/1) a texto
        for (int t = 0; t < nTok; t++) {
            if (esBinario(tokens[t])) {
                char* dec = binToText(tokens[t]);
                delete[] tokens[t];
                tokens[t] = dec;
            }
            // si no es binario, lo dejamos tal cual (ej: "Administrador", "saldo")
        }

        // Ahora interpretamos esquema práctico (según tus ejemplos):
        // Puede ser:
        // Administrador, <name>, Contracena, <pass>
        // Usuario, <name>, Contracena, <pass>, saldo, <saldo>
        // General: buscar "Administrador" o "Usuario" en tokens[0]
        if (nTok >= 4 && strcmp(tokens[0], "Administrador") == 0) {
            const char* nombre = tokens[1];
            const char* contr = tokens[3];
            addUserMemory("Administrador", nombre, contr, 0);
        } else if (nTok >= 6 && strcmp(tokens[0], "Usuario") == 0) {
            const char* nombre = tokens[1];
            const char* contr = tokens[3];
            long s = 0;
            // tokens[4] is "saldo" maybe, tokens[5] numeric
            if (nTok >= 6) s = atol(tokens[5]);
            addUserMemory("Usuario", nombre, contr, s);
        } else {
            // comportamiento por defecto: intentar parsear por posición
            if (nTok >= 4) {
                const char* nombre = tokens[1];
                const char* contr = tokens[3];
                long s = 0;
                if (nTok >= 6) s = atol(tokens[5]);
                addUserMemory(tokens[0], nombre, contr, s);
            }
        }

        freeTokens(tokens, nTok);
    }

    fin.close();
}

// ---------- GUARDAR NUEVO USUARIO EN ARCHIVO CODIFICADO ----------
// Construimos la línea (texto legible), la convertimos a binario por campos según formato original y la añadimos
void appendUserToEncodedFile(const char* rol, const char* nombre, const char* contrasena, long saldo) {
    // Formato a crear:
    // si Administrador:
    // Administrador,<bin(name)>,Contracena,<bin(pass)>
    // si Usuario:
    // Usuario,<bin(name)>,Contracena,<bin(pass)>,saldo,<saldo>
    char* binName = textToBin(nombre);
    char* binPass = textToBin(contrasena);

    ofstream fout(INPUT_FILE, ios::app);
    if (!fout.is_open()) {
        cout << "No se pudo abrir el archivo para agregar usuario.\n";
        delete[] binName;
        delete[] binPass;
        return;
    }

    if (strcmp(rol, "Administrador") == 0) {
        fout << "Administrador," << binName << ",Contracena," << binPass << "\n";
    } else {
        // escribir saldo en texto normal (no bin) para mantener compatibilidad como en tu ejemplo
        char numBuf[64];
        sprintf(numBuf, "%ld", saldo);
        fout << "Usuario," << binName << ",Contracena," << binPass << ",saldo," << numBuf << "\n";
    }

    fout.close();
    delete[] binName;
    delete[] binPass;
}

// ---------- REGISTRO DE TRANSACCIONES (CODIFICADO) ----------
void registrarTransaccionCodificada(const char* usuario, const char* operacion, long valor, long costo, long nuevoSaldo) {
    // crear linea legible primero
    char linea[512];
    sprintf(linea, "Usuario:%s | Operacion:%s | Valor:%ld | Costo:%ld | Nuevo saldo:%ld",
            usuario, operacion, valor, costo, nuevoSaldo);

    // codificar linea en binario ASCII
    char* bin = textToBin(linea);
    if (!bin) return;

    // guardar en transacciones.txt (en formato codificado)
    ofstream fout(TRANS_FILE, ios::app);
    if (!fout.is_open()) {
        cout << "No se pudo abrir " << TRANS_FILE << " para escribir.\n";
        delete[] bin;
        return;
    }
    fout << bin << "\n";
    fout.close();

    delete[] bin;
}

// ---------- MENÚ ADMIN ----------
void menuAdministradorInteractive(const char* adminName) {
    while (1) {
        cout << "\n--- MENU ADMINISTRADOR (" << adminName << ") ---\n";
        cout << "1) Listar usuarios\n";
        cout << "2) Agregar usuario\n";
        cout << "3) Salir admin\n";
        cout << "Seleccione opcion: ";
        int op;
        cin >> op;
        if (op == 1) {
            cout << "\nLista de usuarios:\n";
            for (int i = 0; i < userCount; i++) {
                cout << " - Rol: " << rolArr[i] << " | Nombre: " << nameArr[i];
                if (strcmp(rolArr[i], "Usuario") == 0) {
                    cout << " | Saldo: " << saldoArr[i];
                }
                cout << "\n";
            }
        } else if (op == 2) {
            // pedir datos
            char rolIn[20];
            char nombreIn[100];
            char passIn[100];
            long saldoIn = 0;
            cout << "Ingrese rol (Administrador/Usuario): ";
            cin >> rolIn;
            cout << "Ingrese nombre: ";
            cin >> nombreIn;
            cout << "Ingrese contrasena: ";
            cin >> passIn;
            if (strcmp(rolIn, "Usuario") == 0) {
                cout << "Ingrese saldo inicial (numero): ";
                cin >> saldoIn;
            }
            // agregar en memoria y en archivo codificado
            addUserMemory(rolIn, nombreIn, passIn, saldoIn);
            appendUserToEncodedFile(rolIn, nombreIn, passIn, saldoIn);
            cout << "Usuario agregado.\n";
        } else {
            break;
        }
    }
}

// ---------- MENÚ USUARIO ----------
void menuUsuarioInteractive(const char* username, int idxUser) {
    if (idxUser < 0) return;
    long saldo = saldoArr[idxUser];
    while (1) {
        cout << "\n--- MENU USUARIO (" << username << ") ---\n";
        cout << "1) Consultar saldo\n";
        cout << "2) Retirar dinero\n";
        cout << "3) Salir\n";
        cout << "Seleccione opcion: ";
        int op;
        cin >> op;
        if (op == 1) {
            // descontar costo
            if (saldo < COSTO_OPERACION) {
                cout << "No hay dinero suficiente para pagar el costo de ingreso (" << COSTO_OPERACION << ").\n";
            } else {
                saldo -= COSTO_OPERACION;
                saldoArr[idxUser] = saldo;
                cout << "Saldo despues de costo: " << saldo << "\n";
                registrarTransaccionCodificada(username, "Consulta", 0, COSTO_OPERACION, saldo);
            }
        } else if (op == 2) {
            long valor;
            cout << "Ingrese cantidad a retirar: ";
            cin >> valor;
            long total = valor + COSTO_OPERACION;
            if (valor <= 0) {
                cout << "Valor invalido.\n";
            } else if (saldo < total) {
                cout << "Fondos insuficientes (se requiere " << total << ").\n";
            } else {
                saldo -= total;
                saldoArr[idxUser] = saldo;
                cout << "Retiro exitoso. Nuevo saldo: " << saldo << "\n";
                registrarTransaccionCodificada(username, "Retiro", valor, COSTO_OPERACION, saldo);
            }
        } else {
            break;
        }
    }
}

// ---------- MAIN ----------
int main() {
    cout << "Iniciando sistema bancario (leer " << INPUT_FILE << ")...\n";
    cargarUsuariosDesdeArchivo();
    if (userCount == 0) {
        cout << "No hay usuarios cargados. Revise " << INPUT_FILE << " o agregue entradas codificadas.\n";
    }

    while (1) {
        cout << "\n--- MENU PRINCIPAL ---\n";
        cout << "1) Login\n";
        cout << "2) Salir\n";
        cout << "Seleccione opcion: ";
        int op;
        cin >> op;
        if (op == 2) break;
        if (op != 1) continue;

        char userIn[100], passIn[100];
        cout << "Usuario: ";
        cin >> userIn;
        cout << "Contrasena: ";
        cin >> passIn;

        int idx = findUserIndex(userIn);
        if (idx < 0) {
            cout << "Usuario no encontrado.\n";
            continue;
        }
        if (strcmp(passArr[idx], passIn) != 0) {
            cout << "Contrasena incorrecta.\n";
            continue;
        }

        // autenticado
        if (strcmp(rolArr[idx], "Administrador") == 0) {
            cout << "Bienvenido administrador " << nameArr[idx] << ".\n";
            menuAdministradorInteractive(nameArr[idx]);
        } else {
            cout << "Bienvenido usuario " << nameArr[idx] << ".\n";
            menuUsuarioInteractive(nameArr[idx], idx);
        }
    }

    // antes de salir, actualizar el archivo de usuarios no es necesario porque
    // agregamos en el archivo cada vez que el admin añade un usuario.
    freeAllUsers();
    cout << "Saliendo. Gracias por usar el sistema.\n";
    return 0;
}
