#ifndef LABERINTO_LABERINTO_H
#define LABERINTO_LABERINTO_H

#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <thread>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <climits>
#include <atomic>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
using namespace std;

// -- Creacion del Laberinto
class Laberinto {
private:
    vector<vector<char>> lab;
public:
    Laberinto(int fil,int col);
    void print()const;
    char getVal(int fil,int col);
    void reemplaza(int x0,int y0,int x1,int y1);
    void reemplazaCamino(string v);
    Laberinto& operator=(const Laberinto& lab2);
    void ActualizandoLaberinto();
    const vector<vector<char>>& getLab() const {
        return lab;
    }
};
// -- Validacion de Datos
void valida(string tipo,int& n, int min, int max);
void validaEntrada(Laberinto &lab,int& x0,int& y0,int& x1,int& y1,int filas,int columnas);

// -- Grafo
template<typename T>
struct Grafo{
    unordered_map<T,unordered_map<T, int>> g_map;
    unordered_set<T> visitados;
    unordered_map<T,int>distancias;
    priority_queue<pair<int,T>,vector<pair<int,T>>,greater<pair<int,T>>> no_visitados;
    mutex mux;

    void nuevaArista(const T& u,const T& v,int peso){
        lock_guard<mutex> lock(mux);
        g_map[u][v] = peso;
        g_map[v][u] = peso;
    }
    void nuevoGrafo(Laberinto &lab,int filas,int columnas){
        for(int i=0;i<filas;i++){
            for(int j=0;j<columnas;j++){
                if(lab.getVal(i,j)=='X' || lab.getVal(i,j)=='I' || lab.getVal(i,j)=='F' || lab.getVal(i,j)=='I'){
                    if(j+1<columnas && (lab.getVal(i,j+1)=='X' || lab.getVal(i,j+1)=='F' || lab.getVal(i,j+1)=='I')) {
                        nuevaArista(to_string(i+1) + ',' + to_string(j+1), to_string(i+1) + ',' + to_string(j + 2),1);
                    }
                    if(i+1<filas && (lab.getVal(i+1,j)=='X'||lab.getVal(i+1,j)=='F'|| lab.getVal(i+1,j)=='I')) {
                        nuevaArista(to_string(i+1) + ',' + to_string(j+1), to_string(i + 2) + ',' + to_string(j+1),1);
                    }
                }
            }
        }
    }
    void DFS(T actual, T fin,int pesoActual,int& pesoMinimo,vector<T> &res,vector<T> &caminoActual);
    void BFS(T actual, T fin,int pesoActual,int& pesoMinimo,vector<T>& res,vector<T>& caminoActual);
    unordered_map<T,T>dijkstra(T u);
    vector<T> caminoCorto(T inicio,T fin,string tipo);
};
void printAlgoritmo(Grafo<string> &g,Laberinto& lab,string origen,string destino,string tipo);
void dibujarLaberinto(const Laberinto& laberinto);
#endif //LABERINTO_LABERINTO_H