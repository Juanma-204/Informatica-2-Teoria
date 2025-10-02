#include <iostream>
#include <fstream>
using namespace std;

//funciones
unsigned char rotaralaizquierda(unsigned char b,int n){return (b<<n)|(b>>(8-n));}
unsigned char rotaraladerecha(unsigned char b,int n){return (b>>n)|(b<<(8-n));}


unsigned char* leer(const char* nombre,int &tam){
    ifstream f(nombre,ios::binary|ios::ate); if(!f) return nullptr;
    tam=(int)f.tellg(); if(tam<=0){f.close(); return nullptr;}
    f.seekg(0,ios::beg);
    unsigned char* buf=new unsigned char[tam]; f.read((char*)buf,tam); f.close();
    return buf;
}

void aplicarelxor(unsigned char* d,int tam,unsigned char c){for(int i=0;i<tam;i++) d[i]^=c;}

char* descomprimirRLE(unsigned char* d,int tam,int &t){
    t=0; int cap=1024; char* out=new char[cap];
    for(int i=0;i+1<tam;i+=2){
        char c=d[i]; int r=d[i+1];
        for(int k=0;k<r;k++){
            if(t>=cap){cap*=2; char* tmp=new char[cap]; for(int j=0;j<t;j++) tmp[j]=out[j]; delete[] out; out=tmp;}
            out[t++]=c;
        }
    }
    char* f=new char[t+1]; for(int i=0;i<t;i++) f[i]=out[i]; f[t]='\0'; delete[] out; return f;
}

char* descomprimirLZ78(unsigned char* d,int tam,int &t){
    t=0; int cap=2048; char* out=new char[cap];
    const int MAX=4096; char* dic[MAX]; int len[MAX]; int entradas=1;
    dic[0]=new char[1]; dic[0][0]='\0'; len[0]=0; for(int i=1;i<MAX;i++) dic[i]=nullptr;

    int p=0;
    while(p+2<=tam-1){
        int hi=d[p++],lo=d[p++]; int idx=(hi<<8)|lo; char c=d[p++];
        if(idx<0||idx>=entradas) break;
        int nlen=len[idx]+1; char* n=new char[nlen];

        for(int j=0;j<len[idx];j++) n[j]=dic[idx][j]; n[nlen-1]=c;

        if(t+nlen>=cap){cap=t+nlen+1024; char* tmp=new char[cap];
            for(int j=0;j<t;j++) tmp[j]=out[j]; delete[] out; out=tmp;}

        for(int j=0;j<nlen;j++) out[t+j]=n[j]; t+=nlen;
        if(entradas<MAX){dic[entradas]=n; len[entradas]=nlen; entradas++;} else delete[] n;
    }
    char* f=new char[t+1]; for(int i=0;i<t;i++) f[i]=out[i]; f[t]='\0';
    for(int i=0;i<entradas;i++) if(dic[i]) delete[] dic[i]; delete[] out;
    return f;
}

void mostrar(char* buf,int t){for(int i=0;i<t;i++){char c=buf[i]; if(c>=32&&c<=126) cout<<c;} cout<<"\n";}


//Main del codigo
int main(){
    const char* files[4]={"Encriptado1.txt","Encriptado2.txt","Encriptado3.txt","Encriptado4.txt"};
    const unsigned char claves[4]={0x5A,0x5A,0x5A,0x5A};
    const int rot[4]={3,3,3,3};
    const bool xorprimero[4]={false,true,false,true};
    const bool rotarderecha[4]={true,true,true,true};
    const int metodo[4]={0,1,0,1};

    for(int f=0;f<4;f++){
        cout<<"Los archivos"<<files[f]<< endl;
        int tam; unsigned char* buf=leer(files[f],tam);
        if(!buf){cout<<"No se pudo abrir el archivo"; continue;}

        if(xorprimero[f]){
            aplicarelxor(buf,tam,claves[f]);
            for(int i=0;i<tam;i++) buf[i]=rotarderecha[f]?rotaraladerecha(buf[i],rot[f]):rotaralaizquierda(buf[i],rot[f]);
        } else {
            for(int i=0;i<tam;i++) buf[i]=rotarderecha[f]?rotaraladerecha(buf[i],rot[f]):rotaralaizquierda(buf[i],rot[f]);
            aplicarelxor(buf,tam,claves[f]);
        }

        int tamaoriginal=0;
        char* msg=(metodo[f]==0)?descomprimirRLE(buf,tam,tamaoriginal):descomprimirLZ78(buf,tam,tamaoriginal);
        if(msg){ cout<<"El texto final descomprimido es:"; mostrar(msg,tamaoriginal); delete[] msg; }
        else cout<<"No se puedo descomprimir";

        delete[] buf;
    }

    cout<<"Dele enter para salir del menu"; cin.ignore(); cin.get();
}
