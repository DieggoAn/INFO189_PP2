#include <cstdlib> 
#include <fstream>
#include <string>
#include <iostream>

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

void executeInvertedIndex(const std::string& param1, const std::string& param2, const std::string& param3){ //funcion para ejecutar inverted index
    std::string command = param3 + " " + param1 + " " + param2;
    int exitCode = std::system(command.c_str());
}



int main() {

    loadEnvFromFile(".env"); //se cargan variables de entorno
    executeInvertedIndex(getenv("PATH_FILES_OUT"), getenv("INVERTED_INDEX_FILE"),getenv("INVERTED_INDEX_APP"));//ejecucion inverted index

    



    return 0;
}
