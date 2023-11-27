#include "laberinto.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

// --- Creacion del Laberinto
Laberinto::Laberinto(int fil, int col) {
    srand(time(NULL));
    lab.resize(fil,vector<char>(col,' '));
    for(int i=0;i<fil;i++)
        for(int j=0;j<col;j++)
            lab[i][j] = [](){return rand()%4!=0?'X':static_cast<char>(254);}();
}
char Laberinto::getVal(int fil, int col) {
    return lab[fil][col];
}
void Laberinto::print() const {
    for(const auto &fila:lab) {
        for (const auto &c:fila)
            cout << c << " ";
        cout << endl;
    }
    cout << endl;
}
void Laberinto::reemplaza(int x0, int y0, int x1, int y1) {
    if (x0-1<lab.size() && y0-1<lab[0].size() && x1-1<lab.size() && y1-1<lab[0].size()) {
        lab[x0 - 1][y0 - 1] = 'I';
        lab[x1 - 1][y1 - 1] = 'F';
    }else{
        cout << "No se puede reemplazar" << endl;
    }
}
void Laberinto::reemplazaCamino(string v) {
    int x = stoi(v.substr(0,v.find(',')));
    int y = stoi(v.substr(v.find(',')+1));
    lab[x-1][y-1] = '*';
}
Laberinto& Laberinto::operator=(const Laberinto& lab2) {
    if(this!=&lab2){
        lab = lab2.lab;
    }
    return *this;
}

// --- Validaciones de Datos
void valida(string tipo,int&n,int min,int max){
    do {
        cout << "Ingrese un numero de "<<tipo<<" entre "<<min<<" y "<<max<<": ";
        cin >> n;
    }    while (n<min || n>max);
}
void validaEntrada(Laberinto &lab,int& x0,int& y0,int& x1,int& y1,int filas,int columnas){
    do {
        valida("fila x0", x0, 1, filas);
        valida("columna y0", y0, 1, columnas);
        valida("fila x1", x1, 1, filas);
        valida("columna y1", y1, 1, columnas);
    } while(lab.getVal(x0-1,y0-1)==static_cast<char>(254) || lab.getVal(x1-1,y1-1)==static_cast<char>(254));
}
//------------------------------------------------------------------------------------------------------------------

// --- Grafo
template<typename T>
void Grafo<T>::DFS(T actual, T fin,int pesoActual,int& pesoMinimo,vector<T> &res,vector<T> &caminoActual) {
    if(visitados.find(actual) != visitados.end()) return;
    visitados.insert(actual);
    caminoActual.push_back(actual);
    if(actual==fin) {
        lock_guard<mutex> lock(mux);
        if (res.empty() || pesoActual < pesoMinimo) {
            res = caminoActual;
            pesoMinimo= pesoActual;
        }
    }
    for_each(g_map[actual].begin(),g_map[actual].end(),[&](auto& e){DFS(e.first,fin,pesoActual+e.second,pesoMinimo,res,caminoActual);});
    visitados.erase(actual);
    caminoActual.pop_back();
}
template<typename T>
void Grafo<T>::BFS(T actual, T fin,int pesoActual,int& pesoMinimo,vector<T>& res,vector<T>& caminoActual){
    queue<vector<T>> cola;
    cola.push({actual});

    while(!cola.empty()){
        caminoActual = cola.front();
        cola.pop();
        actual = caminoActual.back();

        if(actual == fin){
            lock_guard<mutex> lock(mux);
            if(res.empty() || pesoActual<pesoMinimo){
                res = caminoActual;
                pesoMinimo = pesoActual;
            }
        }

        vector<thread> levelThreads;
        for_each(g_map[actual].begin(),g_map[actual].end(),[&](auto& e){
            if(visitados.find(e.first)==visitados.end()){
                visitados.insert(e.first);
                vector<T> camino = caminoActual;
                camino.push_back(e.first);
                cola.push(camino);
            }
        });
        for_each(g_map[actual].begin(),g_map[actual].end(),[&](auto& e){
            if(visitados.find(e.first)==visitados.end()){
                levelThreads.emplace_back(&Grafo::BFS,this,e.first,fin,pesoActual+e.second,ref(pesoMinimo),ref(res),ref(caminoActual));
            }
        });
        for(auto &t:levelThreads){
            t.join();
        }
    }
}
template<typename T>
unordered_map<T,T>Grafo<T>::dijkstra(T u){
    unordered_map<T,T> prev;
    for(const auto& v:g_map)
        distancias[v.first] = INT_MAX;
    distancias[u] = 0;
    prev[u] = u;
    no_visitados.push(pair<int,T>(0,u));

    auto t1=chrono::high_resolution_clock::now();
    while(no_visitados.size()!=0){
        T v = no_visitados.top().second;
        int distanciaActual = no_visitados.top().first;
        no_visitados.pop();

        if(distanciaActual> distancias[v]) continue;
        for_each(g_map[v].begin(),g_map[v].end(),[&](auto& e){
            if(distancias[e.first] > distancias[v] + e.second){
                distancias[e.first] = distancias[v] + e.second;
                prev[e.first] = v;
                no_visitados.push(pair<int,T>(distancias[e.first],e.first));
            }
        });
    }
    auto t2=chrono::high_resolution_clock::now();
    auto duration=chrono::duration_cast<chrono::microseconds>(t2-t1);
    cout << "Tiempo de ejecucion: " << duration.count() << " microsegundos" << endl;
    return prev;
}
template<typename T>
vector<T>Grafo<T>::caminoCorto(T inicio,T fin,string tipo) {
    vector<thread> threads;
    vector<T> res;
    vector<T> caminoActual;
    int pesoMinimo = 0;

    auto t1=chrono::high_resolution_clock::now();
    for(int i=0; i<2;i++){
        if(tipo=="DFS")
            threads.emplace_back(&Grafo::DFS,this,inicio,fin,0,ref(pesoMinimo),ref(res),ref(caminoActual));
        else if(tipo=="BFS")
            threads.emplace_back(&Grafo::BFS,this,inicio,fin,0,ref(pesoMinimo),ref(res),ref(caminoActual));
    }
    for(auto &t:threads){
        t.join();
    }
    auto t2=chrono::high_resolution_clock::now();
    auto duration=chrono::duration_cast<chrono::microseconds>(t2-t1);
    cout << "Tiempo de ejecucion: " << duration.count() << " microsegundos" << endl;
    //DFS(inicio,fin,res,caminoActual);
    return res;
}
void Laberinto::ActualizandoLaberinto() {
    for (int i = 0; i < lab.size(); ++i) {
        for (int j = 0; j < lab[i].size(); ++j) {
            // Anotaciones: '*' se convierte en morado, 'I' en azul, 'F' en verde, 'X' en amarillo y lo demas en blanco
            switch (lab[i][j]) {
                case '*':
                    lab[i][j] = 'P';  
                    break;
                case 'I':
                    lab[i][j] = 'B'; 
                    break;
                case 'F':
                    lab[i][j] = 'G';
                    break;
                case 'X':
                    lab[i][j] = 'Y'; 
                    break;
                default:
                    lab[i][j] = 'W'; 
                    break;
            }
        }
    }
}
void dibujarLaberinto(const Laberinto& laberinto) {
    sf::RenderWindow window(sf::VideoMode(800, 600), "GATOLABERINTO");

    // Tamaño de los cuadrados
    const float squareSize = 20.0f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();

        const vector<vector<char>>& currentLab = laberinto.getLab();


        for (size_t i = 0; i < currentLab.size(); ++i) {
            for (size_t j = 0; j < currentLab[i].size(); ++j) {
                sf::RectangleShape square(sf::Vector2f(squareSize, squareSize));
                square.setPosition(j * squareSize, i * squareSize);  // Se ajusta la posición según la cuadrícula

                switch (currentLab[i][j]) {
                    case 'P':
                        square.setFillColor(sf::Color(128, 0, 128));
                        break;
                    case 'B':
                        square.setFillColor(sf::Color::Blue);
                        break;
                    case 'G':
                        square.setFillColor(sf::Color::Green);
                        break;
                    case 'Y':
                        square.setFillColor(sf::Color::Yellow);
                        break;
                    default:
                        square.setFillColor(sf::Color::White);
                        break;
                }
                window.draw(square);
            }
        }
        window.display();
    }
}

void printAlgoritmo(Grafo<string> &g,Laberinto& lab,string origen,string destino,string tipo){
    if (tipo!="Dijkstra") {
        auto res = g.caminoCorto(origen, destino, tipo);
        if (!res.empty()) {
            cout << "Camino mas corto " << tipo << ": " << endl;
            for (int i = 1; i < res.size() - 1; i++)
                lab.reemplazaCamino(res[i]);

            lab.ActualizandoLaberinto();
            dibujarLaberinto(lab);
        } else {
            cout << "No hay camino" << endl;
        }
    }else{
        unordered_map<string,string> padres = g.dijkstra(origen);
        if (padres.find(destino) == padres.end()) {
            cout << "No hay camino" << endl;
        } else {
            vector<string>path;
            string current = destino;
            while (current != origen && padres.find(current) != padres.end()) {
                path.emplace_back(current);
                current = padres[current];
            }
            path.emplace_back(origen);
            cout << "Camino mas corto "<<tipo<<": " << endl;
            int pathSize = path.size()-2;
            for (auto i = pathSize; i > 0; i--)
                lab.reemplazaCamino(path[i]);
            lab.ActualizandoLaberinto();
            dibujarLaberinto(lab);
        }
    }
    cout << endl;
}
