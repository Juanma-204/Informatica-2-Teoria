#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

// ===================== CONFIGURACION ======================
const int SEED = 2;
const int METODO = 1;
const int TARIFA = 1000;

const string ARCH_USR_COD = "Usuario_Codificado.txt";
const string ARCH_USR_DEC = "Usuarios_Decodificados.txt";

// ================= FUNCIONES DE CODIFICACION =================
string caracterABits(unsigned char c) {
    string bits(8, '0');
    for (int i = 7; i >= 0; i--) bits[7 - i] = (c & (1 << i)) ? '1' : '0';
    return bits;
}

unsigned char bitsAChar(const string &bits) {
    unsigned char c = 0;
    for (int i = 0; i < 8; i++) c = (c << 1) | (bits[i] - '0');
    return c;
}

void aplicarMetodo1(string &bits, int total, int i, int n) {
    int len = min(n, total - i);
    if (i == 0) {
        for (int j = 0; j < len; j++) bits[i + j] = (bits[i + j] == '0') ? '1' : '0';
    } else {
        int unos = 0, ceros = 0;
        for (int j = i - n; j < i; j++) {
            if (bits[j] == '1') unos++; else ceros++;
        }
        if (unos == ceros) {
            for (int j = 0; j < len; j++) bits[i + j] = (bits[i + j] == '0') ? '1' : '0';
        } else if (ceros > unos) {
            for (int j = 0; j < len; j += 2) bits[i + j] = (bits[i + j] == '0') ? '1' : '0';
        } else {
            for (int j = 0; j < len; j += 3) bits[i + j] = (bits[i + j] == '0') ? '1' : '0';
        }
    }
}

void metodo1Codificar(string &bits, int total, int n) {
    for (int i = 0; i < total; i += n) aplicarMetodo1(bits, total, i, n);
}

void metodo1Decodificar(string &bits, int total, int n) {
    int start = total - (total % n == 0 ? n : total % n);
    for (int i = start; i >= 0; i -= n) {
        aplicarMetodo1(bits, total, i, n);
        if (i == 0) break;
    }
}

void codificarArchivo(const string &inFile, const string &outFile, int n, int metodo) {
    ifstream fin(inFile, ios::binary);
    if (!fin.is_open()) return;
    string bits;
    char c;
    while (fin.get(c)) bits += caracterABits((unsigned char)c);
    fin.close();

    int totalbits = bits.size();
    if (totalbits % n != 0) {
        int pad = n - (totalbits % n);
        bits.append(pad, '0');
        totalbits += pad;
    }
    if (metodo == 1) metodo1Codificar(bits, totalbits, n);

    ofstream fout(outFile, ios::binary);
    fout << bits;
    fout.close();
}

void decodificarArchivo(const string &inFile, const string &outFile, int n, int metodo) {
    ifstream fin(inFile, ios::binary);
    if (!fin.is_open()) {
        ofstream o(outFile);
        o.close();
        return;
    }
    string bits;
    char c;
    while (fin.get(c)) if (c == '0' || c == '1') bits += c;
    fin.close();

    int totalbits = bits.size();
    if (totalbits == 0) { ofstream o(outFile); o.close(); return; }
    int usable = (totalbits / 8) * 8;
    bits = bits.substr(0, usable);
    totalbits = bits.size();

    if (totalbits % n != 0) {
        int pad = n - (totalbits % n);
        bits.append(pad, '0');
        totalbits += pad;
    }

    if (metodo == 1) metodo1Decodificar(bits, totalbits, n);

    ofstream fout(outFile, ios::binary);
    for (int i = 0; i < totalbits; i += 8) {
        string byte = bits.substr(i, 8);
        fout.put(bitsAChar(byte));
    }
    fout.close();
}

// ====================== USUARIOS ======================
struct Usuario {
    string rol;
    string nombre;
    string clave;
    string cedula;
    int saldo;
};

vector<string> splitComas(const string &s) {
    vector<string> res;
    string temp;
    for (char c : s) {
        if (c == ',') { res.push_back(temp); temp.clear(); }
        else temp += c;
    }
    res.push_back(temp);
    return res;
}

Usuario parseLinea(const string &line) {
    Usuario u; u.saldo = 0;
    vector<string> v = splitComas(line);
    if (v.size() < 4) return u;
    u.rol = v[0];
    u.nombre = v[1];
    if (v.size() > 3) u.clave = v[3];

    for (size_t i = 0; i < v.size(); i++) {
        string campo = v[i];
        transform(campo.begin(), campo.end(), campo.begin(), ::tolower);
        if (campo == "cedula" && i + 1 < v.size()) u.cedula = v[i + 1];
        if (campo == "saldo" && i + 1 < v.size()) {
            try { u.saldo = stoi(v[i + 1]); } catch (...) { u.saldo = 0; }
        }
    }
    if (u.cedula.empty() && v.size() > 4) u.cedula = v[4];
    return u;
}

vector<Usuario> cargarUsuarios(const string &archivo) {
    vector<Usuario> lista;
    ifstream f(archivo);
    string linea;
    while (getline(f, linea)) {
        if (linea.empty()) continue;
        Usuario u = parseLinea(linea);
        if (!u.cedula.empty()) lista.push_back(u);
    }
    f.close();
    return lista;
}

void guardarUsuarios(const string &archivo, const vector<Usuario> &usuarios) {
    ofstream f(archivo);
    for (auto &u : usuarios) {
        f << u.rol << "," << u.nombre << ",Contrasena," << u.clave
          << ",Cedula," << u.cedula << ",Saldo," << u.saldo << "\n";
    }
    f.close();
}

// ====================== MENUS ======================
void menuUsuario(Usuario &u, vector<Usuario> &usuarios) {
    int opc;
    do {
        cout << "\n--- MENU USUARIO (" << u.nombre << ") ---\n";
        cout << "1. Consultar saldo\n2. Retirar dinero\n3. Salir\nOpcion: ";
        cin >> opc;
        if (opc == 1) {
            if (u.saldo >= TARIFA) {
                u.saldo -= TARIFA;
                cout << "Saldo actual: " << u.saldo << " (se desconto tarifa de 1000)\n";
            } else cout << "Saldo insuficiente para cubrir tarifa.\n";
        } else if (opc == 2) {
            int monto;
            cout << "Monto a retirar: ";
            cin >> monto;
            if (monto + TARIFA <= u.saldo) {
                u.saldo -= monto + TARIFA;
                cout << "Retiro exitoso. Saldo restante: " << u.saldo << "\n";
            } else cout << "Saldo insuficiente.\n";
        }
    } while (opc != 3);
}

void menuAdmin(vector<Usuario> &usuarios) {
    int opc;
    do {
        cout << "\n--- MENU ADMIN ---\n";
        cout << "1. Ver usuarios\n2. Registrar nuevo\n3. Salir\nOpcion: ";
        cin >> opc; cin.ignore();
        if (opc == 1) {
            for (auto &u : usuarios)
                cout << u.rol << " | " << u.nombre << " | Cedula: " << u.cedula
                     << " | Saldo: " << u.saldo << "\n";
        } else if (opc == 2) {
            Usuario nu;
            cout << "Rol: "; getline(cin, nu.rol);
            cout << "Nombre: "; getline(cin, nu.nombre);
            cout << "Clave: "; getline(cin, nu.clave);
            cout << "Cedula: "; getline(cin, nu.cedula);
            cout << "Saldo: "; cin >> nu.saldo; cin.ignore();
            usuarios.push_back(nu);
            guardarUsuarios(ARCH_USR_DEC, usuarios);
            codificarArchivo(ARCH_USR_DEC, ARCH_USR_COD, SEED, METODO);
            cout << "Usuario agregado.\n";
        }
    } while (opc != 3);
}

// ======================= MAIN =======================
int main() {
    cout << "=== SISTEMA BANCARIO ===\n";
    decodificarArchivo(ARCH_USR_COD, ARCH_USR_DEC, SEED, METODO);

    vector<Usuario> usuarios = cargarUsuarios(ARCH_USR_DEC);
    if (usuarios.empty()) {
        cout << "No se encontraron usuarios.\n";
        return 0;
    }

    cout << "Ingresar como (1) Administrador (2) Usuario: ";
    int tipo; cin >> tipo; cin.ignore();

    if (tipo == 1) {
        string clave;
        cout << "Clave admin: "; getline(cin, clave);
        bool ok = false;
        for (auto &u : usuarios) {
            if (u.rol == "Administrador" && u.clave == clave) {
                ok = true;
                menuAdmin(usuarios);
                break;
            }
        }
        if (!ok) cout << "Clave incorrecta.\n";
    } else if (tipo == 2) {
        string ced, clave;
        cout << "Cedula: "; getline(cin, ced);
        cout << "Clave: "; getline(cin, clave);
        bool ok = false;
        for (auto &u : usuarios) {
            if (u.cedula == ced && u.clave == clave) {
                ok = true;
                menuUsuario(u, usuarios);
                break;
            }
        }
        if (!ok) cout << "Credenciales invalidas.\n";
    }

    guardarUsuarios(ARCH_USR_DEC, usuarios);
    codificarArchivo(ARCH_USR_DEC, ARCH_USR_COD, SEED, METODO);

    cout << "Saliendo del sistema.\n";
    return 0;
}
