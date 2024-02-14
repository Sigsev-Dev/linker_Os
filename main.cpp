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
    vector<int> moduleSizes;
    Token token = tokenizer.nextToken();

    while (token.type != END_OF_FILE) {
        // Read definition list
        if (token.type == INT) {
            int defCount = stoi(token.value);
            for (int i = 0; i < defCount; ++i) {
                Token symbolToken = tokenizer.nextToken(); // Expect SYMBOL
                Token relativeAddressToken = tokenizer.nextToken(); // Expect INT
                if (symbolToken.type == SYMBOL && relativeAddressToken.type == INT) {
                    int relativeAddress = stoi(relativeAddressToken.value);
                    if (symbolTable.find(symbolToken.value) == symbolTable.end()) {
                        symbolTable[symbolToken.value] = moduleBaseAddress + relativeAddress;
                    } else {
                        // Error: symbol defined multiple times, but only the first definition is used
                        cout << "Error: This variable " << symbolToken.value << " is multiple times defined; first value used." << endl;
                    }
                }
            }
        }

        // Skip use list and program text for the first pass, just calculate the module size
        // Assuming the next token after definition list is the module size (INT type)
        token = tokenizer.nextToken(); // Skip use list
        if (token.type == INT) {
            int moduleSize = stoi(token.value);
            moduleBaseAddresses.push_back(moduleBaseAddress);
            moduleBaseAddress += moduleSize;
            moduleSizes.push_back(moduleSize);
        }

        token = tokenizer.nextToken(); // Proceed to the next part or END_OF_FILE
    }

    // Output the symbol table after the first pass
    cout << "Symbol Table" << endl;
    for (const auto& symbol : symbolTable) {
        cout << symbol.first << "=" << symbol.second << endl;
    }
}


void secondPass(Tokenizer& tokenizer) {
    vector<int> memoryMap;
    int currentModuleIndex = 0;
    int currentModuleBase = moduleBaseAddresses[currentModuleIndex]; // Start with the base address of the first module
    map<int, string> useList; // Temporary storage for the use list of the current module
    Token token;

    while ((token = tokenizer.nextToken()).type != END_OF_FILE) {
        if (token.type == MODULE_START) {
            // Reset or initialize necessary variables at the start of a new module
            currentModuleBase = moduleBaseAddresses[currentModuleIndex++];
            useList.clear(); // Clear previous module's use list
        } else if (token.type == USE) {
            // Populate the use list for the current module
            int useCount = stoi(token.value); // Assuming token.value has the count
            for (int i = 0; i < useCount; ++i) {
                Token symbolToken = tokenizer.nextToken(); // Fetch next symbol token
                if (symbolToken.type == SYMBOL) {
                    useList[i] = symbolToken.value; // Map use list index to symbol
                }
            }
        } else if (token.type == ADDR) {
            // Process instructions and resolve addresses
            char mode = token.value[0];
            Token instrToken = tokenizer.nextToken(); // Fetch the instruction
            int instruction = stoi(instrToken.value);
            int opcode = instruction / 1000;
            int operand = instruction % 1000;

            switch(mode) {
                case 'A': // Absolute address
                    // Validate against machine size
                    break;
                case 'R': // Relative address
                    operand += currentModuleBase;
                    break;
                case 'E': // External address
                    if (useList.find(operand) != useList.end()) {
                        string symbol = useList[operand];
                        if (symbolTable.find(symbol) != symbolTable.end()) {
                            operand = symbolTable[symbol];
                        } else {
                            // Handle error: Symbol not defined
                            operand = 0; // Use zero if symbol is undefined
                        }
                    } else {
                        // Handle error: External operand exceeds use list length
                        operand = 0; // Treat as relative=0
                    }
                    break;
                case 'I': // Immediate address
                    // Validate immediate value
                    break;
                case 'M': // Module address
                    // This requires interpreting the module-specific logic based on your specifications
                    break;
            }
            memoryMap.push_back(opcode * 1000 + operand);
        }
    }

    // Output the memory map
    cout << "Memory Map" << endl;
    for (int i = 0; i < memoryMap.size(); ++i) {
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
    // Reset or reinitialize tokenizer if necessary
    secondPass(tokenizer);

    // Output results: symbol table, memory map, etc.
    return 0;
}
