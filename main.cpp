#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <limits>
#include <cassert>

class Movie {
public:
    Movie() = default; // Constructor predeterminado
    Movie(int id, const std::string &title, const std::string &synopsis, const std::vector<std::string> &tags)
            : id(id), title(title), synopsis(synopsis), tags(tags) {}

    int getId() const { return id; }
    std::string getTitle() const { return title; }
    std::string getSynopsis() const { return synopsis; }
    std::vector<std::string> getTags() const { return tags; }

private:
    int id;
    std::string title;
    std::string synopsis;
    std::vector<std::string> tags;
};

class MovieDatabase {
public:
    // Implementación del Singleton
    static MovieDatabase& getInstance() {
        static MovieDatabase instance; // Instancia única
        return instance;
    }

    // Eliminar los métodos de copia y asignación para asegurar el Singleton
    MovieDatabase(const MovieDatabase&) = delete;
    MovieDatabase& operator=(const MovieDatabase&) = delete;

    void loadMoviesFromCSV(const std::string &filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Error opening file: " + filePath);
        }

        std::string line, word;
        int id = 0;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string title, synopsis;
            std::vector<std::string> tags;
            std::getline(ss, title, ',');
            std::getline(ss, synopsis, ',');
            while (std::getline(ss, word, ',')) {
                tags.push_back(word);
            }
            Movie movie(id++, title, synopsis, tags);
            movies.push_back(movie);
            movieMap[movie.getId()] = movie;
        }
    }

    std::vector<Movie> searchMovies(const std::string &query) const {
        std::vector<Movie> results;
        std::mutex resultsMutex;
        size_t numThreads = std::thread::hardware_concurrency();
        std::vector<std::thread> threads;

        auto searchInRange = [&](size_t start, size_t end) {
            std::vector<Movie> localResults;
            for (size_t i = start; i < end; ++i) {
                const auto &movie = movies[i];
                if (movie.getTitle().find(query) != std::string::npos) {
                    localResults.push_back(movie);
                } else {
                    for (const auto &tag : movie.getTags()) {
                        if (tag.find(query) != std::string::npos) {
                            localResults.push_back(movie);
                            break;
                        }
                    }
                }
            }
            std::lock_guard<std::mutex> lock(resultsMutex);
            results.insert(results.end(), localResults.begin(), localResults.end());
        };

        size_t chunkSize = movies.size() / numThreads;
        for (size_t i = 0; i < numThreads; ++i) {
            size_t start = i * chunkSize;
            size_t end = (i == numThreads - 1) ? movies.size() : start + chunkSize;
            threads.emplace_back(searchInRange, start, end);
        }

        for (auto &thread : threads) {
            thread.join();
        }

        return results;
    }

    void showMovieDetails(int movieId) const {
        if (movieMap.find(movieId) != movieMap.end()) {
            const Movie &movie = movieMap.at(movieId);
            std::cout << "Title: " << movie.getTitle() << std::endl;
            std::cout << "Synopsis: " << movie.getSynopsis() << std::endl;
            std::cout << "Tags: ";
            for (const auto &tag : movie.getTags()) {
                std::cout << tag << " ";
            }
            std::cout << std::endl;
        } else {
            std::cerr << "Movie not found!" << std::endl;
        }
    }

    void likeMovie(int movieId) {
        likedMovies[movieId] = true;
    }

    void watchLater(int movieId) {
        watchLaterMovies[movieId] = true;
    }

    void showWatchLater() const {
        std::cout << "Movies to Watch Later:\n";
        for (const auto &pair : watchLaterMovies) {
            if (pair.second) {
                showMovieDetails(pair.first);
            }
        }
    }

    void showLikedMovies() const {
        std::cout << "Liked Movies:\n";
        for (const auto &pair : likedMovies) {
            if (pair.second) {
                showMovieDetails(pair.first);
            }
        }
    }

    std::vector<Movie> getSimilarMovies() const {
        std::unordered_set<std::string> likedTags;
        std::unordered_set<int> seenMovies; // Para evitar duplicados
        for (const auto &pair : likedMovies) {
            if (pair.second) {
                const auto &movie = movieMap.at(pair.first);
                for (const auto &tag : movie.getTags()) {
                    likedTags.insert(tag);
                }
            }
        }

        std::vector<Movie> similarMovies;
        for (const auto &movie : movies) {
            if (seenMovies.find(movie.getId()) == seenMovies.end()) {
                for (const auto &tag : movie.getTags()) {
                    if (likedTags.find(tag) != likedTags.end()) {
                        similarMovies.push_back(movie);
                        seenMovies.insert(movie.getId());
                        break;
                    }
                }
            }
        }

        return similarMovies;
    }

private:
    MovieDatabase() {} // Constructor privado para el Singleton

    std::vector<Movie> movies;
    std::unordered_map<int, Movie> movieMap;
    std::unordered_map<int, bool> likedMovies;
    std::unordered_map<int, bool> watchLaterMovies;
};

void showMenu() {
    std::cout << "1. Load Movies from CSV\n";
    std::cout << "2. Search Movies\n";
    std::cout << "3. Show Movie Details\n";
    std::cout << "4. Like a Movie\n";
    std::cout << "5. Watch Later\n";
    std::cout << "6. Show Watch Later Movies\n";
    std::cout << "7. Show Liked Movies\n";
    std::cout << "8. Show Recommended Movies\n";
    std::cout << "0. Exit\n";
}

int getValidatedInput() {
    int choice;
    while (true) {
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "Invalid input. Please enter a number.\n";
        } else {
            break;
        }
    }
    return choice;
}

void runTests() {
    // Implement basic unit tests for MovieDatabase
    MovieDatabase& db = MovieDatabase::getInstance();
    db.loadMoviesFromCSV("C:/Users/fbver/Documents/vscode/progra3-proyectofinal/mpst_full_data.csv");

    // Test search functionality
    std::string query = "cult";
    auto results = db.searchMovies(query);
    assert(!results.empty());

    // Test like functionality
    int movieId = results[0].getId();
    db.likeMovie(movieId);
    assert(db.getSimilarMovies().size() > 0);

    // Test watch later functionality
    db.watchLater(movieId);
    db.showWatchLater();

    std::cout << "All tests passed!\n";
}

int main() {
    MovieDatabase& db = MovieDatabase::getInstance();
    int choice;
    std::string filePath = "C:/Users/fbver/Documents/vscode/progra3-proyectofinal/mpst_full_data.csv";
    std::string query;
    int movieId;

    try {
        db.loadMoviesFromCSV(filePath);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    runTests();

    while (true) {
        showMenu();
        choice = getValidatedInput();
        switch (choice) {
            case 1:
                try {
                    db.loadMoviesFromCSV(filePath);
                    std::cout << "Movies loaded successfully.\n";
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                }
                break;
            case 2:
                std::cout << "Enter search query: ";
                std::cin.ignore();
                std::getline(std::cin, query);
                {
                    auto results = db.searchMovies(query);
                    for (size_t i = 0; i < results.size() && i < 5; ++i) {
                        std::cout << results[i].getId() << ": " << results[i].getTitle() << std::endl;
                    }
                    if (results.size() > 5) {
                        std::cout << "Enter 'next' to view more results: ";
                        std::string next;
                        std::cin >> next;
                        if (next == "next") {
                            for (size_t i = 5; i < results.size(); ++i) {
                                std::cout << results[i].getId() << ": " << results[i].getTitle() << std::endl;
                            }
                        }
                    }
                }
                break;
            case 3:
                std::cout << "Enter movie ID: ";
                movieId = getValidatedInput();
                db.showMovieDetails(movieId);
                break;
            case 4:
                std::cout << "Enter movie ID to like: ";
                movieId = getValidatedInput();
                db.likeMovie(movieId);
                break;
            case 5:
                std::cout << "Enter movie ID to watch later: ";
                movieId = getValidatedInput();
                db.watchLater(movieId);
                break;
            case 6:
                db.showWatchLater();
                break;
            case 7:
                db.showLikedMovies();
                break;
            case 8:
            {
                auto recommendedMovies = db.getSimilarMovies();
                std::cout << "Recommended Movies:\n";
                for (const auto &movie : recommendedMovies) {
                    std::cout << movie.getId() << ": " << movie.getTitle() << std::endl;
                }
            }
                break;
            case 0:
                return 0;
            default:
                std::cout << "Invalid choice. Please try again.\n";
        }
    }
    return 0;
}




