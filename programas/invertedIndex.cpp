#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <unistd.h>

//Funcion que busca archivos en "PATH_FILES_OUT" y agrega su nombre a un vector "fileNames"
std::vector<std::string> getFilesInFolder(std::string& PATH_FILES_OUT) {
    std::vector<std::string> fileNames;
    try {
        // Iterate over the entries in the folder
        for (const auto& entry : std::filesystem::directory_iterator(PATH_FILES_OUT)) {
            std::string fileName = entry.path().filename().string();
            std::string fileExtension = fileName.substr(fileName.length() - 3);
            if (entry.is_regular_file()) {
                fileNames.push_back(fileName);
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error accessing folder: " << e.what() << std::endl;
    }

    return fileNames;
}


struct WordOccurrence {
    std::string fileName;
    int count;
};

// Function to consolidate word occurrences from pre-parsed files
void consolidateWordOccurrences(const std::vector<std::string>& fileNames, const std::string& outputPath, const std::string& PATH_FILES_OUT) {
    std::unordered_map<std::string, std::vector<WordOccurrence>> wordOccurrences; // Map to store word occurrences

    //mapeo de palabras en todos los archivos del vector
    for (const std::string& fileName : fileNames) {
        std::ifstream inputFile(PATH_FILES_OUT + fileName);
        std::string word;
        int count;
        while (inputFile >> word >> count) {
            wordOccurrences[word].push_back({fileName, count});
        }

        inputFile.close();
    }

    std::ifstream fileCheck(outputPath);
    if (!fileCheck) {
        // File doesn't exist, create it
        std::ofstream fileCreate(outputPath);
        if (fileCreate) {
            std::cout << "Se creo el archivo de Indice invertido" << std::endl;
        } else {
            std::cerr << "Error creando el archivo de indice invertido." << std::endl;
        }
    }
    std::ofstream outputFile(outputPath);

    if (!outputFile.is_open()) {
        std::cerr << "Error: Failed to open the output file." << std::endl;
    return;
    }
    pid_t pid = getpid();
    std::cout <<"El proceso pid: " << pid << " Palabras procesadas: " << wordOccurrences.size() << " En el archivo: " << outputPath << std::endl;

    // Lectura del map, y escritura en outputhfile
    for (const auto& entry : wordOccurrences) {
        const std::string& word = entry.first;
        const std::vector<WordOccurrence>& occurrences = entry.second;

        outputFile << word << "";
        for (const WordOccurrence& occurrence : occurrences) {
            outputFile << "" << occurrence.fileName << ":" << occurrence.count << ";";
        }
        outputFile << std::endl;
    }

    outputFile.close();
}

int main(int argc, char* argv[]) {
    
    std::string indexFile = argv[2];
    std::string pathOut = argv[1];
    std::vector<std::string> fileNames = getFilesInFolder(pathOut);
    consolidateWordOccurrences(fileNames,indexFile, pathOut);

    return 0;
}