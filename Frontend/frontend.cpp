#include <cstdlib> 
#include <fstream>
#include <string>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <cctype>

void loadEnvFromFile(const std::string& envFilePath) { //funcion para cargar variables de entorno
    std::ifstream file(envFilePath);
    std::string line;

    if (file.is_open()) {
        while (getline(file, line)) {
            // Check if the line contains an environment variable assignment
            size_t equalsPos = line.find('=');
            if (equalsPos != std::string::npos) {
                std::string varName = line.substr(0, equalsPos);
                std::string varValue = line.substr(equalsPos + 1);
                
                // Set the environment variable in the current process
                setenv(varName.c_str(), varValue.c_str(), 1);
            }
        }
        file.close();
    } else {
        std::cerr << "Unable to open .env file: " << envFilePath << std::endl;
    }
}

int main() {

    loadEnvFromFile("Frontend/.env"); //se cargan variables de entorno
    char seguir = 's';
    std::string userInput;
    while (seguir== 's'){ 
            pid_t pid = getpid();
            std::cout << "“BUSCADOR BASADO EN INDICE INVERTIDO” (" << pid<<")" <<std::endl;
            std::cout << "Los top K documentos serán =: "<< getenv("TOPK") <<std::endl;
            std::cout << "Escriba texto a buscar: ";
            std::cin >> std::ws;
            std::getline(std::cin, userInput);
            std::transform(userInput.begin(), userInput.end(), userInput.begin(), ::tolower); 
            std::cout << "Respuesta: ";          
            // Imprime los resultados ACA SE GUARDA EL STRING CON LA RESPUESTA!!!!!!
            std::cout << "Desea seguir (S/N):" ;
            std::cin >> seguir;

            seguir = std::tolower(seguir);

    }
    return 0;
}
