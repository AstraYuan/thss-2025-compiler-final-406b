#include <iostream>
#include <fstream>
#include "antlr4-runtime.h"
#include "SysYLexer.h"
#include "SysYParser.h"
#include "IRBuilder.h"

using namespace antlr4;

int main(int argc, const char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: ./compiler <input-file> <output-file>"
              << std::endl;
    return 1;
  }
  // TODO: Implement the main function of the compiler. // completed in init
    
  std::string inputFile = argv[1];
  std::string outputFile = argv[2];
  
  // Read input file
  std::ifstream stream(inputFile);
  if (!stream) {
    std::cerr << "Cannot open input file: " << inputFile << std::endl;
    return 1;
  }
  
  // Create ANTLR input stream
  ANTLRInputStream input(stream);
  
  // Create lexer
  SysYLexer lexer(&input);
  CommonTokenStream tokens(&lexer);
  
  // Create parser
  SysYParser parser(&tokens);
  
  // Parse the input
  SysYParser::CompUnitContext* tree = parser.compUnit();
  
  // Check for parser errors
  if (parser.getNumberOfSyntaxErrors() > 0) {
    std::cerr << "Syntax errors found" << std::endl;
    return 1;
  }
  
  // Build IR
  IRBuilder builder;
  builder.visitCompUnit(tree);
  
  // Write output
  std::ofstream outStream(outputFile);
  if (!outStream) {
    std::cerr << "Cannot open output file: " << outputFile << std::endl;
    return 1;
  }
  
  outStream << builder.getIR();
  outStream.close();
  
  return 0;
}