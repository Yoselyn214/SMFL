#include "laberinto.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
using namespace std;
int main() {
    // Declaracion de variables
    int filas,columnas, minF=3,maxF=50,minC=3,maxC=50, x0,y0,x1,y1;

    // Ingreso y validacion de datos
    valida("filas",filas,minF,maxF);
    valida("columnas",columnas,minC,maxC);
    Laberinto lab(filas,columnas);
    lab.print();
    // Ingreso de Punto de Incio y Punto Final
    validaEntrada(lab,x0,y0,x1,y1,filas,columnas);
    lab.reemplaza(x0,y0,x1,y1);

    vector<Laberinto> labs(3,lab);
    vector<string> algoritmos={"DFS","BFS","Dijkstra"};
    string origen = to_string(x0)+','+to_string(y0);
    string destino = to_string(x1)+','+to_string(y1);

    // Creacion del Grafo
    Grafo<string> g;
    g.nuevoGrafo(lab,filas,columnas);
    int opcion;
    cout << "----------------------\n"
         << "1. Algoritmo DFS\n"
         << "2. Algoritmo BFS\n"
         << "3. Algoritmo Dijkstra\n"
         << "----------------------\n";
    do {
        cout << "Ingrese una opcion: ";
        cin >> opcion;
    }while(opcion<1 || opcion>3);
    printAlgoritmo(g,labs[opcion-1],origen,destino,algoritmos[opcion-1]);
    return 0;
}
