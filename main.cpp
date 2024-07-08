#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <stack>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <stdexcept>

using namespace std;

// Definición de la clase Pelicula
class Pelicula {
private:
    string imdb_id;
    string titulo;
    string sinopsis;
    vector<string> tags;

public:
    Pelicula(const string& imdb_id, const string& titulo, const string& sinopsis, const vector<string>& tags)
        : imdb_id(imdb_id), titulo(titulo), sinopsis(sinopsis), tags(tags) {}

    string getTitulo() const { return titulo; }
    string getSinopsis() const { return sinopsis; }
    const vector<string>& getTags() const { return tags; }
    string getImdbId() const { return imdb_id; }
};

// Definición de la clase Plataforma
class Plataforma {
private:
    vector<shared_ptr<Pelicula>> peliculas;
    map<string, shared_ptr<Pelicula>> peliculasPorTitulo;
    map<string, shared_ptr<Pelicula>> peliculasPorSinopsis;
    multimap<string, shared_ptr<Pelicula>> peliculasPorTag;
    vector<shared_ptr<Pelicula>> verMasTarde;
    vector<shared_ptr<Pelicula>> likes;
    stack<vector<shared_ptr<Pelicula>>> paginacion;

public:
    // Función para cargar las películas desde un archivo CSV
    void cargarPeliculas(const string& archivo) {
        ifstream file(archivo);
        if (!file.is_open()) {
            cerr << "No se pudo abrir el archivo " << archivo << ". Verifique que la ruta sea correcta y que el archivo exista." << endl;
            return;
        }

        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string imdb_id, titulo, sinopsis, tags_str;
            getline(ss, imdb_id, ',');
            getline(ss, titulo, ',');
            getline(ss, sinopsis, ',');
            getline(ss, tags_str);

            vector<string> tags;
            stringstream tags_ss(tags_str);
            string tag;
            while (getline(tags_ss, tag, '|')) {
                tags.push_back(tag);
            }

            auto pelicula = make_shared<Pelicula>(imdb_id, titulo, sinopsis, tags);
            peliculas.push_back(pelicula);
            peliculasPorTitulo[titulo] = pelicula;
            peliculasPorSinopsis[sinopsis] = pelicula;
            for (const auto& t : tags) {
                peliculasPorTag.insert({t, pelicula});
            }
        }
        file.close();
        cout << "Películas cargadas correctamente desde " << archivo << endl;
    }

    // Función para buscar películas por título o sinopsis
    vector<shared_ptr<Pelicula>> buscarPorPalabraOFrase(const string& palabra) {
        vector<shared_ptr<Pelicula>> resultados;

        for (const auto& [key, pelicula] : peliculasPorTitulo) {
            if (key.find(palabra) != string::npos) {
                resultados.push_back(pelicula);
            }
        }

        for (const auto& [key, pelicula] : peliculasPorSinopsis) {
            if (key.find(palabra) != string::npos && find(resultados.begin(), resultados.end(), pelicula) == resultados.end()) {
                resultados.push_back(pelicula);
            }
        }

        paginacion.push(resultados);
        return resultados;
    }

    // Función para buscar películas por etiqueta (tag)
    vector<shared_ptr<Pelicula>> buscarPorTag(const string& tag) {
        auto range = peliculasPorTag.equal_range(tag);
        vector<shared_ptr<Pelicula>> resultados;

        for (auto it = range.first; it != range.second; ++it) {
            resultados.push_back(it->second);
        }

        paginacion.push(resultados);
        return resultados;
    }

    // Función pública para mostrar películas con paginación
    void mostrarPeliculas(const vector<shared_ptr<Pelicula>>& pelis, int start = 0, int count = 5) const {
        int end = min(start + count, static_cast<int>(pelis.size()));
        for (int i = start; i < end; ++i) {
            cout << setw(3) << i + 1 << ". " << pelis[i]->getTitulo() << endl;
        }
        if (end < static_cast<int>(pelis.size())) {
            cout << "Más resultados disponibles. Ingresa 'm' para ver más o 'q' para salir: ";
            string input;
            cin.ignore();
            getline(cin, input);
            if (input == "m") {
                mostrarPeliculas(pelis, start + count, count);
            }
        }
    }

    // Función para mostrar detalles de una película según su índice
    void verDetalles(int indice) {
        if (indice < 1 || indice > static_cast<int>(peliculas.size())) {
            cout << "Índice inválido." << endl;
            return;
        }
        auto pelicula = peliculas[indice - 1];
        cout << "Detalles de la película:" << endl;
        cout << "Título: " << pelicula->getTitulo() << endl;
        cout << "ID IMDB: " << pelicula->getImdbId() << endl;
        cout << "Sinopsis: " << pelicula->getSinopsis() << endl;
        cout << "Tags: ";
        for (const auto& tag : pelicula->getTags()) {
            cout << tag << " ";
        }
        cout << endl;

        cout << "1. Like\n2. Ver más tarde\n3. Volver al menú principal\nSelecciona una opción: ";
        int opcion;
        cin >> opcion;
        if (opcion == 1) {
            likes.push_back(pelicula);
            cout << "Película agregada a tus likes." << endl;
        } else if (opcion == 2) {
            verMasTarde.push_back(pelicula);
            cout << "Película agregada a 'Ver más tarde'." << endl;
        }
    }

    // Función para sugerir películas similares basadas en los likes del usuario
    void sugerirPeliculasSimilares() const {
        if (likes.empty()) {
            cout << "Aún no has dado like a ninguna película. Da like a algunas películas para obtener sugerencias." << endl;
            return;
        }

        set<string> tagsLiked;
        for (const auto& pelicula : likes) {
            for (const auto& tag : pelicula->getTags()) {
                tagsLiked.insert(tag);
            }
        }

        vector<shared_ptr<Pelicula>> sugerencias;
        for (const auto& [tag, pelicula] : peliculasPorTag) {
            if (tagsLiked.find(tag) != tagsLiked.end()) {
                sugerencias.push_back(pelicula);
            }
        }

        sort(sugerencias.begin(), sugerencias.end(), [](const shared_ptr<Pelicula>& a, const shared_ptr<Pelicula>& b) {
            return a->getTitulo() < b->getTitulo();
        });

        cout << "Sugerencias de películas similares basadas en tus likes:" << endl;
        mostrarPeliculas(sugerencias);
    }

    // Función para mostrar las películas marcadas para ver más tarde
    void mostrarVerMasTarde() const {
        if (verMasTarde.empty()) {
            cout << "Lista 'Ver más tarde' está vacía." << endl;
            return;
        }
        cout << "Películas marcadas para ver más tarde:" << endl;
        mostrarPeliculas(verMasTarde);
    }

    // Función para mostrar las películas que me gustan
    void mostrarLikes() const {
        if (likes.empty()) {
            cout << "Lista de likes está vacía." << endl;
            return;
        }
        cout << "Películas que te gustan:" << endl;
        mostrarPeliculas(likes);
    }

    // Función para guardar y cargar las listas de "Ver más tarde" y "Likes"
    void cargarEstado(const string& archivo, vector<shared_ptr<Pelicula>>& lista) {
        ifstream file(archivo);
        if (!file.is_open()) {
            cerr << "No se pudo abrir el archivo " << archivo << endl;
            return;
        }

        string imdb_id;
        while (getline(file, imdb_id)) {
            auto it = find_if(peliculas.begin(), peliculas.end(), [&imdb_id](const shared_ptr<Pelicula>& p) {
                return p->getImdbId() == imdb_id;
            });
            if (it != peliculas.end()) {
                lista.push_back(*it);
            }
        }
        file.close();
    }

    void guardarEstado(const string& archivo, const vector<shared_ptr<Pelicula>>& lista) const {
        ofstream file(archivo);
        if (!file.is_open()) {
            cerr << "No se pudo abrir el archivo " << archivo << endl;
            return;
        }

        for (const auto& pelicula : lista) {
            file << pelicula->getImdbId() << endl;
        }
        file.close();
    }

    void cargarVerMasTarde() {
        cargarEstado("ver_mas_tarde.txt", verMasTarde);
    }

    void guardarVerMasTarde() const {
        guardarEstado("ver_mas_tarde.txt", verMasTarde);
    }

    void cargarLikes() {
        cargarEstado("likes.txt", likes);
    }

    void guardarLikes() const {
        guardarEstado("likes.txt", likes);
    }
};

// Función principal para el menú
void mostrarMenu(Plataforma& plataforma) {
    int opcion;
    do {
        cout << "Menú:\n";
        cout << "1. Buscar películas por título o sinopsis\n";
        cout << "2. Buscar películas por tag\n";
        cout << "3. Ver detalles de una película\n";
        cout << "4. Sugerir películas similares\n";
        cout << "5. Mostrar Ver más tarde\n";
        cout << "6. Ver mis likes\n";
        cout << "7. Salir\n";
        cout << "Selecciona una opción: ";
        cin >> opcion;

        switch (opcion) {
            case 1: {
                cout << "Ingresa una palabra o frase para buscar: ";
                string palabra;
                cin.ignore();
                getline(cin, palabra);
                auto resultados = plataforma.buscarPorPalabraOFrase(palabra);
                cout << "Resultados de la búsqueda (" << resultados.size() << " encontrados):" << endl;
                plataforma.mostrarPeliculas(resultados);
                break;
            }
            case 2: {
                cout << "Ingresa una etiqueta (tag) para buscar: ";
                string tag;
                cin >> tag;
                auto resultados = plataforma.buscarPorTag(tag);
                cout << "Resultados de la búsqueda por tag (" << resultados.size() << " encontrados):" << endl;
                plataforma.mostrarPeliculas(resultados);
                break;
            }
            case 3: {
                cout << "Ingresa el índice de la película para ver detalles: ";
                int indice;
                cin >> indice;
                plataforma.verDetalles(indice);
                break;
            }
            case 4: {
                plataforma.sugerirPeliculasSimilares();
                break;
            }
            case 5: {
                plataforma.mostrarVerMasTarde();
                break;
            }
            case 6: {
                plataforma.mostrarLikes();
                break;
            }
            case 7:
                cout << "Saliendo del programa." << endl;
                break;
            default:
                cout << "Opción inválida. Por favor, selecciona una opción válida." << endl;
        }
    } while (opcion != 7);
}

int main() {
    Plataforma plataforma;
    try {
        plataforma.cargarPeliculas("C:/Users/fbver/Documents/vscode/progra3-proyectofinal/mpst_full_data.csv");
    } catch (const runtime_error& e) {
        cerr << e.what() << endl;
        return 1;
    }

    plataforma.cargarVerMasTarde();
    plataforma.cargarLikes();
    mostrarMenu(plataforma);
    plataforma.guardarVerMasTarde();
    plataforma.guardarLikes();

    return 0;
}

