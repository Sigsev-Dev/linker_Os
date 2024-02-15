/*********************************************************************************************************
*
*
*
* Operating System Assignment
* Lab 1
* Submitted by: Dev Pant (dp3887)
* New York University
*
*
*
*///******************************************************************************************************


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
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <time.h>
#include <math.h>
#include <climits>
#include <stdio.h>
#include <cstring>
#include <regex>
using namespace std;


typedef vector<int> vi;


const int Machine_Size=512;
int lineNum=0, offset=1;
string line;
const char* delimeter= "\t\n";
bool fileNotEnd;


void parseerror(int errcode)
{
    static const char *errstr[] = {
        "NUM_EXPECTED",           // 0 Number expect, anything >= 2^30 is not a number either
        "SYM_EXPECTED",           // 1 Symbol Expected
        "MARIE_EXPECTED",         // 2 Addressing Expected which is M/A/R/I/E
        "SYM_TOO_LONG",           // 3 Symbol Name is too long
        "TOO_MANY_DEF_IN_MODULE", // 4 > 16
        "TOO_MANY_USE_IN_MODULE", // 5 > 16
        "TOO_MANY_INSTR"          // 6
    };
    printf("Parse Error line %d offset %d: %s\n", lineNum, offset, errstr[errcode]);
    exit(1);
}

struct Symbol {
    string symbol;
    int value;
    int moduleNumber;
    bool isMultDef = false;
    bool used = false;
    int position;
};

map<std::string, Symbol> symbol_table;

struct Module{
    int base_address=0;
    int defcount;
    int usecount;
    int codecount=0;
};

std::vector<Module> modules;
size_t lastTokenEndPos=0;

string fetchNextToken(std::ifstream& fileStream) {
    static string currentToken;
    static std::istringstream currentStream;
    static bool firstCall = true;
    static string lastLine = "";

    if (firstCall || currentStream.eof()) {
        lastLine=line;
        if (!getline(fileStream, line)) {
            fileNotEnd = false; // End of file
            offset=lastTokenEndPos;
            return "";
        }
        currentStream.clear(); // Clear any errors.
        currentStream.str(line); // Load the new line
        firstCall = false;
        lineNum++;
        offset = 1; // Reset offset with each new line
        lastTokenEndPos=line.size()+1;
    }

    if (!(currentStream >> currentToken)) {
        firstCall = true; // Trigger fetching a new line on the next call
        return fetchNextToken(fileStream); // Recursively fetch the next token if the current line is exhausted
    }

    // Calculate offset for the next token by finding the position of the current token in the line
    //size_t endPos = currentStream.tellg();
    //offset = line.find(currentToken, offset - 1) + 1;
    // size_t startPos = currentStream.str().find_first_not_of(" \t", lastTokenEndPos);
    
    // if (startPos != string::npos) {
    //     // Calculate the offset for the next token
    //     // Offset in the file stream starts from 0, but we want to show it as starting from 1 for the user
    //     offset = startPos+1;
    // } else {
    //     // When transitioning between lines, reset the offset
    //     offset = lastTokenEndPos+1;
    // }
    // size_t tokenEndPos = offset + currentToken.length() - 1;
    // lastTokenEndPos = tokenEndPos;
    offset=currentStream.tellg();
    if(offset<0){
        offset=lastTokenEndPos-currentToken.size();
    }
    else{
        offset=offset-currentToken.size()+1;
    }
    return currentToken;
}

bool isValidInteger(const string& input) {
    return std::all_of(input.begin(), input.end(), [](unsigned char c) { return std::isdigit(c); });
}

bool isValidSymbol(const string& input) {
    regex pattern("[a-zA-Z][a-zA-Z0-9]*");
    return regex_match(input, pattern);
}

bool isValidAddress(const string& input) {
    const set<string> validAddresses = {"M", "A", "E", "I", "R"};
    return validAddresses.find(input) != validAddresses.end();
}

// void createSymbol(const std::string& symbolName, int position, int moduleNumber) {
//     // Check if the symbol already exists in the table
//     auto it = symbol_table.find(symbolName);
//     if (it != symbol_table.end()) {
//         // If the symbol exists, mark it as multiply defined
//         if (it->second.moduleNumber == moduleNumber) { // Adjust this check based on your logic
//             cout << "Warning: Module " << moduleNumber-1 << ": " << symbolName << " redefinition ignored" << endl;
//         }
//         //cout << "Warning: Module " << moduleNumber-1 << ": " << symbolName << " redefinition ignored" << endl;
//         else{it->second.isMultDef = true;}
//     } else {
//         // If the symbol doesn't exist, add it to the table
//         Symbol newSymbol;
//         newSymbol.symbol = symbolName;
//         newSymbol.position = position;
//         newSymbol.moduleNumber = moduleNumber;
//         newSymbol.isMultDef = false; // This is already the default value
//         newSymbol.used = false;
//         newSymbol.value = position; // Assuming 'value' should be set to 'position', adjust if needed

//         symbol_table[symbolName] = newSymbol;
//         // symbolRange.push_back()
//     }
// }

int fetchInteger(std::ifstream& fileStream) {
    string token = fetchNextToken(fileStream);
    if (token.empty()) return -1;
    if (!isValidInteger(token)) {
        parseerror(0);
        exit(EXIT_FAILURE);
    }
    return std::stoi(token);
}

string fetchSymbol(std::ifstream& fileStream) {
    string token = fetchNextToken(fileStream);
    if (token.empty() || !isValidSymbol(token)) {
        parseerror(1);
        exit(EXIT_FAILURE);
    } else if (token.length() > 16) {
        parseerror(3);
        exit(EXIT_FAILURE);
    }
    return token;
}

string fetchAddress(std::ifstream& fileStream) {
    string token = fetchNextToken(fileStream);
    if (token.empty() || !isValidAddress(token)) {
        parseerror(2);
        exit(EXIT_FAILURE);
    }
    return token;
}

void printSymbolTable(vector<Symbol> symbols, vector<int> module_addresses) {
    cout << "Symbol Table" << std::endl;
    for (const auto& symbol : symbols) {
        // const std::string& symbol = entry.first;
        //const Symbol& symData = symbol_table[symbol.symbol];
        //cout<<"tester "<<symbol.position<<" "<<symbol.moduleNumber<<" "<<module_addresses[symbol.moduleNumber]<<endl;
        //cout<<"position: ="<<symbol_table[symbol.symbol].position<<" "<<symbol.moduleNumber<<endl;
        if(symbol.moduleNumber==0){
            symbol_table[symbol.symbol].position=symbol_table[symbol.symbol].position+0;
        }else{
        symbol_table[symbol.symbol].position=symbol_table[symbol.symbol].position+module_addresses[symbol.moduleNumber-1];
        }
        cout << symbol.symbol << "=" << symbol_table[symbol.symbol].position;
        if (symbol_table[symbol.symbol].isMultDef) {
            std::cout << " Error: This variable is multiple times defined; first value used";
        }
        cout << std::endl;
    }
}


void firstPass(const std::string& input_file) {
    vector<Symbol> symbols;

    std::ifstream fileStream(input_file);
    if (!fileStream) {
        std::cerr << "Cannot open file" << std::endl;
        return; // Prefer returning over exit in a library or modular code
    }

    int total_code = 0;
    int module_num = 0;
    //int global_module_address=0;
    vector<int> module_addresses;
    while (fileStream.peek() != EOF) {
        Module module;
        
        //module.base_address=total_code;
        
        
        // Definition list
        int defcount = fetchInteger(fileStream);
        if(defcount < 0 || defcount > 16){
            parseerror(4);
            return;
        }
        vector<Symbol> tempSymbols;


        for(int i = 0; i < defcount; i++) {
            std::string symbol = fetchSymbol(fileStream);
            int relativePosition = fetchInteger(fileStream);
            // if (relativePosition >= module.codecount) {
            //     // cout<<"flag check!!!!!"<<module.codecount<<endl;
            //     cout << "Warning: Module " << module_num-1 << ": " << symbol << "=" << relativePosition << " valid=[0.." << module.codecount-1 << "] assume zero relative" << endl;
            //     relativePosition = 0; // Adjust to zero relative
            // }
            
            Symbol newSymbol;
            newSymbol.symbol = symbol;
            newSymbol.position = relativePosition;
            newSymbol.moduleNumber = module_num;
            newSymbol.isMultDef = false; // This is already the default value
            newSymbol.used = false;
            newSymbol.value = relativePosition; // Assuming 'value' should be set to 'position', adjust if needed
            
            if(symbol_table.find(symbol) == symbol_table.end()){
                symbol_table[symbol] = newSymbol;
                symbols.push_back(newSymbol);
            }
            else{
                newSymbol.isMultDef=true;
                symbol_table[symbol].isMultDef=true;
            }
            tempSymbols.push_back(newSymbol);
            //int absolutePosition = module.base_address + relativePosition;
            //createSymbol(symbol, absolutePosition, module_num);
        }
        
        // Use list
        int usecount = fetchInteger(fileStream);
        if(usecount < 0 || usecount > 16){
            parseerror(5);
            return;
        }
        
        // Assuming use list symbols are not processed in pass1, so we skip reading them
        for (int i = 0; i < usecount; i++) {
            fetchSymbol(fileStream); // Skip symbols in the use list
        }

        // Program text
        int codecount = fetchInteger(fileStream);
        //checking for exceeding module size
        for (auto& sym : tempSymbols) {
            auto it = symbol_table.find(sym.symbol);
            //symbol_table[sym.symbol].position = sym.position + module.base_address;
                
                if(sym.isMultDef==true){
                    cout << "Warning: Module " << sym.moduleNumber << ": " << sym.symbol << " redefinition ignored" << endl;
                }
            
            else if (sym.position >= codecount) {
                cout << "Warning: Module " << module_num << ": " << sym.symbol << "=" << sym.position << " valid=[0.."<<codecount-1<<"] assume zero relative" << endl;
                //sym.position = 0; // Adjust to zero relative
                symbol_table[sym.symbol].position=0;
            }
            //sym.position=sym.position + module.base_address;
            
            //symbol_table[sym.symbol].position = sym.position + module.base_address;
            
            //createSymbol(sym.first, sym.second + module.base_address , module_num);
        }

        module.base_address=total_code;
        total_code += codecount;
        //cout<<"total_code="<<total_code<<endl;
        //module.base_address = total_code; // Assuming Module has a base_address field
        if(codecount < 0 || total_code > 512){
            parseerror(6);
            return;
        }
        module_addresses.push_back(total_code);
        module.codecount = codecount;
        
        for (int i = 0; i < codecount; i++) {
            std::string addressmode = fetchAddress(fileStream); // Adjusted to use std::istream
            int instr = fetchInteger(fileStream); // Adjusted to use std::istream
            // Assuming you process address mode and instruction here or add them to the module
        }
        modules.push_back(module);
        
        module_num++;
    }
    printSymbolTable(symbols,module_addresses);
}


void secondPass(const std::string& input_file) {
    std::ifstream fileStream(input_file);
    if (!fileStream) {
        std::cerr << "Cannot open file\n";
        return;
    }

    int count = 0; // Memory map count
    int module_num = 0; // Current module index
    unordered_map<string,bool> checker_for_symbols_in_DefList;
    map<string,bool> symbol_usage_status;
    std::cout << "\nMemory Map\n";

    while (module_num < modules.size()) {
        std::vector<std::string> uselist;
        std::unordered_set<std::string> actually_used;

        // Skip def list and use list as they were processed in the first pass
        int defcount = fetchInteger(fileStream);
        for(int i = 0; i < defcount; i++) {
            std::string symbol = fetchSymbol(fileStream);
            int relativePosition = fetchInteger(fileStream);
            // int absolutePosition = module.base_address + relativePosition;
            // createSymbol(symbol, absolutePosition, module_num);
            checker_for_symbols_in_DefList[symbol];
        } // Assume this function skips over the def list correctly

        int usecount = fetchInteger(fileStream);
        for (int i = 0; i < usecount; i++) {
            std::string symbol = fetchSymbol(fileStream);
            uselist.push_back(symbol);
        }

        int codecount = fetchInteger(fileStream);
        for (int i = 0; i < codecount; i++) {
            std::string addressmode = fetchAddress(fileStream);
            int instr = fetchInteger(fileStream);

            if (instr > 9999) {
                cout << setw(3) << setfill('0') << count << ": 9999 Error: Illegal opcode; treated as 9999\n";
                instr = 9999;
                count++;
                continue; // Move to the next instruction
            }

            int opcode = instr / 1000;
            int operand = instr % 1000;

            
            
            if(addressmode == "M"){
                // Assuming 'operand' specifies the module number (0-based indexing)
                if (operand >= modules.size()) {
                    // Handle error: Illegal module operand
                    cout << setw(3) << setfill('0') << count << ": ";
                    cout << opcode << setw(3) << setfill('0') << 0;
                    cout << " Error: Illegal module operand ; treated as module=0\n";
                } else {
                    // Fetch the base address of the specified module
                    int moduleBaseAddress = modules[operand].base_address;
                    // Adjust the instruction with the module's base address
                    instr = (opcode * 1000) + moduleBaseAddress;
                    cout << setw(3) << setfill('0') << count << ": ";
                    cout << setw(4) << setfill('0') << instr << endl;
                }
            }else if (addressmode == "I") {
                // Immediate address mode, print as is
                string err;
                if (operand >= 900)
                {
                    operand = 999;
                     err = "Error: Illegal immediate operand; treated as 999";
                }
                else 
                {
                     err= "";
                }
                std::cout << std::setfill('0') << std::setw(3) << count << ": " << instr / 1000 << std::setw(3) << operand <<" "<< err << endl;
            } else if (addressmode == "A") {
                // Absolute address mode
                if (operand < Machine_Size) {
                    std::cout << std::setfill('0') << std::setw(3) << count << ": " << std::setw(4) << instr << endl;
                } else {
                    // Handle error: Absolute address exceeds machine size
                    std::cout << std::setfill('0') << std::setw(3) << count << ": " << std::setw(4) << (opcode * 1000) << " Error: Absolute address exceeds machine size; zero used\n";
                }
            } else if (addressmode == "R") {
                // Relative address mode
                operand += modules[module_num].base_address; // Adjust with the module's base address
                if (operand < modules[module_num].base_address + modules[module_num].codecount) {
                    instr = opcode * 1000 + operand;
                    std::cout << std::setfill('0') << std::setw(3) << count << ": " << std::setw(4) << instr << endl;
                } else {
                    // Handle error: Relative address exceeds module size
                    std::cout << std::setfill('0') << std::setw(3) << count << ": " << std::setw(4) << (opcode * 1000 + modules[module_num].base_address) << " Error: Relative address exceeds module size; relative zero used\n";
                }
            } else if (addressmode == "E") {
                // External address mode
                if (operand < usecount) {
                    std::string symbol = uselist[operand];
                    symbol_usage_status[symbol]=true;
                    actually_used.insert(symbol);
                    if (symbol_table.find(symbol) != symbol_table.end()) {
                        // Symbol is defined
                        Symbol& sym = symbol_table[symbol];
                        sym.used = true;
                        instr = opcode * 1000 + sym.position;
                        std::cout << std::setfill('0') << std::setw(3) << count << ": " << std::setw(4) << instr << endl;
                    } else {
                        // Handle error: Symbol not defined
                        std::cout << std::setfill('0') << std::setw(3) << count << ": " << std::setw(4) << (opcode * 1000) << " Error: " << symbol << " is not defined; zero used\n";
                    }
                } else {
                    // Handle error: External address exceeds length of uselist
                    // Here, directly print the error without attempting to resolve a symbol
                    instr = opcode * 1000; // Reset operand to 0
                    std::cout << std::setfill('0') << std::setw(3) << count << ": " << std::setw(4) << instr << " Error: External operand exceeds length of uselist; treated as relative=0\n";
                }
            }

            count++;
        }

        int index=0;
        // Check for symbols that were defined but not used
        for (const auto& sym : uselist) {
            if (actually_used.find(sym) == actually_used.end()) {
                std::cout << "Warning: Module " << module_num << ": uselist["<<index<<"]=" << sym << " was not used\n";
            }
            ++index;
        }

        module_num++;
    }

    // Check for symbols that were defined but never used
    for (const auto& entry : symbol_table) {
        if (!entry.second.used && !checker_for_symbols_in_DefList[entry.first]) {
            std::cout << "\nWarning: Module " << entry.second.moduleNumber << ": " << entry.first << " was defined but never used";
        }
    }

    fileStream.close();
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <inputfile>" << std::endl;
        return 1;
    }
    std::string inputFile = argv[1];

    //     clock_t time;
    // #ifdef DEBUG
    //     freopen("input.txt", "r", stdin);
    //     freopen("output.txt", "w", stdout);
    //     time = clock();
    // #endif

    //     ios_base::sync_with_stdio(false);
    //     cin.tie(NULL);

    //     ll n = 1;

    //     //cin >> n;
    //     rep(i, n) {
    //         //cout << "Case #" << i + 1 << ": ";
    //         solve();

    //     }
    // #ifdef DEBUG
    //     cout << endl << ((ld)clock() - time) / CLOCKS_PER_SEC;
    // #endif

    firstPass(inputFile);
    //printSymbolTable();
    secondPass(inputFile);

    return 0;
}