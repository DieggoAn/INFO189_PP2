#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <regex>


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

std::map<std::string, std::vector<FileData>> parseData(const std::string& data) { //funcion que parsea mensaje y los anade a un map
    std::map<std::string, std::vector<FileData>> dataMap;

    std::istringstream iss(data);
    std::string word;
    while (std::getline(iss, word, ';')) {
        std::string key;
        std::vector<FileData> fileDataList;
        
        std::istringstream wordStream(word);
        std::getline(wordStream, key, ',');
        
        std::string fileDataStr;
        while (std::getline(wordStream, fileDataStr, ';')) {
            std::istringstream fileDataStream(fileDataStr);
            std::string filename;
            char separator;
            int recurrence;
            fileDataStream >> filename >> separator >> recurrence;
            fileDataList.push_back({filename, recurrence});
        }
        
        dataMap[key] = fileDataList;
    }

    return dataMap;
}

bool checkWordsInMap(const std::string& txtToSearch, int TOPK,const std::map<std::string, std::vector<FileData>>& wordMap) {  //busca si el texto existe en el map  
    // All words found in the map, return true
    return wordMap.find(txtToSearch) != wordMap.end();
}

int main() {
    std::string data = "meagre impositions,file034.txt:1;file029.txt:1;file021.txt:4;file025.txt:1;file035.txt:1;";
    std::map<std::string, std::vector<FileData>> dataMap = parseData(data);
    // Example: Print the parsed data
    for (const auto& entry : dataMap) {
        std::cout << "Word: " << entry.first << std::endl;
        for (const FileData& fileData : entry.second) {
            std::cout << "  Filename: " << fileData.filename << ", Recurrence: " << fileData.recurrence << std::endl;
        }
    }

    std::string message = "{origen:\"XXXX\",destino:\"YYYY\",contexto:{topk:\"5\", txtToSearch:\"meagre impositions\"}}";

    std::pair<std::string, std::string> result = parseMessage(message);

    std::cout << "Topk: " << result.first << std::endl;
    std::cout << "TxtToSearch: " << result.second << std::endl;

    bool allWordsFound = checkWordsInMap(result.second, std::stoi(result.first),dataMap);

    if (allWordsFound) {
        std::cout << "All words found in the map." << std::endl;
    } else {
        std::cout << "Not all words found in the map." << std::endl;
    }
    return 0;

}
