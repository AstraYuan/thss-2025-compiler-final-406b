#ifndef IRGENERATOR_H
#define IRGENERATOR_H

#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <memory>
#include "Type.h"

class IRGenerator {
private:
    std::ostringstream header;  // For declarations and global variables
    std::ostringstream body;    // For function definitions
    int tempCounter;
    int labelCounter;
    std::vector<int> breakLabels;
    std::vector<int> continueLabels;
    
public:
    IRGenerator() : tempCounter(0), labelCounter(0) {}
    
    std::string getNewTemp() {
        return "%t" + std::to_string(tempCounter++);
    }
    
    int getNewLabel() {
        return labelCounter++;
    }
    
    void emitHeader(const std::string& code) {
        header << code;
    }
    
    void emit(const std::string& code) {
        body << code;
    }
    
    void emitLabel(int label) {
        body << "label" << label << ":\n";
    }
    
    void pushBreakLabel(int label) {
        breakLabels.push_back(label);
    }
    
    void popBreakLabel() {
        if (!breakLabels.empty()) {
            breakLabels.pop_back();
        }
    }
    
    int getBreakLabel() {
        return breakLabels.empty() ? -1 : breakLabels.back();
    }
    
    void pushContinueLabel(int label) {
        continueLabels.push_back(label);
    }
    
    void popContinueLabel() {
        if (!continueLabels.empty()) {
            continueLabels.pop_back();
        }
    }
    
    int getContinueLabel() {
        return continueLabels.empty() ? -1 : continueLabels.back();
    }
    
    std::string getOutput() {
        return header.str() + "\n" + body.str();
    }
    
    void reset() {
        header.str("");
        header.clear();
        body.str("");
        body.clear();
        tempCounter = 0;
        labelCounter = 0;
        breakLabels.clear();
        continueLabels.clear();
    }
};

#endif // IRGENERATOR_H
