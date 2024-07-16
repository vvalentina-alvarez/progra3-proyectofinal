# Programación III: Proyecto Final (2024-1)
# Integrantes
- Valentina Alvarez
* Fabricio Bautista
# Informe

El siguiente código implementa una plataforma de streaming para explorar y recomendar películas. El programa permite a los usuarios buscar películas por título, sinopsis o etiquetas (tags), ver detalles de películas específicas, marcar películas para ver más tarde, dar "like" a películas, y obtener sugerencias de películas basadas en sus gustos.

El código tiene 2 clases principales: "Movie" y "MovieDatabase".

La clase "Movie" está conformada por las variables: id (número identificador de la pelicula en la base de datos), título, sinopsis, y tags. Con respecto a métodos, la clase cuenta con "gettitle", "getsynopsis", "getTags", "getId".

La clase "MovieDatabase" es un Singleton que maneja la base de datos de películas, permitiendo cargar películas desde un archivo CSV, buscar películas, mostrar detalles, marcar películas como "me gusta" o "ver más tarde", y recomendar películas basadas en las etiquetas de las películas que han gustado. Esta clase tiene 3 partes principales: miembros privados, constructores y métodos de Singleton, y métodos públicos. Los miembros privados consisten en un vector que almacena todas las peliculas y tres mapas  que asocian las características de las peliculas a cada pelicula y almacenan las peliculas marcadas como "me gusta" y #ver mas tarde", respectivamente. Los métodos públicos permiten cargar las peliuculas desde el archivo CSV de la base de datos, buscar las peliculas por titulo o tag, mostrarlos detalles de la pelicula, marcar una pelicula como "me gusta" o "ver más tarde", y recomendar peliculas basadas en los "likes" anteriores del usuario.

Además, se crea una función "showMenu" fuera de las clases que permite al usuario realize diferentes acciones como buscar películas por título o sinopsis, buscar películas por etiqueta (tag), ver detalles de una película específica, sugerir películas similares basadas en los likes del usuario, mostrar la lista de "Ver más tarde", mostrar la lista de "Likes"  y salir del programa.

En la función principal "main", se utilizan todos los métodos de las clases para construir la plataforma de streaming q utilizará el usuario. Aquí, se llama a la función "mostrarMenu" para que el usuario pueda interactuar con el programa.

El código utiliza varias librerías que permiten manejar diferentes funciones necesitadas para la creación del programa  y hacerlo más optimo.  Se utiliza la librería "fstream" para manejar los archivos, por ejemplo se utiliza para abrir y leer el archivo CSV de la base de datos. Para un acceso más rápido a los elementos, se utilizó las librerías "unordered_map" y "unordered_set". Para poder realizar tareas simultáneamente y de manera concurrente, se consideró oportuno utilizar la librería "threads".
