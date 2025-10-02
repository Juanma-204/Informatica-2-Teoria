#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

// Estructura de usuario
struct Usuario {
    string rol;
    string nombre;
    string contrasena;
    string id;
};


string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");

    return str.substr(first, (last - first + 1));
}


string decodificar(const string &texto) {
    string resultado;
    for (unsigned char c : texto) {
        // 🚨 CAMBIO DE SEMILLA 🚨: Se cambia de -5 a -10 para una prueba más probable.
        resultado += (c - 10);
    }
    return resultado;
}

// ------------------ Cargar usuarios desde archivo ------------------

vector<Usuario> cargarUsuarios() {
    vector<Usuario> usuarios;
    ifstream archivo;
    const string RUTA_ABSOLUTA = "C:/Users/JuanManuel/Downloads/Informatica/Proyecto 3/Proyecto_de_admin_y_usuario/build/Desktop_Qt_6_9_2_MinGW_64_bit-Debug/debug/Informacion_Usuarios_c.txt";

    archivo.open(RUTA_ABSOLUTA);

    if (!archivo.is_open()) {
        // Si incluso la ruta absoluta falla, hay un problema de permisos.
        cout << "❌ ERROR CRITICO: No se pudo abrir el archivo usando la ruta absoluta.\n";
        cout << "   Verifique los permisos de lectura de la carpeta.\n";
        return usuarios;
    } else {
        cout << "Archivo de usuarios encontrado y procesando...\n";
    }

    string linea;
    while (getline(archivo, linea)) {
        if (linea.empty()) continue;

        stringstream ss(linea);
        string rol_c, nombre_c, contrasena_c, id_c;

        getline(ss, rol_c, ',');
        getline(ss, nombre_c, ',');
        getline(ss, contrasena_c, ',');
        getline(ss, id_c, ',');

        Usuario u;
        u.rol = trim(decodificar(rol_c));
        u.nombre = trim(decodificar(nombre_c));
        u.contrasena = trim(decodificar(contrasena_c));
        u.id = trim(decodificar(id_c));

        usuarios.push_back(u);
    }

    archivo.close();
    return usuarios;
}

// ------------------ Función de Login Mejorada ------------------

Usuario login(const vector<Usuario>& usuarios, const string& nombre, const string& contrasena) {
    string nombre_limpio = trim(nombre);
    string contrasena_limpia = trim(contrasena);

    for (const auto& u : usuarios) {
        if (u.nombre == nombre_limpio && u.contrasena == contrasena_limpia) {
            return u; // Usuario encontrado
        }
    }
    return Usuario{}; // Usuario vacío (no encontrado)
}

// ------------------ Main ------------------

int main() {
    // La prueba de depuración se queda para asegurar que el contenido es correcto
    vector<Usuario> usuarios = cargarUsuarios();

    if (usuarios.empty()) {
        return 0;
    }

    // 🚨 PRUEBA DE DEPURACIÓN 🚨: Muestra el contenido decodificado
    cout << "\n--- USUARIOS DECODIFICADOS EN MEMORIA ---\n";
    for (const auto& u : usuarios) {
        cout << "Nombre: [" << u.nombre
             << "] | Contraseña: [" << u.contrasena << "]\n";
    }
    cout << "-------------------------------------------\n";

    string nombre, contrasena;
    cout << "\n--- Login ---\n";
    cout << "Usuario: ";
    cin >> nombre;
    cout << "Contraseña: ";
    cin >> contrasena;

    Usuario usuario_logeado = login(usuarios, nombre, contrasena);

    if (usuario_logeado.nombre.empty()) {
        cout << "❌ Usuario o contraseña incorrectos.\n";
    } else {
        cout << "✅ Bienvenido " << usuario_logeado.rol << " " << usuario_logeado.nombre << "!\n";
    }

    return 0;
}

