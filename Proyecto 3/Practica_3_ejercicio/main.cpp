#include <iostream>
#include <fstream>
using namespace std;

// =================================================================
// FUNCIONES DE CIPHER INVERSO (ROTACIÓN)
// =================================================================

// Rotación inversa (derecha) para revertir una rotación izquierda.
// n debe ser el mismo número de bits de rotación usado en el cifrado.
unsigned char rotaraladerecha(unsigned char b,int n){return (b>>n)|(b<<(8-n));}


// =================================================================
// FUNCIÓN DE LECTURA DE ARCHIVOS
// =================================================================

// Lee el archivo binario completo y retorna un buffer de bytes.
// Retorna nullptr si no puede abrir el archivo o si el tamaño es 0.
unsigned char* leer(const char* nombre,int &tam){
    // Abre el archivo en modo binario y se posiciona al final (ios::ate)
    ifstream f(nombre,ios::binary|ios::ate);
    if(!f) return nullptr;

    // Obtiene el tamaño del archivo
    tam=(int)f.tellg();
    if(tam<=0){f.close(); return nullptr;}

    // Regresa al inicio para leer
    f.seekg(0,ios::beg);

    // Asigna memoria y lee el contenido
    unsigned char* buf=new unsigned char[tam];
    f.read((char*)buf,tam);
    f.close();

    return buf;
}

// =================================================================
// FUNCIÓN DE DESCIFRADO (XOR)
// =================================================================

// Aplica la operación XOR a cada byte del buffer con la clave 'c'.
// Esta operación revierte el XOR aplicado en el cifrado.
void aplicarelxor(unsigned char* d,int tam,unsigned char c){for(int i=0;i<tam;i++) d[i]^=c;}


// =================================================================
// FUNCIÓN DE DESCOMPRESIÓN RLE
// =================================================================

// Descomprime datos codificados con Run-Length Encoding (RLE).
// Lee pares de bytes: [Carácter] y [Repeticiones].
char* descomprimirRLE(unsigned char* d,int tam,int &t){
    t=0; int cap=1024; char* out=new char[cap];

    // Itera sobre el buffer de entrada en pasos de 2 bytes (carácter, repeticiones)
    for(int i=0;i+1<tam;i+=2){
        char c=d[i];
        int r=d[i+1]; // El número de repeticiones

        for(int k=0;k<r;k++){
            // Comprueba si se necesita más capacidad en el buffer de salida
            if(t>=cap){
                cap*=2;
                char* tmp=new char[cap];
                for(int j=0;j<t;j++) tmp[j]=out[j];
                delete[] out;
                out=tmp;
            }
            out[t++]=c; // Añade el carácter repetido
        }
    }

    // Copia el contenido final a un nuevo buffer con el tamaño exacto y terminador nulo
    char* f=new char[t+1];
    for(int i=0;i<t;i++) f[i]=out[i];
    f[t]='\0';
    delete[] out;
    return f;
}

// =================================================================
// FUNCIÓN DE DESCOMPRESIÓN LZ78
// =================================================================

// Descomprime datos codificados con LZ78 (Algoritmo de Diccionario).
char* descomprimirLZ78(unsigned char* d,int tam,int &t){
    t=0; int cap=2048; char* out=new char[cap];

    const int MAX=65536; // Límite del diccionario de 16 bits
    char* dic[MAX]; int len[MAX]; int entradas=1;

    // Inicialización del diccionario: La entrada 0 es la cadena vacía
    dic[0]=new char[1]; dic[0][0]='\0'; len[0]=0;
    for(int i=1;i<MAX;i++) dic[i]=nullptr;

    int p=0; // Puntero de lectura en el buffer de entrada

    // El bucle lee ternas: (Hi byte, Lo byte, Carácter)
    while(p+2 < tam){
        int hi=d[p++],lo=d[p++];
        int idx=(hi<<8)|lo; // Combina los dos bytes para obtener el índice de 16 bits
        char c=d[p++]; // El carácter que se añade a la frase del diccionario

        // **VERIFICACIÓN CRÍTICA DE LZ78:** Si el índice es inválido,
        // la clave o rotación fue incorrecta.
        if(idx>=entradas || idx < 0) {
            cerr << "Error LZ78: Indice fuera de rango. Indice=0x" << hex << idx
                 << ", TamanoDic=" << dec << entradas << endl;
            // Limpia la memoria antes de salir
            for(int i=0;i<entradas;i++) if(dic[i]) delete[] dic[i];
            delete[] out;
            return nullptr;
        }

        // Construye la nueva frase: Frase_anterior + Caracter
        int nlen=len[idx]+1;
        char* n=new char[nlen]; // La nueva cadena a añadir a la salida y al diccionario

        for(int j=0;j<len[idx];j++) n[j]=dic[idx][j];
        n[nlen-1]=c; // Añade el nuevo carácter

        // Comprueba y redimensiona el buffer de salida si es necesario
        if(t+nlen>=cap){
            cap=t+nlen+1024;
            char* tmp=new char[cap];
            for(int j=0;j<t;j++) tmp[j]=out[j];
            delete[] out;
            out=tmp;
        }

        // Añade la nueva frase al buffer de salida
        for(int j=0;j<nlen;j++) out[t+j]=n[j];
        t+=nlen;

        // Añade la nueva frase al diccionario (si no está lleno)
        if(entradas<MAX){
            dic[entradas]=n;
            len[entradas]=nlen;
            entradas++;
        } else {
            // Si el diccionario está lleno, liberamos la memoria de la cadena creada
            delete[] n;
        }
    }

    // Copia el contenido final a un buffer con terminador nulo y limpia la memoria
    char* f=new char[t+1];
    for(int i=0;i<t;i++) f[i]=out[i];
    f[t]='\0';

    // Limpieza de memoria del diccionario y buffer temporal
    for(int i=0;i<entradas;i++) if(dic[i]) delete[] dic[i];
    delete[] out;

    return f;
}

// =================================================================
// FUNCIÓN DE MUESTRA DE DATOS
// =================================================================

// Muestra solo caracteres imprimibles (32 a 126).
void mostrar(char* buf,int t){
    for(int i=0;i<t;i++){
        char c=buf[i];
        if(c>=32&&c<=126) cout<<c;
        else cout<<"."; // Usa un punto para caracteres no imprimibles (común en RLE)
    }
    cout<<"\n";
}


// =VELOCIDAD DE EJECUCIÓN (MAIN)================================================
int main(){
    const char* files[4]={"Encriptado1.txt","Encriptado2.txt","Encriptado3.txt","Encriptado4.txt"};

    // CLAVES DE DESCIFRADO (Solución final encontrada)
    // E1: 0x5A, E2: 0x5A, E3: 0x40, E4: 0x5B
    const unsigned char claves[4]={0x5A,0x5A,0x40,0x5B};

    // ROTACIÓN (bits)
    // E1: 3, E2: 3, E3: 3, E4: 4
    const int rot[4]={3,3,3,4};

    // ORDEN DE DESCIFRADO
    // E1: Rotacion -> XOR (XORprimero=false)
    // E2: XOR -> Rotacion (XORprimero=true)
    // E3: Rotacion -> XOR (XORprimero=false)
    // E4: XOR -> Rotacion (XORprimero=true)
    const bool xorprimero[4]={false,true,false,true};

    // DIRECCIÓN DE ROTACIÓN (Siempre derecha para revertir la izquierda)
    const bool rotarderecha[4]={true,true,true,true};

    // ALGORITMO DE COMPRESIÓN (0=RLE, 1=LZ78)
    const int metodo[4]={0,1,0,1};

    for(int f=0;f<4;f++){
        cout << "========================================\n";
        cout << "Procesando Archivo: " << files[f] << endl;
        cout << "  Key=0x" << hex << (int)claves[f] << ", Rotacion=" << dec << rot[f]
             << ", Metodo=" << (metodo[f]==0?"RLE":"LZ78") << endl;
        cout << "========================================\n";

        int tam;
        unsigned char* buf=leer(files[f],tam);
        if(!buf){cout<<"ERROR: No se pudo abrir el archivo " << files[f] << endl; continue;}

        // 1. INVERTIR ORDEN DE LAS OPERACIONES DE CIFRADO
        if(xorprimero[f]){
            // Cifrado original fue: Rotacion -> XOR. Revertimos: XOR -> Rotacion Derecha.
            aplicarelxor(buf,tam,claves[f]);
            for(int i=0;i<tam;i++)
                buf[i]=rotarderecha[f]?rotaraladerecha(buf[i],rot[f]):rotaralaizquierda(buf[i],rot[f]);
        } else {
            // Cifrado original fue: XOR -> Rotacion. Revertimos: Rotacion Derecha -> XOR.
            for(int i=0;i<tam;i++)
                buf[i]=rotarderecha[f]?rotaraladerecha(buf[i],rot[f]):rotaralaizquierda(buf[i],rot[f]);
            aplicarelxor(buf,tam,claves[f]);
        }

        // 2. DESCOMPRESIÓN
        int tamaoriginal=0;
        char* msg = (metodo[f]==0)
                        ? descomprimirRLE(buf,tam,tamaoriginal)
                        : descomprimirLZ78(buf,tam,tamaoriginal);

        // 3. MUESTRA DE RESULTADOS Y LIMPIEZA
        if(msg){
            cout << "ÉXITO. Tamano Original: " << dec << tamaoriginal << " bytes.\n";
            cout << "Mensaje Descomprimido (Preview 1000 chars):\n";
            cout << "------------------------------------------\n";
            mostrar(msg, min(tamaoriginal, 1000)); // Muestra los primeros 1000 caracteres
            if (tamaoriginal > 1000) cout << "(continúa...)\n";
            cout << "------------------------------------------\n";
            delete[] msg;
        } else {
            cout << "ERROR: No se pudo descomprimir el archivo " << files[f] << endl;
            cout << "(" << (metodo[f]==0?"RLE":"LZ78") << " falló por índice fuera de rango o buffer vacío)\n";
        }

        delete[] buf;
    }

    cout << "\nFin del programa.\n";
    cin.ignore();
    cin.get();
}
