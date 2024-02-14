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

const ll MOD = 1000000007;
const int Machine_Size=512;
int lineNum, offset;
size_t linecap, linelen;
const char* delimeter= "\t\n";


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


void __parseerror(int errcode)
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

void printError(int errcode){
    static string errstr[] = {
        "Error: Absolute address exceeds machine size; zero used", // [0]
        "Error: Relative address exceeds module size; zero used", // [1]
        "Error: External address exceeds length of uselist; treated as immediate", // [2]  
        "Error: This variable is multiple times defined; first value used", // [3] 
        "TOO_MANY_DEF_IN_MODULE", // [4] > 16 
        "TOO_MANY_USE_IN_MODULE", // [5] > 16
        "TOO_MANY_INSTR" // [6] total num_instr exceeds memory size (512)  
    };
    printf("Parse Error line %d offset %d: %s\n",lineNum, offset, errstr[errcode]);
}

char* fetchNextToken(std::ifstream& fileStream) {
    static char* token = nullptr;
    static bool firstCall = true;

    if (firstCall || token == nullptr) {
        if (!getline(fileStream, line)) {
            fileNotEnd = false; // End of file
            return nullptr;
        }
        static char* lineCStr = nullptr;
        delete[] lineCStr; // Free previous buffer
        lineCStr = new char[line.length() + 1];
        std::strcpy(lineCStr, line.c_str());
        token = std::strtok(lineCStr, delimeter);
        firstCall = false;
        lineNum++;
    } else {
        token = std::strtok(nullptr, delimeter); // Get next token from the current line
    }

    if (token == nullptr) {
        firstCall = true; // Reset for the next call
        return fetchNextToken(fileStream); // Attempt to fetch the next token recursively
    }

    offset = token - line.c_str() + 1; // Update offset
    return token;
}

bool isValidInteger(const string& input) {
    return all_of(input.begin(), input.end(), isdigit);
}

bool isValidSymbol(const string& input) {
    regex pattern("[a-zA-Z][a-zA-Z0-9]*");
    return regex_match(input, pattern);
}

bool isValidAddress(const string& input) {
    const set<string> validAddresses = {"A", "E", "I", "R"};
    return validAddresses.find(input) != validAddresses.end();
}

struct Symbol {
    int position;
    bool isMultDef = false; // False by default, true if multiply defined
    int moduleNumber;
};

map<string, Symbol> symbol_table;

void createSymbol(const char* symbolName, int value, int moduleNumber) {
    std::string symbol(symbolName);
    
    // Check if the symbol already exists in the table
    auto it = symbol_table.find(symbol);
    if (it != symbol_table.end()) {
        // If the symbol exists, mark it as multiply defined
        it->second.isMultDef = true;
    } else {
        // If the symbol doesn't exist, add it to the table
        symbol_table[symbol] = Symbol{value, false, moduleNumber};
    }
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


int fetchInteger(FILE* filePtr) {
    char* token = fetchNextToken(filePtr);
    if (token == nullptr) return -1;
    std::string tokenStr(token);
    if (!isValidInteger(tokenStr)) {
        __parseerror(0);
        exit(EXIT_FAILURE);
    }
    return stoi(tokenStr);
}

string fetchSymbol(FILE* filePtr) {
    char* token = fetchNextToken(filePtr);
    if (token == nullptr || !isValidSymbol(token)) {
        __parseerror(1);
        exit(EXIT_FAILURE);
    } else if (std::strlen(token) > 16) {
        __parseerror(3);
        exit(EXIT_FAILURE);
    }
    return string(token);
}

string fetchAddress(FILE* filePtr) {
    char* token = fetchNextToken(filePtr);
    if (token == nullptr || !isValidAddress(token)) {
        __parseerror(2);
        exit(EXIT_FAILURE);
    }
    return string(token);
}

void printSymbolTable() {
    std::cout << "Symbol Table" << std::endl;
    for (const auto& entry : symbol_table) {
        const std::string& symbol = entry.first;
        const Symbol& symData = entry.second;

        std::cout << symbol << "=" << symData.position;
        if (symData.isMultDef) {
            std::cout << " Error: This variable is multiple times defined; first value used";
        }
        std::cout << std::endl;
    }
}

// void firstPass(char* inputFile) {
//     FILE* fptr = fopen(inputFile, "r");
//     if (!fptr) {
//         printf("Cannot open file\n");
//         return;
//     }

//     int offset = 0, totalCode = 0, moduleNum = 0;
    
//     while (!feof(fptr)) {
//         int defCount = fetchInteger(fptr);
//         if (defCount == -1) break; // End of file or error reading defCount

//         if (defCount > 16) {
//             __parseerror(4);
//             exit(EXIT_FAILURE);
//         }

//         for (int i = 0; i < defCount; ++i) {
//             string symbol = fetchSymbol(fptr);
//             int val = fetchInteger(fptr);
//             // Additional checks and symbol table updates go here
//         }

//         int useCount = fetchInteger(fptr);
//         if (useCount > 16) {
//             __parseerror(5);
//             exit(EXIT_FAILURE);
//         }

//         for (int i = 0; i < useCount; ++i) {
//             // Just fetching symbols for use list; assuming the logic is similar to defCount
//             fetchSymbol(fptr);
//         }

//         int codeCount = fetchInteger(fptr);
//         totalCode += codeCount;
//         if (totalCode > 512) {
//             __parseerror(6);
//             exit(EXIT_FAILURE);
//         }

//         for (int i = 0; i < codeCount; ++i) {
//             // Assuming similar processing for instruction entries
//             fetchAddress(fptr);
//             fetchInteger(fptr);
//         }

//         offset += codeCount;
//         // Update module number, symbol table, etc., as necessary
//         moduleNum++;
//     }

//     fclose(fptr);
// }

// void firstPass(char* pointr) {
//     ifstream file(pointr);
//     char* identifier= pointr[1];
//     int location;
//     char* opcode;
//     int operandValue;

//     while (!endOfFileCheck()) {
//         int definitionCount = readInteger();
        
//         if (definitionCount < 0) {
//             return; // Exit on EOF or read error
//         }

//         if (definitionCount > 16) {
//             errorReport(4); // Too many definitions error
//         } else {
//             for (int i = 0; i < definitionCount; ++i) {
//                 identifier = fetchSymbol();
//                 if (!identifier) {
//                     adjustOffsetForError(previousLineLength);
//                     errorReport(1); // Symbol read error
//                 }
//                 location = readInteger();
//                 if (location < 0) {
//                     adjustOffsetForError(previousLineLength);
//                     errorReport(0); // Integer read error
//                 }
//                 if (!inSecondPass) {
//                     registerSymbol(identifier, location);
//                 }
//             }
//         }

//         int usageCount = readInteger();
//         if (usageCount < 0) {
//             adjustOffsetForError(previousLineLength + 1);
//             errorReport(0); // EOF or read error on usage count
//         } else if (usageCount > 16) {
//             errorReport(5); // Too many usages error
//         } else {
//             for (int i = 0; i < usageCount; ++i) {
//                 identifier = fetchSymbol();
//                 if (!identifier) {
//                     adjustOffsetForError(previousLineLength + 1);
//                     errorReport(1); // Symbol read error within usage list
//                 }
//                 // Usage list processing for Pass2 would be different
//             }
//         }

//         int instructionCount = readInteger();
//         if (instructionCount < 0) {
//             adjustOffsetForError(previousLineLength + 1);
//             errorReport(0); // EOF or read error on instruction count
//         } else if (instructionCount > 512) {
//             errorReport(6); // Instruction count exceeds limit
//         } else {
//             if (instructionCountList[currentModule] + instructionCount > 512) {
//                 errorReport(6); // Total instruction count exceeds limit
//                 return;
//             }
//             instructionCountList[currentModule] += instructionCount;
//             for (int i = 0; i < instructionCount; ++i) {
//                 opcode = fetchAddressMode();
//                 if (!opcode) {
//                     adjustOffsetForError(previousLineLength + 1);
//                     errorReport(2); // Opcode/address mode read error
//                 }
//                 operandValue = fetchOperand();
//                 if (operandValue < 0) {
//                     adjustOffsetForError(previousLineLength + 1);
//                     errorReport(2); // Operand read error
//                 }
//                 // Instruction processing for Pass2 would be different
//             }
//         }
//         currentModule++; // Move to the next module
//     }
// }

// // void firstPass(const char* pointr) {
// //     std::ifstream file(pointr);
// //     if (!file.is_open()) {
// //         std::cerr << "Cannot open file" << std::endl;
// //         exit(2); // Error code for file not opening.
// //     }

// //     while (true) {
// //         int defcount = fetchInteger(file);
// //         if (defcount < 0) break; // End of file or error reading.

// //         for (int i = 0; i < defcount; ++i) {
// //             std::string sym = fetchSymbol(file);
// //             int val = fetchInteger(file);
// //             createSymbol(Symbol, val);
// //         }

// //         int usecount = fetchInteger(file);
// //         // detailed use processing for pass1.

// //         int instcount = fetchInteger(file);
// //         for (int i = 0; i < instcount; ++i) {
// //             char addressmode = fetchAddress(file);
// //             int operand = fetchInteger(file);
// //             // detailed instruction processing for pass1.
// //         }
// //     }

// //     file.close();
// // }


// int main(int argc, char *argv[])
// {

//     char *pointr= argv[1];
// //     clock_t time;
// // #ifdef DEBUG
// //     freopen("input.txt", "r", stdin);
// //     freopen("output.txt", "w", stdout);
// //     time = clock();
// // #endif

// //     ios_base::sync_with_stdio(false);
// //     cin.tie(NULL);

// //     ll n = 1;

// //     //cin >> n;
// //     rep(i, n) {
// //         //cout << "Case #" << i + 1 << ": ";
// //         solve();

// //     }
// // #ifdef DEBUG
// //     cout << endl << ((ld)clock() - time) / CLOCKS_PER_SEC;
// // #endif

//     firstPass(pointr);
//     printSymbolTable();
//     //secondPass(filename);
// }


void pass1(char* input_file){
    FILE *fptr;
    fptr = fopen(input_file, "r");
    if(fptr == NULL){
        printf("cannot open file");
    }
    
    int offset = 0;
    int total_code = 0;
    int module_num = 0;

    while(!feof(fptr)){
        Module module;
        vector<Symbol> deflist;
        
        // def list - defcount pairs of (S, R)
        int defcount = readInt(fptr);
        if(defcount >= 0 && defcount <= 16){
            module.defcount = defcount;
            for(int i = 0; i < defcount; i++) {
                string symbol = readSymbol(fptr);
                int val = readInt(fptr);
                Symbol s;
                s.txt = symbol;
                s.position = val;
                s.module_num = module_num + 1;
                deflist.push_back(s);
            }
        } else if(defcount > 16){
            parseerror(4);
            exit(0);
        }
        
        // use list - usecount symbols
        int usecount = readInt(fptr);
        if(usecount >= 0 && usecount <=16){
            module.usecount = usecount;
            for (int i = 0; i < usecount; i++) {
                string symbol = readSymbol(fptr);
            }
        } else if(usecount > 16){
            parseerror(5);
            exit(0);
        }
        
        // program text - codecount pairs of (type, instr)
        int codecount = readInt(fptr);
        total_code += codecount;
        if(codecount >= 0 && total_code <= 512){
            module.codecount = codecount;
            module.base_address = offset;
            for (int i = 0; i < codecount; i++) {
                string addressmode = readIEAR(fptr);
                int instr = readNumber(fptr);
            }
            offset += codecount;
            modules.push_back(module);
        }
        else if(total_code > 512){
            parseerror(6);
            exit(0);
        }
        // error_check[5] - is val > codecount? => set it as 0 (relative to module)
        for(int i = 0; i < deflist.size(); i++){
            Symbol s = deflist[i];
            int val = s.position;
            if(symbol_table.find(s.txt) == symbol_table.end()){ // if symbol not defined
                if(val >= codecount){ // display warning, 0 relative offset
                    printf("Warning: Module %d: %s too big %d (max=%d) assume zero relative\n", module_num + 1, s.txt.c_str(), val, codecount - 1);
                    val = 0;
                }
            }
            createSymbol(s.txt, val + (offset - codecount), module_num + 1);
        }
        module_num++;
    }

    // iterate and print symbol table
    printf("Symbol Table\n");
    for(map<string, Symbol>::iterator i = symbol_table.begin(); i != symbol_table.end(); ++i){
        // error_check[2]
        if(!(*i).second.is_mult_def){
            printf("%s=%d\n", (*i).first.c_str(), (*i).second.position);
        } else {
            printf("%s=%d Error: This variable is multiple times defined; first value used\n", (*i).first.c_str(), (*i).second.position);
        }
    }
    fclose(fptr);
}

void pass2(char* input_file){
    FILE *fptr;
    fptr = fopen(input_file, "r");
    if(fptr == NULL){
        printf("cannot open file");
    }

    int count = 0;
    int module_num = 0;

    printf("\nMemory Map\n");

    while(!feof(fptr)){

        vector<string> uselist;
        unordered_set<string> actually_used; 
        
        // def list - defcount pairs of (S, R)
        int defcount = readInt(fptr);
        if(defcount >= 0){
            for(int i = 0; i < defcount; i++) {
                string symbol = readSymbol(fptr);
                int val = readInt(fptr);
            }
        }
        
        // use list - usecount symbols
        int usecount = readInt(fptr);
        if(usecount >= 0){
            for (int i = 0; i < usecount; i++) {
                string symbol = readSymbol(fptr);
                uselist.push_back(symbol);
                if(symbol_table.find(symbol) == symbol_table.end()){
                    // ERROR TO BE HANDLED - handled in E instr case
                } else {
                    symbol_table.at(symbol).in_use_list = true;
                }
            }
        }
        
        // program text - codecount pairs of (type, instr)
        int codecount = readInt(fptr);
        if(codecount >= 0){
            for (int i = 0; i < codecount; i++) {
                string addressmode = readIEAR(fptr);
                int  instr = readNumber(fptr);
                int opcode = instr / 1000;
                int operand = instr % 1000;
                // error_check[11] - contains error_check[10]
                if(opcode <= 9){ // valid

                    // make relevant modifications to instr
                    if(!strcmp(addressmode.c_str(), "I")){
                        printf("%0.3d: %0.4d", count, instr);
                    } 
                    else if(!strcmp(addressmode.c_str(), "A")){
                        // error_check[8] - abs addr <= size of machine (i.e. 512)
                        if(operand < 512){
                            printf("%0.3d: %0.4d", count, instr);
                        } else {
                            operand = 0;
                            instr = (opcode * 1000) + operand;
                            printf("%0.3d: %0.4d Error: Absolute address exceeds machine size; zero used", count, instr);
                        }
                    }
                    else if(!strcmp(addressmode.c_str(), "E")){
                        // error_check[6]
                        if(operand < uselist.size()){ // valid
                            string sym = uselist[operand];
                            // error_check[3] - check if symbol present in symbol_table
                            if(symbol_table.find(sym) == symbol_table.end()){ // symbol not present
                                instr = (opcode * 1000) +  (0);
                                actually_used.insert(sym);
                                printf("%0.3d: %0.4d Error: %s is not defined; zero used", count, instr, sym.c_str());
                            } else { // symbol is present
                                Symbol s = symbol_table.at(sym);
                                actually_used.insert(sym);
                                instr = (opcode * 1000) +  (s.position);
                                printf("%0.3d: %0.4d", count, instr);
                            }
                        } else { // error
                            printf("%0.3d: %0.4d Error: External address exceeds length of uselist; treated as immediate", count, instr);
                        }
                    }
                    else if(!strcmp(addressmode.c_str(), "R")){
                        // error_check[9]
                        if(operand < modules[module_num].codecount){ //VERIFY THIS
                            instr = (opcode * 1000) +  (operand + modules[module_num].base_address);
                            printf("%0.3d: %0.4d", count, instr);
                        } else {
                            operand = 0;
                            instr = (opcode * 1000) +  (operand + modules[module_num].base_address);
                            printf("%0.3d: %0.4d Error: Relative address exceeds module size; zero used", count, instr);
                        }
                    }

                } else { // error
                    instr = 9999;
                    opcode = instr / 1000;
                    operand = instr % 1000;
                    if(!strcmp(addressmode.c_str(), "I")) printf("%0.3d: %0.4d Error: Illegal immediate value; treated as 9999", count, instr);
                    else printf("%0.3d: %0.4d Error: Illegal opcode; treated as 9999", count, instr);
                }
                printf("\n");
                count++;
            }

            // error_check[7] - in uselist but not actually used
            for(int i = 0; i < uselist.size(); i++){
                if(actually_used.find(uselist[i]) == actually_used.end()){
                    printf("Warning: Module %d: %s appeared in the uselist but was not actually used\n", module_num + 1, uselist[i].c_str());
                }
            }
            module_num++;
        }
    }

    // (warning) error_check[4]
    for(map<string, Symbol>::iterator i = symbol_table.begin(); i != symbol_table.end(); ++i){
        if(!(*i).second.in_use_list){ 
            printf("Warning: Module %d: %s was defined but never used\n", (*i).second.module_num, (*i).second.txt.c_str());
        }
    }

    fclose(fptr);
}

int main(int argc, char *argv[]){

    if(argc==1) printf("\nNo Extra Command Line Argument Passed Other Than Program Name"); 
    char *filename = argv[1];

    pass1(filename);
    pass2(filename);

    return 0;