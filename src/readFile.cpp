#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <readFile.h>

std::vector<std::string> splitString(std::string str){
    std::vector<std::string> output;
    int start = 0;
    int end = str.find(" ");
    while(end != -1){
        output.push_back(str.substr(start, end-start));
        start = end + 1;
        end = str.find(" ", start);
    }
    //get the last item in list
    output.push_back(str.substr(start, end-start));
    return output;
}

std::vector<std::string> readFile(std::string filename){
    std::vector<std::string> output;
    std::string line;
    std::ifstream file(filename);
    while(getline(file, line)){
        output.push_back(line);
    }
    file.close();
    return output;
}

std::vector<std::vector<std::string>> readFile2d(std::string filename){
    std::vector<std::vector<std::string>> output;
    std::string line;
    std::ifstream file(filename);
    while(getline(file, line)){
        output.push_back(splitString(line));
    }
    file.close();
    return output;
}

void writeFile2d(std::vector<std::vector<std::string>> inputMatrix, std::string filename){
    std::ofstream channelFile;
    channelFile.open(filename);
    for(int i = 0; i < inputMatrix.size(); i++){
        for(int j = 0; j < inputMatrix[i].size(); j++){
            channelFile << inputMatrix[i][j] << " ";
        }
        channelFile << std::endl;
    }
    channelFile.close();
}
