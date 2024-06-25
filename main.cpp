#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <set>

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
    unordered_map<string, vector<shared_ptr<Pelicula>>> peliculasPorTitulo;
    multimap<string, shared_ptr<Pelicula>> peliculasPorTag;
    vector<shared_ptr<Pelicula>> verMasTarde;
    vector<shared_ptr<Pelicula>> likes;

    // Función privada para mostrar las películas con paginación
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

public:
    // Función para cargar las películas desde un archivo CSV
    void cargarPeliculas(const string& archivo) {
        ifstream file(archivo);
        if (!file.is_open()) {
            cerr << "No se pudo abrir el archivo " << archivo << endl;
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
            peliculasPorTitulo[titulo].push_back(pelicula);
            for (const auto& t : tags) {
                peliculasPorTag.insert({t, pelicula});
            }
        }
        file.close();
    }

    // Función para buscar películas por título o sinopsis
    void buscarPorPalabraOFrase(const string& palabra) {
        vector<shared_ptr<Pelicula>> resultados;
        string palabraLower = palabra;
        transform(palabraLower.begin(), palabraLower.end(), palabraLower.begin(), ::tolower);

        for (const auto& p : peliculas) {
            string tituloLower = p->getTitulo();
            string sinopsisLower = p->getSinopsis();
            transform(tituloLower.begin(), tituloLower.end(), tituloLower.begin(), ::tolower);
            transform(sinopsisLower.begin(), sinopsisLower.end(), sinopsisLower.begin(), ::tolower);

            if (tituloLower.find(palabraLower) != string::npos || sinopsisLower.find(palabraLower) != string::npos) {
                resultados.push_back(p);
            }
        }
        cout << "Resultados de la búsqueda (" << resultados.size() << " encontrados):" << endl;
        mostrarPeliculas(resultados);
    }

    // Función para buscar películas por etiqueta (tag)
    void buscarPorTag(const string& tag) {
        auto range = peliculasPorTag.equal_range(tag);
        vector<shared_ptr<Pelicula>> resultados;
        for (auto it = range.first; it != range.second; ++it) {
            resultados.push_back(it->second);
        }
        cout << "Resultados de la búsqueda por tag (" << resultados.size() << " encontrados):" << endl;
        mostrarPeliculas(resultados);
    }

    // Función para mostrar las películas más relevantes
    void mostrarPeliculasRelevantes(int count = 5) const {
        cout << "Películas más relevantes:" << endl;
        vector<shared_ptr<Pelicula>> peliculasOrdenadas = peliculas;
        sort(peliculasOrdenadas.begin(), peliculasOrdenadas.end(), [](const shared_ptr<Pelicula>& a, const shared_ptr<Pelicula>& b) {
            // Aquí puedes implementar tu propio criterio de relevancia
            return a->getTitulo() < b->getTitulo();
        });
        mostrarPeliculas(peliculasOrdenadas, 0, count);
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
        for (const auto& pelicula : peliculas) {
            if (find(likes.begin(), likes.end(), pelicula) == likes.end()) {
                for (const auto& tag : pelicula->getTags()) {
                    if (tagsLiked.find(tag) != tagsLiked.end()) {
                        sugerencias.push_back(pelicula);
                        break;
                    }
                }
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
                plataforma.buscarPorPalabraOFrase(palabra);
                break;
            }
            case 2: {
                cout << "Ingresa una etiqueta (tag) para buscar: ";
                string tag;
                cin >> tag;
                plataforma.buscarPorTag(tag);
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
    plataforma.cargarPeliculas("C:/Users/fbver/.vscode/PROYECTO_PROGRA/mpst_full_data.csv");
    mostrarMenu(plataforma);
    return 0;
}
