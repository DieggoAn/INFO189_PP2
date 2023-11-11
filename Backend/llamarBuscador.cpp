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
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>


const int PORT_CACHE = 9091;  // Port for cache
const int PORT_BACKEND = 9092;  // Port for backend

bool sendMessageToCache(const std::string& message) {
    int cacheSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (cacheSocket == -1) {
        std::cerr << "Error creating cache socket" << std::endl;
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_CACHE);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(cacheSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error connecting to cache" << std::endl;
        close(cacheSocket);
        return false;
    }

    send(cacheSocket, message.c_str(), message.size(), 0);

    close(cacheSocket);
    return true;
}

std::string receiveMessageFromCache() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating server socket" << std::endl;
        return "";
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_BACKEND);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding server socket" << std::endl;
        close(serverSocket);
        return "";
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error listening on server socket" << std::endl;
        close(serverSocket);
        return "";
    }

    int clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == -1) {
        std::cerr << "Error accepting connection from cache" << std::endl;
        close(serverSocket);
        return "";
    }
    // Receive data in a loop until a message is received
    std::string receivedMessage;
    char buffer[1024];
    int bytesRead;

    while (true) {
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead > 0) {
            receivedMessage.append(buffer, bytesRead);
            if (receivedMessage.find('\0') != std::string::npos) {
                // Assuming '\0' is the message terminator; adjust as needed
                break; // Message received completely
            }
        } else if (bytesRead == 0) {
            // Connection closed by the client
            break;
        } else {
            std::cerr << "Error receiving data" << std::endl;
            close(clientSocket);
            close(serverSocket);
            return "";
        }
    }
    close(clientSocket);
    close(serverSocket);
    return std::string(buffer);
}


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
std::string printFileDataForMultipleKeys(const std::map<std::string, std::vector<FileData>>& dataMap, const std::string& keys, int TOPK) {
    // Split the keys into individual words.
    std::istringstream keyStream(keys);
    std::vector<std::string> keyList;
    std::string key;
    std::cout << keys << std::endl;
    std::string rezultado;
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
    int cont = 0;
    for (const auto& entry : sortedRecurrence) {
        const std::string& filename = entry.first;
        int recurrence = entry.second;
        int matchedCount = matchedKeysCount[filename];

        if (matchedCount == totalKeys) {
            if(cont < TOPK){
                rezultado += filename + ":" + std::to_string(recurrence) + ";";
                cont ++;
                std::cout << rezultado << std::endl;
            }
        }
    }
    return rezultado;
}
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

int main() {;
        loadEnvFromFile("Backend/.env1");
    while (true){
        std::string receivedMessage = receiveMessageFromCache();
        std::cout << "Received message from frontend: " << receivedMessage << std::endl;
        std::cout<<receivedMessage;
        std::map<std::string, std::vector<FileData>> dataMap = readIndexFile(getenv("FILE"));
        std::string rezultado = printFileDataForMultipleKeys(dataMap, receivedMessage, std::stoi(getenv("TOPK")));  

        sendMessageToCache(rezultado);
        std::cout<<"Message sent : " << rezultado <<std::endl;
    }
    return 0;
    }
