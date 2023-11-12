#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <regex>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

const int PORT_CACHE = 9091;  // Port for cache
const int PORT_FRONTEND = 9090;  // Port for frontend
const int PORT_BACKEND = 9092; // Port to backend

std::string receiveMessageFromFrontend() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating server socket" << std::endl;
        return "";
    }
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_CACHE);
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
        std::cerr << "Error accepting connection from frontend" << std::endl;
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

    return receivedMessage;
}



void sendResponseToFrontend(const std::string& response) {
    int frontendSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (frontendSocket == -1) {
        std::cerr << "Error creating frontend socket" << std::endl;
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_FRONTEND);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(frontendSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error connecting to frontend" << std::endl;
        close(frontendSocket);
        return;
    }

    send(frontendSocket, response.c_str(), response.size(), 0);
    close(frontendSocket);
}

void sendResponseToBackend(const std::string& response) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    int frontendSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (frontendSocket == -1) {
        std::cerr << "Error creating frontend socket" << std::endl;
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_BACKEND);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(frontendSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error connecting to frontend" << std::endl;
        close(frontendSocket);
        return;
    }

    send(frontendSocket, response.c_str(), response.size(), 0);
    close(frontendSocket);
}



std::pair<std::string, std::string> parseMessage(const std::string& message) { //funcion que parsea el mensaje 
    std::smatch match;
    std::regex pattern("topk:\"([^\"]*)\", txtToSearch:\"([^\"]*)\"");
    if (std::regex_search(message, match, pattern)) {
        return std::make_pair(match[1].str(), match[2].str());
    }
    
    return std::make_pair("", "");
}

struct FileData {
    std::string filename;
    int recurrence;
};


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


void parseData(const std::string& keyValue, const std::string& data, std::map<std::string, std::vector<std::string>>& dataMap, size_t maxSize) {
    std::istringstream dataStream(data);
    std::string fileDataStr;

    std::vector<std::string> fileDataList;

    while (std::getline(dataStream, fileDataStr, ';')) {
        fileDataList.push_back(fileDataStr);
    }

    // Check if the maximum size is reached
    if (dataMap.size() >= maxSize) {
        // Remove the oldest entry (the first one added)
        auto oldestEntry = dataMap.begin();
        std::cout << "Removing oldest entry: " << oldestEntry->first << std::endl;
        dataMap.erase(oldestEntry);
    }

    // Add the new data to the map
    dataMap[keyValue] = fileDataList;
    std::cout << "Added to map: " << keyValue << std::endl;
}

std::string extractResultados(const std::string& message) {
    size_t startPos = message.find("resultados:[");
    size_t endPos = message.find("]", startPos);

    if (startPos != std::string::npos && endPos != std::string::npos) {
        // Extract the content within "resultados:[...]"
        std::string resultadosContent = message.substr(startPos + 12, endPos - startPos - 12);
        return resultadosContent;
    }

    return ""; // Return an empty string if "resultados:[...]" is not found
}

bool checkWordsInMap(const std::string& txtToSearch, int TOPK,const std::map<std::string, std::vector<std::string>>& wordMap) {  //busca si el texto existe en el map  
    // All words found in the map, return true
    return wordMap.find(txtToSearch) != wordMap.end();
}

std::string durationToString(const std::chrono::microseconds& duration) {
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    auto milliseconds = microseconds / 1000;
    auto seconds = milliseconds / 1000;

    return std::to_string(seconds) + "s " +
           std::to_string(milliseconds % 1000) + "ms " +
           std::to_string(microseconds % 1000) + "us";
}

int main() {
    loadEnvFromFile("Cache/.env");
    size_t maxSize = getenv("MEMORYSIZE") != nullptr ? static_cast<size_t>(std::atol(getenv("MEMORYSIZE"))) : 0;
    std::map<std::string, std::vector<std::string>> dataMap;
    while(true){

        std::string receivedMessage = receiveMessageFromFrontend();
        std::cout << "Received message from frontend: " << receivedMessage << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        std::pair<std::string, std::string> result = parseMessage(receivedMessage);

        std::cout << "Topk: " << result.first << std::endl;
        std::cout << "TxtToSearch: " << result.second << std::endl;

        bool allWordsFound = checkWordsInMap(result.second, std::stoi(result.first),dataMap);

        if (allWordsFound) {
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::string resultados = "";
            std::cout << "All words found in the map." << std::endl;
            auto last = std::prev(dataMap.end());
            for (auto it = dataMap.begin(); it != last; ++it) {
                const auto& entry = *it;
                resultados += entry.first + ",";
            }
            std::string tiempo = "15";
            std::string response = "{\n\torigen:\"" + std::string(getenv("HOST")) + "\",\n\tdestino:\"" + std::string(getenv("FRONT")) + "\",\n\tcontexto:{tiempo:\"" + durationToString(duration) + "\",ori:\"CACHE\",isFound=true,resultados:[" + resultados + "]}\n}";
            sendResponseToFrontend(response);
        } else {
            std::cout << "Not all words found in the map." << std::endl;
            std::cout <<result.second<<std::endl;
            sendResponseToBackend(result.second);
            std::string receivedMessage = receiveMessageFromFrontend();
            std::string parsedMessage = extractResultados(receivedMessage);
            std::cout << "Received message from backend: " << receivedMessage << std::endl;
            parseData(result.second,parsedMessage, dataMap, maxSize);
            sendResponseToFrontend(receivedMessage);
        }
    }
    return 0;

}
