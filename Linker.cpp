#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <cctype>
#include <stdexcept>
#include <iomanip>
#include <algorithm>
#include <typeinfo>
#include <iterator>

using namespace std;

enum TokenType { SYMBOL, INT, ADDR, INSTR, END_OF_FILE, DEFINITION, USE, MODULE_START, TEXT };

struct Token {
    TokenType type;
    string value;
};


class Tokenizer {
    vector<string> tokens;
    size_t current = 0;

public:
    Tokenizer(const string& filename) {
        ifstream file(filename);
        string line, token;
        while (getline(file, line)) {
            istringstream iss(line);
            while (iss >> token) {
                tokens.push_back(token);
            }
        }
        tokens.push_back(""); // End of file marker
    }

    Token nextToken() {
    if (current >= tokens.size()) {
        return {END_OF_FILE, ""};
    }

    string currentToken = tokens[current++];
    Token token;

    // Determine if the token is a symbol: begins with an alphabet character and is alphanumeric
    if (isalpha(currentToken[0])) {
        for (size_t i = 1; i < currentToken.size(); ++i) {
            if (!isalnum(currentToken[i])) {
                token.type = TEXT; // Not a valid symbol if it contains non-alphanumeric characters after the first
                token.value = currentToken;
                return token;
            }
        }
        token.type = SYMBOL;
    }
    // Determine if the token is an integer: all characters are digits
    else if (all_of(currentToken.begin(), currentToken.end(), ::isdigit)) {
        token.type = INT;
    }
    // Determine instruction type characters: 'A', 'R', 'E', 'I', 'M'
    else if (currentToken == "A" || currentToken == "R" || currentToken == "E" || currentToken == "I" || currentToken == "M") {
        token.type = ADDR;
    }
    else {
        token.type = TEXT; // Default to TEXT for any other format
    }

    token.value = currentToken;
    return token;
}
};

map<string, int> symbolTable;
vector<int> moduleBaseAddresses;
vector<int> memoryMap;

void firstPass(Tokenizer& tokenizer) {
    int moduleBaseAddress = 0;
    Token token = tokenizer.nextToken();

    while (token.type != END_OF_FILE) {
        // Read definition list
        if (token.type == INT) {
            int defCount = stoi(token.value);
            for (int i = 0; i < defCount; ++i) {
                Token symbolToken = tokenizer.nextToken(); // Expect SYMBOL
                Token relativeAddressToken = tokenizer.nextToken(); // Expect INT
                
                if (symbolToken.type != SYMBOL || relativeAddressToken.type != INT) {
                    cerr << "Format error: Expected SYMBOL and INT." << endl;
                    continue; // Skip to next module or token as per your error handling policy
                }

                int relativeAddress = stoi(relativeAddressToken.value);
                if (symbolTable.find(symbolToken.value) == symbolTable.end()) {
                    symbolTable[symbolToken.value] = moduleBaseAddress + relativeAddress;
                } else {
                    cerr << "Error: Symbol " << symbolToken.value << " is defined multiple times; first definition used." << endl;
                }
            }
        }

        // Adjust for moduleBaseAddresses and module sizes
        token = tokenizer.nextToken(); // Potentially next module or EOF
        if (token.type == INT) {
            int moduleSize = stoi(token.value);
            moduleBaseAddresses.push_back(moduleBaseAddress);
            moduleBaseAddress += moduleSize;
        } else if (token.type == END_OF_FILE) {
            break; // End of file reached
        }
        // Prepare for next token in the loop
        token = tokenizer.nextToken();
    }

    // Print the symbol table
    cout << "Symbol Table" << endl;
    for (const auto& entry : symbolTable) {
        cout << entry.first << "=" << entry.second << endl;
    }
}

void secondPass(Tokenizer& tokenizer) {
    vector<int> memoryMap;
    int currentModuleIndex = 0;
    map<string, bool> symbolUsed; // Tracks if a symbol is used in the module
    Token token = tokenizer.nextToken();

    while (token.type != END_OF_FILE) {
        if (token.type == MODULE_START) {
            currentModuleIndex++; // Advance to next module
            symbolUsed.clear(); // Reset for the new module
        } else if (token.type == USE) {
            int useCount = stoi(tokenizer.nextToken().value);
            for (int i = 0; i < useCount; ++i) {
                Token symbolToken = tokenizer.nextToken();
                symbolUsed[symbolToken.value] = false; // Initialize as unused
            }
        } else if (token.type == ADDR) {
            char mode = token.value[0];
            Token instrToken = tokenizer.nextToken();
            int instruction = stoi(instrToken.value);
            int opcode = instruction / 1000;
            int operand = instruction % 1000;
            
            switch(mode) {
                case 'A': // Absolute address does not require adjustment, but must not exceed memory size.
                    if (operand >= 512) {
                        cout << setw(3) << setfill('0') << memoryMap.size() << ": " << 9999 << " Error: Absolute address exceeds machine size; treated as 9999" << endl;
                    } else {
                        memoryMap.push_back(opcode * 1000 + operand);
                    }
                    break;
                case 'R': // Relative address needs to be adjusted with the module's base address.
                    operand += currentModuleBase;
                    if (operand >= 512) {
                        cout << setw(3) << setfill('0') << memoryMap.size() << ": " << 9999 << " Error: Relative address exceeds machine size; treated as 9999" << endl;
                    } else {
                        memoryMap.push_back(opcode * 1000 + operand);
                    }
                    break;
                case 'E': // External address is replaced with the symbol's absolute address from the symbol table.
                    if (symbolUsed.find(operand) != symbolUsed.end() && symbolTable.find(symbolUsed[operand]) != symbolTable.end()) {
                        operand = symbolTable[useList[operand]];
                        memoryMap.push_back(opcode * 1000 + operand);
                    } else {
                        cout << setw(3) << setfill('0') << memoryMap.size() << ": " << opcode * 1000 << " Error: Undefined external address; treated as zero" << endl;
                    }
                    break;
                case 'I': // Immediate address should not be modified, but validation is required.
                    if (instruction >= 900) {
                        cout << setw(3) << setfill('0') << memoryMap.size() << ": " << 9999 << " Error: Illegal immediate value; treated as 9999" << endl;
                    } else {
                        memoryMap.push_back(instruction);
                    }
                    break;
                default:
                    cout << "Error: Unknown addressing mode '" << mode << "'" << endl;
                    break;
            }

            memoryMap.push_back(opcode * 1000 + operand); // Add instruction to memory map
        }
        
        token = tokenizer.nextToken(); // Proceed to next token
    }

    // Output warnings for unused symbols
    for (const auto& entry : symbolUsed) {
        if (!entry.second) {
            cout << "Warning: Symbol " << entry.first << " declared in use list but not used" << endl;
        }
    }

    // Print memory map
    cout << "Memory Map" << endl;
    for (size_t i = 0; i < memoryMap.size(); ++i) {
        cout << setfill('0') << setw(3) << i << ": " << memoryMap[i] << endl;
    }
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <inputfile>\n";
        return 1;
    }

    Tokenizer tokenizer(argv[1]);
    firstPass(tokenizer);
    secondPass(tokenizer);
    return 0;
}
