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

#define rep(i,b) for(ll (i) = 0 ;i<(b);i++)
#define per(i,b) for(ll (i) = ll(b)-1;i>=0;i--)
#define reep(i,a,b) for(ll (i)=ll(a);i<(b);i++)
#define peer(i,b,a) for(ll (i)=ll(b)-1;i>=(a);i--)
#define all(x) x.begin(),(x).end()
#define pb(x) push_back(x)
#define mp(x, y) make_pair(x, y)
#define sz(x) (ll) x.size()
#define len() length()
#define ff first
#define ss second
#define ret return

#define printvec(a) for(auto _ : a) cout << _ << ' '; cout << '\n';
#define print2(a, b) cout << a << ' ' << b << endl;
#define print3(a, b, c) cout << a << ' ' << b << ' ' << c << endl;
#define print4(a, b, c, d) cout << a << ' ' << b << ' ' << c << ' ' << d << endl;


typedef long long ll;
typedef unsigned long long ull;
typedef long double ld;

typedef vector<int> vi;
typedef vector<vi> vvi;
typedef vector<ll> vll;
typedef vector<vll> vvl;
typedef vector<char> vc;
typedef vector<vc> vvc;
typedef pair<int, int> pii;
typedef pair<ll, ll> pll;
typedef pair<string, string> pss;
typedef map<int, int> mii;
typedef unordered_map<int, int> umap_ii;
typedef unordered_map<string, int> umap_si;

/**
 * Limits in C++ for reference
 * _____________________________________________________________________________________
 * |Sr| Macro Name | Description                     | Value
 * |No|____________|_________________________________|__________________________________
 * |1.| ULLONG_MAX | Maximum value unsigned long long| 18,446,744,073,709,551,615 (10^20)
 * |2.| LLONG_MAX  | Maximum value long long         | 9,223,372,036,854,775,807 (10^19)
 * |3.| LLONG_MIN  | Minimum value long long         |-9,223,372,036,854,775,808 -1*(10^19)
 * |4.| INT_MAX    | Maximum value int               | 2,147,483,647 (10^10)
 * |5.| INT_MIN    | Minimum value int               |-2,147,483,648 (10^10)
*/


const int Machine_Size=512;
int lineNum=0, offset=1;
string line;
const char* delimeter= "\t\n";
bool fileNotEnd;


// void solve()
// {
    
// char s[1000];
//         scanf("%s",s);
//         for(int i = 0; i < strlen(s); ++i ) {
//             bool flag = false;
//             if(s[i] == 'U' && s[i-1] == 'W' && s[i+1] == 'B')   continue;
//             else if(s[i] == 'W' && s[i+1] == 'U' && s[i+2] == 'B')  continue;
//             else if(s[i] == 'B' && s[i-1] == 'U' && s[i-2] == 'W')  continue;
//             printf("%c",s[i] );
//             if(s[i+1] == 'W' && s[i+2] == 'U' && s[i+3] == 'B')
//                 printf(" ");
//         }
//         printf("\n");   
    
// }

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

string fetchNextToken(std::ifstream& fileStream) {
    static string currentToken;
    static std::istringstream currentStream;
    static bool firstCall = true;

    if (firstCall || currentStream.eof()) {
        if (!getline(fileStream, line)) {
            fileNotEnd = false; // End of file
            return "";
        }
        currentStream.clear(); // Clear any errors.
        currentStream.str(line); // Load the new line
        firstCall = false;
        lineNum++;
        offset = 1; // Reset offset with each new line
    }

    if (!(currentStream >> currentToken)) {
        firstCall = true; // Trigger fetching a new line on the next call
        return fetchNextToken(fileStream); // Recursively fetch the next token if the current line is exhausted
    }

    // Calculate offset for the next token by finding the position of the current token in the line
    offset = line.find(currentToken, offset - 1) + 1;
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

// void demoSymbolTable() {
//     // Adding some symbols for demonstration
//     createSymbol("START", 100, 1);
//     createSymbol("LOOP", 150, 1);
//     createSymbol("END", 200, 1);

//     // Attempting to add a symbol that already exists
//     createSymbol("LOOP", 250, 2);

//     // Printing the symbol table
//     std::cout << "Symbol Table\n";
//     for (const auto& entry : symbol_table) {
//         std::cout << entry.first << "=" << entry.second.position;
//         if (entry.second.isMultDef) {
//             std::cout << " Error: This variable is multiple times defined; first value used";
//         }
//         std::cout << std::endl;
//     }
// }

void createSymbol(const std::string& symbolName, int position, int moduleNumber) {
    // Check if the symbol already exists in the table
    auto it = symbol_table.find(symbolName);
    if (it != symbol_table.end()) {
        // If the symbol exists, mark it as multiply defined
        cout << "Warning: Module " << moduleNumber-1 << ": " << symbolName << " redefinition ignored" << endl;
        it->second.isMultDef = true;
    } else {
        // If the symbol doesn't exist, add it to the table
        Symbol newSymbol;
        newSymbol.symbol = symbolName;
        newSymbol.position = position;
        newSymbol.moduleNumber = moduleNumber;
        newSymbol.isMultDef = false; // This is already the default value
        newSymbol.used = false;
        newSymbol.value = position; // Assuming 'value' should be set to 'position', adjust if needed

        symbol_table[symbolName] = newSymbol;
    }
}

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

void printSymbolTable() {
    cout << "Symbol Table" << std::endl;
    for (const auto& entry : symbol_table) {
        const std::string& symbol = entry.first;
        const Symbol& symData = entry.second;

        cout << symbol << "=" << symData.position;
        if (symData.isMultDef) {
            std::cout << " Error: This variable is multiple times defined; first value used";
        }
        cout << std::endl;
    }
}


void firstPass(const std::string& input_file) {
    std::ifstream fileStream(input_file);
    if (!fileStream) {
        std::cerr << "Cannot open file" << std::endl;
        return; // Prefer returning over exit in a library or modular code
    }

    int total_code = 0;
    int module_num = 1;

    while (fileStream.peek() != EOF) {
        Module module;
        module.base_address=total_code;
        
        
        // Definition list
        int defcount = fetchInteger(fileStream);
        if(defcount < 0 || defcount > 16){
            parseerror(4);
            return;
        }
        
        for(int i = 0; i < defcount; i++) {
            std::string symbol = fetchSymbol(fileStream);
            int relativePosition = fetchInteger(fileStream);
            int absolutePosition = module.base_address + relativePosition;
            createSymbol(symbol, absolutePosition, module_num);
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
        module.base_address = total_code; // Assuming Module has a base_address field
        total_code += codecount;
        if(codecount < 0 || total_code > 512){
            parseerror(6);
            return;
        }
        module.codecount = codecount;
        
        for (int i = 0; i < codecount; i++) {
            std::string addressmode = fetchAddress(fileStream); // Adjusted to use std::istream
            int instr = fetchInteger(fileStream); // Adjusted to use std::istream
            // Assuming you process address mode and instruction here or add them to the module
        }
        modules.push_back(module);
        
        module_num++;
    }

}


void secondPass(const std::string& input_file) {
    std::ifstream fileStream(input_file);
    if (!fileStream) {
        std::cerr << "Cannot open file\n";
        return;
    }

    int count = 0; // Memory map count
    int module_num = 0; // Current module index


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
                    cout << opcode * 1000 << setw(3) << setfill('0') << 0;
                    cout << " Error: Illegal module operand; treated as module=0\n";
                } else {
                    // Fetch the base address of the specified module
                    int moduleBaseAddress = modules[operand].base_address;
                    // Adjust the instruction with the module's base address
                    instr = (opcode * 1000) + moduleBaseAddress;
                    cout << setw(3) << setfill('0') << count << ": ";
                    cout << setw(4) << setfill('0') << instr << "\n";
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
                std::cout << std::setfill('0') << std::setw(3) << count << ": " << err <<  std::setw(4) << instr << "\n";
            } else if (addressmode == "A") {
                // Absolute address mode
                if (operand < Machine_Size) {
                    std::cout << std::setfill('0') << std::setw(3) << count << ": " << std::setw(4) << instr << "\n";
                } else {
                    // Handle error: Absolute address exceeds machine size
                    std::cout << std::setfill('0') << std::setw(3) << count << ": " << std::setw(4) << (opcode * 1000) << " Error: Absolute address exceeds machine size; zero used\n";
                }
            } else if (addressmode == "R") {
                // Relative address mode
                operand += modules[module_num].base_address; // Adjust with the module's base address
                if (operand < modules[module_num].base_address + modules[module_num].codecount) {
                    instr = opcode * 1000 + operand;
                    std::cout << std::setfill('0') << std::setw(3) << count << ": " << std::setw(4) << instr << "\n";
                } else {
                    // Handle error: Relative address exceeds module size
                    std::cout << std::setfill('0') << std::setw(3) << count << ": " << std::setw(4) << (opcode * 1000 + modules[module_num].base_address) << " Error: Relative address exceeds module size; relative zero used\n";
                }
            } else if (addressmode == "E") {
                // External address mode
                if (operand < usecount) {
                    std::string symbol = uselist[operand];
                    actually_used.insert(symbol);
                    if (symbol_table.find(symbol) != symbol_table.end()) {
                        // Symbol is defined
                        Symbol& sym = symbol_table[symbol];
                        sym.used = true;
                        instr = opcode * 1000 + sym.position;
                        std::cout << std::setfill('0') << std::setw(3) << count << ": " << std::setw(4) << instr << "\n";
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
        if (!entry.second.used) {
            std::cout << "\nWarning: Module " << entry.second.moduleNumber - 1 << ": " << entry.first << " was defined but never used";
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
    printSymbolTable();
    secondPass(inputFile);

    return 0;
}