#include <cstdlib> 
#include <fstream>
#include <string>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>

const int PORT_CACHE = 9091;  // Port for cache
const int PORT_FRONTEND = 9090;  // Port for backend

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
    serverAddr.sin_port = htons(PORT_FRONTEND);
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
    pid_t pid = getpid();
    std::cout << "“BUSCADOR BASADO EN INDICE INVERTIDO” (" << pid<<")" <<std::endl;
    std::cout << "Los top K documentos serán =: "<< getenv("TOPK") <<std::endl;
    std::cout << "Escriba texto a buscar: ";
    std::cin >> std::ws;
    std::getline(std::cin, userInput);
    std::transform(userInput.begin(), userInput.end(), userInput.begin(), ::tolower); 
    std::string mensaje = "{origen:\"" + std::string(getenv("FROM")) +  "\",destino:\"" + std::string(getenv("TO")) + "\",contexto:{topk:\"" + (getenv("TOPK")) +"\", txtToSearch:\"" + userInput +"\"}}";
    bool mensahe = sendMessageToCache(mensaje);
    std::string response = receiveMessageFromCache();
    std::cout << "Respuesta: "<<response<<std::endl;          
    std::cout << "Desea seguir (S/N):" ;
    std::cin >> seguir;

    while (seguir == 's'){ 
        pid_t pid = getpid();
        std::cout << "“BUSCADOR BASADO EN INDICE INVERTIDO” (" << pid<<")" <<std::endl;
        std::cout << "Los top K documentos serán =: "<< getenv("TOPK") <<std::endl;
        std::cout << "Escriba texto a buscar: ";
        std::cin >> std::ws;
        std::getline(std::cin, userInput);
        std::transform(userInput.begin(), userInput.end(), userInput.begin(), ::tolower); 
        std::string mensaje = "{origen:\"" + std::string(getenv("FROM")) +  "\",destino:\"" + std::string(getenv("TO")) + "\",contexto:{topk:\"" + (getenv("TOPK")) +"\", txtToSearch:\"" + userInput +"\"}}";
        bool mensahe = sendMessageToCache(mensaje);
        std::string response = receiveMessageFromCache();
        std::cout << "Respuesta: "<<response<<std::endl;          
        std::cout << "Desea seguir (S/N):" ;
        std::cin >> seguir;
        seguir = std::tolower(seguir);  
    }
    return 0;
}
