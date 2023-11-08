#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <cctype>
#include <unordered_map>
#include <algorithm>

struct FileData {
    std::string filename;
    int recurrence;
};

// Define a function to read the file and populate the map.
std::map<std::string, std::vector<FileData>> readIndexFile(const std::string& filename) {
    std::map<std::string, std::vector<FileData>> dataMap;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Failed to open the file: " << filename << std::endl;
        return dataMap; // Return an empty map in case of an error.
    }

    std::string line;
    while (std::getline(file, line)) {
        // Split the line into key and data parts.
        size_t commaPos = line.find(',');
        if (commaPos != std::string::npos) {
            std::string key = line.substr(0, commaPos);
            std::string data = line.substr(commaPos + 1);

            // Split the data into individual filename-recurrence pairs.
            size_t semicolonPos = data.find(';');
            while (semicolonPos != std::string::npos) {
                std::string pair = data.substr(0, semicolonPos);
                size_t colonPos = pair.find(':');
                if (colonPos != std::string::npos) {
                    std::string filename = pair.substr(0, colonPos);
                    int recurrence = std::stoi(pair.substr(colonPos + 1));

                    // Add the data to the map.
                    dataMap[key].push_back({filename, recurrence});
                }
                data = data.substr(semicolonPos + 1);
                semicolonPos = data.find(';');
            }
        }
    }

    file.close();
    return dataMap;
}
void printFileDataForMultipleKeys(const std::map<std::string, std::vector<FileData>>& dataMap, const std::string& keys) {
    // Split the keys into individual words.
    std::istringstream keyStream(keys);
    std::vector<std::string> keyList;
    std::string key;
    std::cout << keys << std::endl;
    while (std::getline(keyStream, key, ' ')) {
        keyList.push_back(key);
    }

    // Create a map to store aggregated recurrence counts for filenames.
    std::map<std::string, int> aggregateRecurrence;

    // Iterate through the keys and update the aggregateRecurrence map.
    for (const std::string& key : keyList) {
        auto it = dataMap.find(key);
        if (it != dataMap.end()) {
            for (const FileData& fileData : it->second) {
                // Add or update the aggregated recurrence count for the filename.
                aggregateRecurrence[fileData.filename] += fileData.recurrence;
            }
        }
    }

    // Determine the total number of keys in the search query.
    int totalKeys = keyList.size();

    // Create a map to store the count of matched keys for each filename.
    std::map<std::string, int> matchedKeysCount;

    // Initialize the count of matched keys for all filenames to zero.
    for (const auto& entry : dataMap) {
        for (const FileData& fileData : entry.second) {
            matchedKeysCount[fileData.filename] = 0;
        }
    }

    // Update the count of matched keys for filenames.
    for (const std::string& key : keyList) {
        auto it = dataMap.find(key);
        if (it != dataMap.end()) {
            for (const FileData& fileData : it->second) {
                matchedKeysCount[fileData.filename]++;
            }
        }
    }
    std::vector<std::pair<std::string, int>> sortedRecurrence(aggregateRecurrence.begin(), aggregateRecurrence.end());

    // Sort the vector in descending order based on values (recurrence counts).
    std::sort(sortedRecurrence.begin(), sortedRecurrence.end(),
            [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                return a.second > b.second;
            });
    for (const auto& entry : sortedRecurrence) {
        const std::string& filename = entry.first;
        int recurrence = entry.second;
        int matchedCount = matchedKeysCount[filename];

        if (matchedCount == totalKeys) {
            std::cout << "Filename: " << filename << ", Recurrence: " << recurrence << std::endl;
        }
    }
}


int main(int argc, char* argv[]) {
    // Define a map to store each line from the input file
    std::string indexFile = argv[1];
    std::string TOPK = argv[2];
    char seguir = 's';
    while (seguir == 's' ){

            std::map<std::string, std::vector<FileData>> dataMap = readIndexFile(indexFile);
            std::string userInput = "";
            pid_t pid = getpid();

            std::cout << "“BUSCADOR BASADO EN INDICE INVERTIDO” (" << pid<<")" <<std::endl;
            std::cout << "Los top K documentos serán =: "<< TOPK <<std::endl;
            std::cout << "Escriba texto a buscar: ";
            std::getline(std::cin, userInput);
            std::transform(userInput.begin(), userInput.end(), userInput.begin(), ::tolower); 
            std::cout << "Respuesta: ";          
            // Imprime los resultados
            printFileDataForMultipleKeys(dataMap, userInput);  
            std::cout << std::endl;
            std::cout<<std::endl;

            std::cout << "Desea seguir (S/N):" ;
            std::cin >> seguir;
            seguir = std::tolower(static_cast<unsigned char>(seguir));
        }
    return 0;
    }
