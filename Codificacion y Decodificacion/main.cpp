#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>

using namespace std;

string caracterabits(unsigned char c) {
    string bits(8, '0');
    for (int i = 7; i >= 0; i--) {
        bits[7 - i] = (c & (1 << i)) ? '1' : '0';
    }
    return bits;
}

unsigned char bitsacaracter(const string& bits) {
    unsigned char c = 0;
    for (int i = 0; i < 8; i++) {
        if (i < bits.length()) {
            c = (c << 1) | (bits[i] - '0');
        }
    }
    return c;
}

// Lógica de Metodo 1 (Aplicada tanto para codificar como decodificar, es simétrica)
void aplicarMetodo1(string& bits, int total, int i, int n) {
    int len = min(n, total - i);

    if (i == 0) {
        for (int j = 0; j < len; j++)
            bits[i+j] = (bits[i+j] == '0') ? '1' : '0';
    } else {
        int unos = 0, ceros = 0;
        // Analiza el bloque ANTERIOR (i - n) para decidir la transformación
        for (int j = i - n; j < i; j++) {
            if (bits[j] == '1') unos++; else ceros++;
        }

        if (unos == ceros) {
            for (int j = 0; j < len; j++)
                bits[i+j] = (bits[i+j]=='0')?'1':'0';
        } else if (ceros > unos) {
            for (int j = 0; j < len; j += 2)
                bits[i+j] = (bits[i+j]=='0')?'1':'0';
        } else { // unos > ceros
            for (int j = 0; j < len; j += 3)
                bits[i+j] = (bits[i+j]=='0')?'1':'0';
        }
    }
}

void metodo1Codificar(string& bits, int total, int n) {
    for (int i = 0; i < total; i += n) {
        aplicarMetodo1(bits, total, i, n);
    }
}

// CORRECCIÓN MÍNIMA: Para Decodificar el Método 1, se debe aplicar la misma
// lógica en ORDEN INVERSO para que el bloque anterior analizado sea el correcto.
void metodo1Decodificar(string& bits, int total, int n) {
    // i empieza en el último bloque que es múltiplo de n
    for (int i = total - (total % n == 0 ? n : total % n); i >= 0; i -= n) {
        aplicarMetodo1(bits, total, i, n);
        // La función aplicarMetodo1 usa el bloque ANTERIOR (i-n) para decidir.
        // Al ir en reversa, aplicamos el descifrado en el orden opuesto al cifrado.
    }
}


void metodo2(string& bits, int total, int n, bool esDecodificar) {
    for (int i = 0; i < total; i += n) {
        int len = min(n, total - i);
        if (len == 0) continue;

        string block = bits.substr(i, len);
        string newBlock = block;

        if (!esDecodificar) { // Codificar: Rotar a la izquierda
            newBlock[0] = block[len - 1];
            for (int j = 1; j < len; j++) {
                newBlock[j] = block[j - 1];
            }
        } else { // Decodificar: Rotar a la derecha (inversa de codificar)
            newBlock[len - 1] = block[0];
            for (int j = 0; j < len - 1; j++) {
                newBlock[j] = block[j + 1];
            }
        }

        bits.replace(i, len, newBlock);
    }
}

void codificar(string Archivodeentrada, string Archivodesalida, int n, int metodo) {
    ifstream fin(Archivodeentrada.c_str(), ios::binary);
    if (!fin) {
        cout << "Error, no se pudo abrir el archivo de entrada: " << Archivodeentrada << endl;
        return;
    }

    string bits;
    char c;

    while (fin.get(c)) {
        bits += caracterabits((unsigned char)c);
    }
    fin.close();

    int totaldebits = bits.length();

    if (totaldebits == 0) {
        cout << "Archivo de entrada vacío." << endl;
        return;
    }
    if (totaldebits % n != 0) {
        cout << "Error de formato: La cantidad total de bits (" << totaldebits
             << ") no es divisible por la semilla n (" << n << "). No se puede codificar correctamente." << endl;
        return;
    }

    if (metodo == 1) metodo1Codificar(bits, totaldebits, n); // Llama al nuevo método de codificación
    else if (metodo == 2) metodo2(bits, totaldebits, n, false);
    else {
        cout << "Error: Método de codificación no válido." << endl;
        return;
    }

    // APLICAMOS LA CORRECCIÓN: Abrir el archivo de salida en modo binario
    ofstream fout(Archivodesalida.c_str(), ios::binary);
    if (!fout) {
        cout << "Error, no se pudo abrir el archivo de salida: " << Archivodesalida << endl;
        return;
    }
    fout << bits;
    fout.close();

    cout << "Archivo codificado con el nombre de " << Archivodesalida << endl;
}

void decodificar(string Archivodeentrada, string Archivodesalida, int n, int metodo) {
    // CORRECCIÓN MÍNIMA: Usar ios::binary para leer los '0's y '1's sin que el sistema
    ifstream fin(Archivodeentrada.c_str(), ios::binary);
    if (!fin) {
        cout << "Error, no se pudo abrir el archivo de entrada: " << Archivodeentrada << endl;
        return;
    }

    string bits;
    char c;

    while (fin.get(c)) {
        if (c == '0' || c == '1') bits += c;
    }
    fin.close();

    int totaldebits = bits.length();

    if (totaldebits == 0 || totaldebits % 8 != 0) {
        cout << "Error: El archivo codificado es inválido o su longitud no es múltiplo de 8." << endl;
        return;
    }
    if (totaldebits % n != 0) {
        cout << "Error de formato: La cantidad total de bits (" << totaldebits
             << ") no es divisible por la semilla n (" << n << "). No se puede decodificar correctamente." << endl;
        return;
    }

    if (metodo == 1) metodo1Decodificar(bits, totaldebits, n); // Llama al nuevo método de decodificación
    else if (metodo == 2) metodo2(bits, totaldebits, n, true);
    else {
        cout << "Error: Método de decodificación no válido." << endl;
        return;
    }

    // Ya corregido: Abrir el archivo de salida en modo binario
    ofstream fout(Archivodesalida.c_str(), ios::binary);
    if (!fout) {
        cout << "Error, no se pudo abrir el archivo de salida: " << Archivodesalida << endl;
        return;
    }

    for (int i = 0; i < totaldebits; i += 8) {
        string byteBits = bits.substr(i, 8);
        fout.put(bitsacaracter(byteBits));
    }
    fout.close();

    cout << "Archivo decodificado con el nombre de " << Archivodesalida << endl;
}

struct Usuario {
    char nombre[50];
    char documento[20];
    char clave[20];
    char rol[7];
    int saldo;
};

int cargarUsuarios(Usuario usuarios[], int max) {
    ifstream f("usuarios.txt");
    int n = 0;
    char linea[200];

    while (f.getline(linea, 200) && n < max) {
        if (strlen(linea) == 0) continue;

        sscanf(linea, "%[^,],%[^,],%[^,],%[^,],%d",
               usuarios[n].nombre,
               usuarios[n].documento,
               usuarios[n].clave,
               usuarios[n].rol,
               &usuarios[n].saldo);

        n++;
    }

    f.close();
    return n;
}

void guardarUsuarios(Usuario usuarios[], int n) {
    ofstream f("usuarios.txt");
    for (int i = 0; i < n; i++) {
        f << usuarios[i].nombre << ","
          << usuarios[i].documento << ","
          << usuarios[i].clave << ","
          << usuarios[i].rol << ","
          << usuarios[i].saldo << "\n";
    }
    f.close();
}

int login(Usuario usuarios[], int n, string doc, string clave) {
    for (int i = 0; i < n; i++) {
        if (doc == usuarios[i].documento && clave == usuarios[i].clave) return i;
    }
    return -1;
}

void registrarTransaccion(Usuario u, string tipo, int valor) {
    ofstream f("transacciones.txt", ios::app);
    f << u.nombre << "," << u.documento << "," << tipo << "," << valor << "," << u.saldo << "\n";
    f.close();
}

void menuUsuario(Usuario &u) {
    int opcion;
    do {
        cout << "\n--- Menu del usuario (" << u.nombre << ") ---" << endl;
        cout << "1. Consultar tu saldo" << endl;
        cout << "2. Retirar plata" << endl;
        cout << "3. Consignar plata" << endl;
        cout << "4. Salir" << endl;
        cout << "Opcion: ";
        cin >> opcion;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            opcion = -1;
            cout << "Entrada inválida. Intente de nuevo." << endl;
            continue;
        }


        if (opcion == 1) {
            cout << "Su saldo actual es de: $" << u.saldo << endl;
            registrarTransaccion(u, "consulta", 0);
        } else if (opcion == 2) {
            int valor;
            cout << "Monto a retirar: $";
            cin >> valor;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Entrada inválida. Abortando retiro." << endl;
                continue;
            }
            if (valor <= u.saldo && valor > 0) {
                u.saldo -= valor;
                cout << "Retiro exitoso. Su nuevo saldo es de: $" << u.saldo << endl;
                registrarTransaccion(u, "retiro", valor);
            } else if (valor <= 0) {
                cout << "El valor a retirar debe ser positivo." << endl;
            } else {
                cout << "No tiene saldo suficiente para ese retiro." << endl;
            }
        } else if (opcion == 3) {
            int valor;
            cout << "Monto a consignar: $";
            cin >> valor;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Entrada inválida. Abortando consignación." << endl;
                continue;
            }
            if (valor > 0) {
                u.saldo += valor;
                cout << "Consignación exitosa. Su nuevo saldo es de: $" << u.saldo << endl;
                registrarTransaccion(u, "consignacion", valor);
            } else {
                cout << "El valor a consignar debe ser positivo." << endl;
            }
        }
    } while (opcion != 4);
    cin.ignore(10000, '\n');
}

void menuAdmin(Usuario usuarios[], int &n, int max) {
    int opcion;
    do {
        cout << "\n--- Menu de administrador ---"<<endl;
        cout << "1. Mostrar usuarios"<<endl;
        cout << "2. Crear un usuario"<<endl;
        cout << "3. Ver transacciones"<<endl;
        cout << "4. Salir"<<endl;
        cout << "Opcion: ";
        cin >> opcion; cin.ignore();

        if (opcion == 1) {
            cout << "\n--- Listado de Usuarios ---" << endl;
            for (int i = 0; i < n; i++) {
                cout << "Nombre: " << usuarios[i].nombre
                     << ", Documento: " << usuarios[i].documento
                     << ", Rol: " << usuarios[i].rol
                     << ", Saldo: $" << usuarios[i].saldo << endl;
            }
            cout << "---------------------------\n" << endl;
        } else if (opcion == 2) {
            if (n < max) {
                Usuario u;
                cout << "Nombre: "; cin.getline(u.nombre, 50);
                cout << "Documento: "; cin.getline(u.documento, 20);
                cout << "Clave: "; cin.getline(u.clave, 20);
                cout << "Rol (admin/usuario): "; cin.getline(u.rol, 7);
                cout << "Saldo inicial: "; cin >> u.saldo; cin.ignore();
                usuarios[n++] = u;
                cout << "Usuario creado exitosamente.\n" << endl;
            } else {
                cout << "Límite máximo de usuarios alcanzado (" << max << ")." << endl;
            }
        } else if (opcion == 3) {
            ifstream f("transacciones.txt");
            if (!f) {
                cout << "No se pudo abrir el archivo de transacciones." << endl;
                continue;
            }
            cout << "\n--- Historial de Transacciones ---" << endl;
            string linea;
            while (getline(f, linea)) cout << linea << "\n";
            f.close();
            cout << "----------------------------------\n" << endl;
        }
    } while (opcion != 4);
}

int main() {
    int opcion;
    do {
        cout << "\n===============================" << endl;
        cout << "       MENU PRINCIPAL" << endl;
        cout << "===============================" << endl;
        cout << "1. Codificacion de archivos"<<endl;
        cout << "2. Decodificacion de archivos"<<endl;
        cout << "3. Cajero automatico"<<endl;
        cout << "0. Salir"<<endl;
        cout << "Opcion: ";
        cin >> opcion;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            opcion = -1;
        } else {
            cin.ignore();
        }

        switch(opcion) {
        case 1: {
            string Archivodeentrada, Archivodesalida;
            int n, metodo;
            cout << "Coloque el archivo de entrada: "; getline(cin, Archivodeentrada);
            cout << "Coloque el archivo de salida: "; getline(cin, Archivodesalida);
            cout << "La semilla n: "; cin >> n;
            cout << "Escoja metodo 1 o 2: "; cin >> metodo; cin.ignore();
            codificar(Archivodeentrada, Archivodesalida, n, metodo);
            break;
        }
        case 2: {
            string Archivodeentrada, Archivodesalida;
            int n, metodo;
            cout << "Coloque el archivo de entrada (codificado): "; getline(cin, Archivodeentrada);
            cout << "Coloque el archivo de salida (decodificado): "; getline(cin, Archivodesalida);
            cout << "La semilla n: "; cin >> n;
            cout << "Escoja metodo 1 o 2: "; cin >> metodo; cin.ignore();
            decodificar(Archivodeentrada, Archivodesalida, n, metodo);
            break;
        }
        case 3: {
            const int MAX = 100;
            Usuario usuarios[MAX];
            int n_users = cargarUsuarios(usuarios, MAX);
            string doc, clave;
            cout << "Documento: "; getline(cin, doc);
            cout << "Clave: "; getline(cin, clave);
            int pos = login(usuarios, n_users, doc, clave);
            if (pos == -1) cout << "Credenciales incorrectas." << endl;
            else {
                Usuario &u = usuarios[pos];
                if (strcmp(u.rol, "usuario") == 0) menuUsuario(u);
                else menuAdmin(usuarios, n_users, MAX);
                guardarUsuarios(usuarios, n_users);
            }
            break;
        }
        case 0:
            cout << "Saliendo del programa." << endl;
            break;
        default:
            cout << "Opción inválida." << endl;
        }
    } while (opcion != 0);
    return 0;
}
