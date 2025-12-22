#include <iostream>        
#include <string>          
#include <vector>          
#include <unordered_map>   
#include <unordered_set>  
#include <set>  
#include <memory>          
#include <utility>         
#include <iomanip>         
#include <sstream>         
#include <optional>
#include <fstream>
#include <algorithm>
#include <cctype>   
#include <cstdlib>

using namespace std;

typedef long long int ll;

// ============================= JSON Parser ======================================
class SimpleJSON {
    public:
        static bool parseSymbols(const string content_recive, vector<unordered_map<string, string>>& symbols, const string& filename="") {
            string content;
            if(!filename.empty()){
                ifstream file(filename);
                if (!file.is_open()) {
                    cerr << "[Error] Cannot open file: " << filename << "\n";
                    return false;
                }
                
                string content_((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
                content = content_;
                file.close();
            } else {
                content = content_recive;
            }
            
            if (content.empty()) {
                cerr << "[Warning] JSON file is empty\n";
                return true;
            }
            
            size_t startPos = 0;
            size_t endPos = content.size();
            
            size_t arrayStart = content.find('[');
            if (arrayStart != string::npos) {
                startPos = arrayStart + 1;
            }
            
            size_t arrayEnd = content.find_last_of(']');
            if (arrayEnd != string::npos && arrayEnd > startPos) {
                endPos = arrayEnd;
            }
            
            string cleanContent = content.substr(startPos, endPos - startPos);
            
            size_t pos = 0;
            while (pos < cleanContent.size()) {
                while (pos < cleanContent.size() && isspace(cleanContent[pos])) pos++;
                if (pos >= cleanContent.size()) break;
                
                size_t objStart = cleanContent.find('{', pos);
                if (objStart == string::npos) break;
                
                size_t objEnd = findMatchingBrace(cleanContent, objStart);
                if (objEnd == string::npos) break;
                
                string objStr = cleanContent.substr(objStart + 1, objEnd - objStart - 1);
                unordered_map<string, string> obj;
                
                if (!parseObject(objStr, obj)) {
                    pos = objEnd + 1;
                    continue;
                }
                
                if (!obj.empty()) {
                    symbols.push_back(obj);
                }
                
                pos = objEnd + 1;
            }
            
            cout << "[Info] Parsed " << symbols.size() << " symbols from JSON\n";
            return true;
        }
        
    private:
        static size_t findMatchingBrace(const string& str, size_t start) {
            int count = 1;
            for (size_t i = start + 1; i < str.size(); i++) {
                if (str[i] == '{') count++;
                else if (str[i] == '}') count--;
                
                if (count == 0) return i;
            }
            return string::npos;
        }
        
        static bool parseObject(const string& objStr, unordered_map<string, string>& obj) {
            size_t pos = 0;
            while (pos < objStr.size()) {
                // Find key
                size_t keyStart = objStr.find('"', pos);
                if (keyStart == string::npos) break;
                
                size_t keyEnd = objStr.find('"', keyStart + 1);
                if (keyEnd == string::npos) break;
                
                string key = objStr.substr(keyStart + 1, keyEnd - keyStart - 1);
                
                // Find colon
                size_t colonPos = objStr.find(':', keyEnd);
                if (colonPos == string::npos) break;
                
                // Find value
                size_t valueStart = colonPos + 1;
                while (valueStart < objStr.size() && isspace(objStr[valueStart])) valueStart++;
                
                if (valueStart >= objStr.size()) break;
                
                string value;
                if (objStr[valueStart] == '"') {
                    // String value
                    size_t valueEnd = objStr.find('"', valueStart + 1);
                    if (valueEnd == string::npos) break;
                    value = objStr.substr(valueStart + 1, valueEnd - valueStart - 1);
                    pos = valueEnd + 1;
                } else if (objStr[valueStart] == '[') {
                    // Array value 
                    size_t arrayEnd = findMatchingBracket(objStr, valueStart);
                    if (arrayEnd == string::npos) break;
                    value = objStr.substr(valueStart, arrayEnd - valueStart + 1);
                    pos = arrayEnd + 1;
                } else if (objStr[valueStart] == '{') {
                    // Object value
                    size_t objectEnd = findMatchingBrace(objStr, valueStart);
                    if (objectEnd == string::npos) break;
                    value = objStr.substr(valueStart, objectEnd - valueStart + 1);
                    pos = objectEnd + 1;
                } else {
                    // Simple value (boolean, number, null)
                    size_t valueEnd = objStr.find_first_of(",}", valueStart);
                    if (valueEnd == string::npos) valueEnd = objStr.size();
                    value = objStr.substr(valueStart, valueEnd - valueStart);
                    
                    // Trim whitespace
                    size_t start = value.find_first_not_of(" \t\n\r");
                    size_t end = value.find_last_not_of(" \t\n\r");
                    if (start != string::npos && end != string::npos) {
                        value = value.substr(start, end - start + 1);
                    } else {
                        value = "";
                    }
                    pos = valueEnd;
                }
                
                if (!key.empty() && !value.empty()) {
                    obj[key] = value;
                }
                
                // Find next comma
                size_t commaPos = objStr.find(',', pos);
                if (commaPos == string::npos) break;
                pos = commaPos + 1;
            }
            return true;
        }
        
        static size_t findMatchingBracket(const string& str, size_t start) {
            int count = 1;
            for (size_t i = start + 1; i < str.size(); i++) {
                if (str[i] == '[') count++;
                else if (str[i] == ']') count--;
                
                if (count == 0) return i;
            }
            return string::npos;
        }
};

// this part is for phase 2 (Error Detection)

vector<pair<size_t,string>> InProcessError;

// end error detection part

// ============================= part 2 from phase 1 - Symbol Table ======================================

enum class IdentifierKind {
    Class,
    Interface,
    Method,
    Constructor,
    Field,
    Variable,
    Parameter,
    MainClass
};

struct TypeInfo {
    string name;      
    bool isArray = false;  
    int dimensions = 0;    
    
    TypeInfo() = default;
    TypeInfo(const string& n) : name(n) {}
};

struct ISymbol {
    virtual ~ISymbol() = default;
    virtual void print() const = 0;
    virtual string getName() const = 0;
    virtual string getScope() const = 0;
    virtual string getIndexInSymbolTable() const = 0;
    virtual size_t getLineInCode() const = 0;
};

struct IdentifierBase : public ISymbol {
    string name;
    IdentifierKind id_kind;
    string scope;
    string IndexInSymbolTable;
    size_t line = 0, col = 0;

    string getName() const override {
        return this->name;
    }
    
    string getScope() const override {
        return this->scope;
    }

    string getIndexInSymbolTable() const override {
        return this->IndexInSymbolTable;
    }

    size_t getLineInCode() const override {
        return this->line;
    }

    void print() const override {
        cout << "Identifier: " << name 
             << " (" << static_cast<int>(id_kind) << ")"
             << " Scope: " << scope
             << " @(" << line << "," << col << ")\n";
    }
};

struct InterfaceInfo : public IdentifierBase {
    void print() const override {
        cout << "[Interface] " << name << " (scope: " << scope << ")\n";
    }
};

struct ClassInfo : public IdentifierBase {
    string parentClass;
    bool isAbstract = false;
    vector<string> interfaces;

    void print() const override {
        cout << "[Class] " << name << " (scope: " << scope << ")";
        if (!parentClass.empty() && parentClass != "null")
            cout << " extends " << parentClass;
        if (isAbstract)
            cout << " (abstract)";
        cout << "\n";
    }
};

struct ParameterInfo : public IdentifierBase {
    TypeInfo type;

    ParameterInfo() {
        id_kind = IdentifierKind::Parameter;
    }
    
    ParameterInfo(const string& n, const string& t, const string& s) {
        name = n;
        type.name = t;
        scope = s;
        id_kind = IdentifierKind::Parameter;
    }

    void print() const override {
        cout << "[Parameter] " << name << " : " << type.name << " (scope: " << scope << ")\n";
    }
};

struct MethodInfo : public IdentifierBase {
    TypeInfo returnType;
    string accessModifier;
    vector<ParameterInfo> parameters; 
    bool isAbstract = false;
    bool isOverride = false;

    MethodInfo() {
        id_kind = IdentifierKind::Method;
    }

    void print() const override {
        cout << "[Method] " << name << " -> " << returnType.name << " (scope: " << scope << ")";
        if (isAbstract) 
            cout << " (abstract)";
        if (isOverride) 
            cout << " (override)";
        if (!accessModifier.empty())
            cout << " (" << accessModifier << ")";
        cout << "\n";
        
        if (!parameters.empty()) {
            cout << "              " << "  Parameters:\n";
            for (const auto& param : parameters) {
                cout << "                 " << "    - " << param.name << " : " << param.type.name << "\n";
            }
        }
    }
};

struct VariableInfo : public IdentifierBase {
    TypeInfo type;
    string accessModifier;
    string initialValue;

    VariableInfo() {
        id_kind = IdentifierKind::Variable;
    }

    void print() const override {
        cout << "[Variable] " << name << " : " << type.name << " (scope: " << scope << ")";
        if (!IndexInSymbolTable.empty())
            cout << " (index: " << IndexInSymbolTable << ")";
        if (!initialValue.empty() && initialValue != "null") 
            cout << " = " << initialValue;
        if (!accessModifier.empty())
            cout << " (" << accessModifier << ")";
        cout << "\n";
    }
};

struct FieldInfo : public VariableInfo {
    FieldInfo() {
        id_kind = IdentifierKind::Field;
    }

    void print() const override {
        cout << "[Field] " << name << " : " << type.name << " (scope: " << scope << ")";
        if (!accessModifier.empty())
            cout << " (" << accessModifier << ")";
        if (!initialValue.empty() && initialValue != "null") 
            cout << " = " << initialValue;
        cout << "\n";
    }
};

struct ConstructorInfo : public IdentifierBase {
    vector<ParameterInfo> parameters;
    string accessModifier;

    ConstructorInfo() {
        id_kind = IdentifierKind::Constructor;
    }

    void print() const override {
        cout << "[Constructor] " << name << " (scope: " << scope << ")";
        if (!accessModifier.empty())
            cout << " (" << accessModifier << ")";
        cout << "\n";
        if (!parameters.empty()) {
            cout<< "              " << "  Parameters:\n";
            for (const auto& param : parameters) {
                cout << "                 " << "    - " << param.name << " : " << param.type.name << "\n";
            }
        }
    }
};

struct Symbol {
    IdentifierKind kind;
    shared_ptr<ISymbol> data;

    Symbol() = default;

    template <typename TYPE2>
    Symbol(const TYPE2& obj, IdentifierKind k) {
        data = make_shared<TYPE2>(obj);
        kind = k;
    }

    void print() const {
        if (data) 
            data->print();
        else 
            cout << "[Empty Symbol]\n";
    }

    string getName() const {
        if (!data) 
            return "";
        return data->getName();
    }
    
    string getScope() const {
        if (!data)
            return "";
        return data->getScope();
    }

    string getIndexInSymbolTable() const {
        if (!data) 
            return "";
        return data->getIndexInSymbolTable();
    }

    size_t getLineInCode() const {
        if (!data) 
            return 0;
        return data->getLineInCode();
    }
};

class Scope {
private:
    string name;
    Scope* parent;
    unordered_map<string, Symbol> symbols;
    vector<unique_ptr<Scope>> children;
    string fullPath;

    void updateFullPath() {
        if (parent) {
            fullPath = parent->fullPath + "::" + name;
        } else {
            fullPath = name;
        }
        
        for (auto& child : children) {
            child->updateFullPath();
        }
    }

public:
    Scope(string n = "GLOBAL", Scope* p = nullptr) : name(move(n)), parent(p) {
        updateFullPath();
    }

    const string& getName() const { return name; }
    const string& getFullPath() const { return fullPath; }
    Scope* getParent() const { return parent; }

    bool insert(const Symbol& sym) {
        string idName = sym.getName();
        string symbolScope = sym.getScope();
        
        if (idName.empty()) {
            cerr << "[Error] Symbol without name in scope '" << fullPath << "'\n";
            return false;
        }
        
        // Allow duplicate names in different scopes
        string uniqueKey = idName + "@" + symbolScope;
        
        if (symbols.count(idName)) {
            // Check if it's truly a duplicate (same name, same scope)
            if (symbols[idName].getScope() == symbolScope) {
                // Don't print error for parameters - they might be duplicated legitimately
                if (sym.kind != IdentifierKind::Parameter) {
                    cerr << "[Warning] Duplicate symbol '" << idName << "' in scope '" << symbolScope << "' - keeping first occurrence\n";
                    InProcessError.push_back(make_pair(sym.data->getLineInCode(), "DuplicateVariableInScope"));
                }
                return false;
            }
        }
        
        symbols[idName] = sym;
        return true;
    }

    Symbol* lookup(const string& target) {
        auto it = symbols.find(target);
        if (it != symbols.end()) {
            return &(it->second);
        }
        
        Scope* currentParent = parent;
        while (currentParent) {
            auto parentIt = currentParent->symbols.find(target);
            if (parentIt != currentParent->symbols.end()) {
                return &(parentIt->second);
            }
            currentParent = currentParent->parent;
        }
        
        return nullptr;
    }

    Symbol* lookupInCurrentScope(const string& target) {
        auto it = symbols.find(target);
        if (it != symbols.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    Scope* addChild(const string& childName) {
        for (const auto& child : children) {
            if (child->getName() == childName) {
                return child.get();
            }
        }
        
        auto child = make_unique<Scope>(childName, this);
        Scope* ptr = child.get();
        children.push_back(move(child));
        updateFullPath();
        return ptr;
    }

    Scope* getChild(const string& childName) {
        for (const auto& child : children) {
            if (child->getName() == childName) {
                return child.get();
            }
        }
        return nullptr;
    }

    vector<Scope*> getAllChildren() {
        vector<Scope*> result;
        for (const auto& child : children) {
            result.push_back(child.get());
        }
        return result;
    }

    void dump(int indent = 0) const {
        string pad(indent, ' ');
        cout << pad << "Scope: " << name << " (full: " << fullPath << ")\n";
        
        for (const auto& [id, sym] : symbols) {
            cout << pad << "  - ";
            sym.print();
        }
        
        for (const auto& child : children) {
            child->dump(indent + 4);
        }
    }

    size_t getSymbolCount() const {
        return symbols.size();
    }

    const unordered_map<string, Symbol>& getSymbols() const {
        return symbols;
    }
};

class SymbolTable {
private:
    unique_ptr<Scope> global;
    Scope* current;
    vector<unordered_map<string, string>> symbolsData;
    unordered_map<string, int> nameToIndex;
    
    static string IdentifierKindToString(IdentifierKind k) {
        switch (k) {
            case IdentifierKind::Class: return "class";
            case IdentifierKind::Interface: return "interface";
            case IdentifierKind::Method: return "method";
            case IdentifierKind::Constructor: return "constructor";
            case IdentifierKind::Field: return "field";
            case IdentifierKind::Variable: return "variable";
            case IdentifierKind::Parameter: return "parameter";
            case IdentifierKind::MainClass: return "mainclass";
            default: return "unknown";
        }
    }
    
    static IdentifierKind stringToIdentifierKind(const string& str) {
        if (str == "class") return IdentifierKind::Class;
        if (str == "interface") return IdentifierKind::Interface;
        if (str == "method") return IdentifierKind::Method;
        if (str == "constructor") return IdentifierKind::Constructor;
        if (str == "field") return IdentifierKind::Field;
        if (str == "variable") return IdentifierKind::Variable;
        if (str == "parameter") return IdentifierKind::Parameter;
        if (str == "mainclass") return IdentifierKind::MainClass;
        return IdentifierKind::Variable;
    }
    
    Scope* ensureScope(const string& scopeName) {
        if (scopeName == "GLOBAL") {
            return global.get();
        }
        
        vector<string> scopeParts;
        stringstream ss(scopeName);
        string part;
        while (getline(ss, part, ':')) {
            if (!part.empty() && part != ":") {
                scopeParts.push_back(part);
            }
        }
        
        Scope* targetScope = global.get();
        for (const auto& part : scopeParts) {
            Scope* nextScope = targetScope->getChild(part);
            if (!nextScope) {
                nextScope = targetScope->addChild(part);
            }
            targetScope = nextScope;
        }
        
        return targetScope;
    }
    
    vector<ParameterInfo> parseParameters(const string& paramsStr, const string& parentScope) {
        vector<ParameterInfo> params;
        
        if (paramsStr.empty() || paramsStr == "[]") {
            return params;
        }
        
        vector<unordered_map<string, string>> paramData;
        SimpleJSON::parseSymbols(paramsStr, paramData);
        
        for (const auto& param : paramData) {
            if (param.count("name")) {
                ParameterInfo paramInfo;
                paramInfo.name = param.at("name");
                
                if (param.count("dataType")) {
                    paramInfo.type.name = param.at("dataType");
                } else if (param.count("type")) {
                    paramInfo.type.name = param.at("type");
                }
                
                if (param.count("scope")) {
                    paramInfo.scope = param.at("scope");
                } else {
                    paramInfo.scope = parentScope;
                }
                
                paramInfo.id_kind = IdentifierKind::Parameter;
                params.push_back(paramInfo);
            }
        }
        
        return params;
    }
    
public:
    SymbolTable() {
        global = make_unique<Scope>("GLOBAL");
        current = global.get();
    }
    
    bool loadFromJSON(const string& filename) {
        symbolsData.clear();
        nameToIndex.clear();
        
        if (!SimpleJSON::parseSymbols("", symbolsData, filename)) {
            cerr << "[Error] Failed to parse JSON file: " << filename << "\n";
            return false;
        }
        
        // First pass: Create all scopes
        cout << "[Info] Creating scope hierarchy...\n";
        set<string> createdScopes;
        for (const auto& sym : symbolsData) {
            if (sym.count("scope")) {
                string scopeName = sym.at("scope");
                if (createdScopes.count(scopeName) == 0) {
                    ensureScope(scopeName);
                    createdScopes.insert(scopeName);
                }
            }
        }
        
        // Second pass: Insert symbols
        cout << "[Info] Inserting symbols...\n";
        set<string> processedSymbols;
        int successCount = 0;
        
        for (size_t i = 0; i < symbolsData.size(); i++) {
            auto& sym = symbolsData[i];
            
            if (!sym.count("name") || !sym.count("symbolType")) {
                continue;
            }
            
            string name = sym["name"];
            string symbolType = sym["symbolType"];
            string scopeName = sym.count("scope") ? sym["scope"] : "GLOBAL";
            
            // Create unique key to prevent processing duplicates
            // string uniqueKey = name + "@" + scopeName + "@" + symbolType;
            
            // if (processedSymbols.count(uniqueKey)) {
            //     continue;
            // }
            // processedSymbols.insert(uniqueKey);
            
            IdentifierKind kind = stringToIdentifierKind(symbolType);
            Scope* targetScope = ensureScope(scopeName);
            Scope* previousCurrent = current;
            current = targetScope;
            
            bool success = false;
            
            if (kind == IdentifierKind::Class || kind == IdentifierKind::MainClass) {
                ClassInfo classInfo;
                classInfo.name = name;
                classInfo.id_kind = kind;
                classInfo.scope = scopeName;
                classInfo.IndexInSymbolTable = to_string(i);
                
                if (sym.count("parent") && sym["parent"] != "null") {
                    classInfo.parentClass = sym["parent"];
                }
                if (sym.count("isAbstract")) {
                    classInfo.isAbstract = (sym["isAbstract"] == "true");
                }
                
                Symbol s(classInfo, kind);
                success = insertSymbol(s);
            } 
            else if (kind == IdentifierKind::Interface) {
                InterfaceInfo intInfo;
                intInfo.name = name;
                intInfo.id_kind = kind;
                intInfo.scope = scopeName;
                intInfo.IndexInSymbolTable = to_string(i);
                
                Symbol s(intInfo, kind);
                success = insertSymbol(s);
            }
            else if (kind == IdentifierKind::Method) {
                MethodInfo methodInfo;
                methodInfo.name = name;
                methodInfo.id_kind = kind;
                methodInfo.scope = scopeName;
                methodInfo.IndexInSymbolTable = to_string(i);
                
                if (sym.count("returnType")) methodInfo.returnType.name = sym["returnType"];
                if (sym.count("access")) methodInfo.accessModifier = sym["access"];
                if (sym.count("isAbstract")) methodInfo.isAbstract = (sym["isAbstract"] == "true");
                if (sym.count("isOverride")) methodInfo.isOverride = (sym["isOverride"] == "true");
                
                // Parse parameters
                if (sym.count("parameters")) {
                    string methodScopeName = scopeName + "::" + name;
                    methodInfo.parameters = parseParameters(sym["parameters"], methodScopeName);
                }
                
                Symbol s(methodInfo, kind);
                success = insertSymbol(s);
                
                // Insert parameters in method scope
                Scope* methodScope = current->addChild(name);
                Scope* prevScope = current;
                current = methodScope;
                
                for (const auto& param : methodInfo.parameters) {
                    Symbol paramSymbol(param, IdentifierKind::Parameter);
                    current->insert(paramSymbol);

                    nameToIndex[param.name + param.scope] = static_cast<int>(successCount);
                    successCount++;
                }
                
                current = prevScope;
            }
            else if (kind == IdentifierKind::Constructor) {
                ConstructorInfo ctorInfo;
                ctorInfo.name = name;
                ctorInfo.id_kind = kind;
                ctorInfo.scope = scopeName;
                ctorInfo.IndexInSymbolTable = to_string(i);
                
                if (sym.count("access")) ctorInfo.accessModifier = sym["access"];
                
                // Parse parameters
                if (sym.count("parameters")) {
                    string ctorScopeName = scopeName + "::" + name;
                    ctorInfo.parameters = parseParameters(sym["parameters"], ctorScopeName);
                }
                
                Symbol s(ctorInfo, kind);
                success = insertSymbol(s);
                
                // Insert parameters in constructor scope
                Scope* ctorScope = current->addChild(name);
                Scope* prevScope = current;
                current = ctorScope;
                
                for (const auto& param : ctorInfo.parameters) {
                    Symbol paramSymbol(param, IdentifierKind::Parameter);
                    current->insert(paramSymbol);

                    nameToIndex[param.name + param.scope] = static_cast<int>(successCount);
                    successCount++;
                }
                
                current = prevScope;
            }
            else if (kind == IdentifierKind::Variable) {
                VariableInfo varInfo;
                varInfo.name = name;
                varInfo.id_kind = kind;
                varInfo.scope = scopeName;
                varInfo.IndexInSymbolTable = to_string(i);
                
                if (sym.count("dataType")) varInfo.type.name = sym["dataType"];
                if (sym.count("initialValue") && sym["initialValue"] != "null") {
                    varInfo.initialValue = sym["initialValue"];
                }
                if (sym.count("access")) varInfo.accessModifier = sym["access"];
                
                Symbol s(varInfo, kind);
                success = insertSymbol(s);
            } 
            else if (kind == IdentifierKind::Field) {
                FieldInfo fieldInfo;
                fieldInfo.name = name;
                fieldInfo.id_kind = kind;
                fieldInfo.scope = scopeName;
                fieldInfo.IndexInSymbolTable = to_string(i);
                
                if (sym.count("dataType")) fieldInfo.type.name = sym["dataType"];
                if (sym.count("initialValue") && sym["initialValue"] != "null") {
                    fieldInfo.initialValue = sym["initialValue"];
                }
                if (sym.count("access")) fieldInfo.accessModifier = sym["access"];
                
                Symbol s(fieldInfo, kind);
                success = insertSymbol(s);
            } 
            else if (kind == IdentifierKind::Parameter) {
                ParameterInfo paramInfo;
                paramInfo.name = name;
                paramInfo.id_kind = kind;
                paramInfo.scope = scopeName;
                paramInfo.IndexInSymbolTable = to_string(i);
                
                if (sym.count("dataType")) paramInfo.type.name = sym["dataType"];
                if (sym.count("type")) paramInfo.type.name = sym["type"];
                
                Symbol s(paramInfo, kind);
                success = insertSymbol(s);
            }
            
            current = previousCurrent;
            
            if (success) {
                nameToIndex[name+scopeName] = static_cast<int>(successCount);
                successCount++;
            }
        }
        
        cout << "[Info] Successfully loaded " << successCount << " symbols from JSON file\n";
        return true;
    }

    void enterScope(const string& name) {
        Scope* newScope = current->addChild(name);
        current = newScope;
    }

    void exitScope() {
        if (current->getParent()) {
            current = current->getParent();
        } else {
            cerr << "[Error] Already at global scope.\n";
        }
    }

    bool insertSymbol(const Symbol& s) {
        return current->insert(s);
    }

    Symbol* lookupSymbol(const string& name) {
        return current->lookup(name);
    }

    Symbol* lookupSymbolInCurrentScope(const string& name) {
        return current->lookupInCurrentScope(name);
    }

    Scope* getCurrentScope() const {
        return current;
    }

    Scope* getGlobalScope() const {
        return global.get();
    }

    void dump() const {
        cout << "\n==================== Symbol Table Scope Structure ====================\n";
        global->dump();
        cout << "=====================================================================\n\n";
    }
    
    void printSymbolsTable() const {
        cout << "\n==================== Symbol Table ====================\n";
        cout << left << setw(6) << "Index" 
             << setw(20) << "Name" 
             << setw(15) << "Kind" 
             << setw(15) << "Type"
             << setw(25) << "Scope" 
             << setw(15) << "Initial Value" << "\n";
        cout << string(96, '-') << "\n";
        
        for (size_t i = 0; i < symbolsData.size(); ++i) {
            const auto& sym = symbolsData[i];
            
            string name = sym.count("name") ? sym.at("name") : "N/A";
            string kind = sym.count("symbolType") ? sym.at("symbolType") : "N/A";
            string type = "N/A";
            string scope = sym.count("scope") ? sym.at("scope") : "GLOBAL";
            string initialValue = sym.count("initialValue") ? sym.at("initialValue") : "N/A";
            
            if (sym.count("dataType")) {
                type = sym.at("dataType");
            } else if (sym.count("returnType")) {
                type = sym.at("returnType");
            }
            
            cout << left << setw(6) << i
                 << setw(20) << (name.length() > 19 ? name.substr(0, 17) + ".." : name)
                 << setw(15) << (kind.length() > 14 ? kind.substr(0, 12) + ".." : kind)
                 << setw(15) << (type.length() > 14 ? type.substr(0, 12) + ".." : type)
                 << setw(25) << (scope.length() > 24 ? scope.substr(0, 22) + ".." : scope)
                 << setw(15) << (initialValue.length() > 14 ? initialValue.substr(0, 12) + ".." : initialValue) << "\n";
        }
        cout << "======================================================\n\n";
    }
    
    // optional<unordered_map<string, string>> lookup(const string& identName,const string& scope) const {
    //     auto it = nameToIndex.find(identName+scope);
    //     if (it != nameToIndex.end() && it->second < symbolsData.size()) {
    //         return symbolsData[it->second];
    //     }
    //     return nullopt;
    // }

    Scope* move_to_scope(const string& scopeName) const {
        if (scopeName.empty() || scopeName == "GLOBAL") {
            return global.get();
        }
        
        vector<string> scopeParts;
        stringstream ss(scopeName);
        string part;
        
        while (getline(ss, part, ':')) {
            if (!part.empty()) {
                scopeParts.push_back(part);
            }
        }
        
        if (scopeParts.empty()) {
            return global.get();
        }
        
        Scope* currentScope = global.get();
        
        for (const auto& part : scopeParts) {
            Scope* nextScope = currentScope->getChild(part);
            if (!nextScope) {
                cerr << "[Error] Scope '" << part << "' not found in hierarchy. Current path: " 
                    << currentScope->getFullPath() << "\n";
                return nullptr;
            }
            currentScope = nextScope;
        }
        
        return currentScope;
    }

    void lookup(const string& identName,const string& scope) const {
        auto it = move_to_scope(scope);

        if (!it) {
            cout << "\n[Error] Scope '" << scope << "' not found\n";
            return;
        }

        bool flag=false;
        auto table_scope = it->getSymbols();
        for(auto& [scope_name,scope_member] : table_scope){
            if(scope_name==identName){
                cout<<'\n';
                scope_member.print();
                cout<<'\n';
                flag=true;
                break;
            }
        }
        if (!flag) {
            cout << "\n[Not Found] Symbol '" << identName << "' not found in symbol table\n";
        }
    }
};

// ============================= Error Detection ============================

enum class ErrorType {
    DuplicateVariableInScope,
    MethodCallSignatureMismatch,
    ReturnTypeMismatch,
    CyclicInheritance,
    InvalidVariableAccess
};

string tostring(ErrorType type){
    switch (type) {
        case ErrorType::DuplicateVariableInScope: return "Duplicate Variable In Scope";
        case ErrorType::MethodCallSignatureMismatch: return "Method Call Signature Mismatch";
        case ErrorType::ReturnTypeMismatch: return "Return Type Mismatch";
        case ErrorType::CyclicInheritance: return "Cyclic Inheritance";
        case ErrorType::InvalidVariableAccess: return "Invalid Variable Access";
        default: return "Unknown Error";
    }
}

class Error{
    protected:
        size_t error_line;
        ErrorType error_type;
    public:
        Error(size_t error_line, ErrorType error_type){
            this->error_line = error_line;
            this->error_type = error_type;
        }

        void PrintError(){
            cout<<"\nError happened in line "<< this->error_line<<'\n';
            cout<<"Error Type is {  " << tostring(this->error_type) <<"  }  \n";
        }
};

class ErrorDetection {
    protected:
        string buffer;
        size_t pos;
        SymbolTable* symbol_table;
    public:
        ErrorDetection(const string buffer ,SymbolTable* symbol_table){
            this->buffer = buffer;
            this->symbol_table = symbol_table;
            this->pos = 0;
        }

        vector<Error> Detect_Duplicate_Variable_In_Scope(){
            vector<Error> ans;
            size_t error_num=0;

            /*
                This section was added to the symbol table module because the project specification
                 states that duplicate variables must not be printed in the symbol table. 
                In this part, the results of the checks performed within that module are used.
            */
            for(auto err : InProcessError){
                ans.push_back(Error(err.first,ErrorType::DuplicateVariableInScope));
                error_num++;
            }

            return ans;
        }

        vector<Error> Detect_Method_Call_Signature_Mismatch(){
            vector<Error> ans;
            size_t error_num=0;
            

            return ans;
        }

        vector<Error> Detect_Return_Type_Mismatch(){
            vector<Error> ans;
            size_t error_num=0;
            

            return ans;
        }

        vector<Error> Detect_Cyclic_Inheritance(){
            vector<Error> ans;
            size_t error_num=0;
            

            return ans;
        }

        vector<Error> Detect_Invalid_Variable_Access(){
            vector<Error> ans;
            size_t error_num=0;
            

            return ans;
        }

        void PrintDetectedErrors(ErrorType type=ErrorType::DuplicateVariableInScope){

            size_t error_num=0;

            vector<Error> Duplicate_Variable_Error = this->Detect_Duplicate_Variable_In_Scope();

            vector<Error> Method_Call_Signature_Mismatch = this->Detect_Method_Call_Signature_Mismatch();

            vector<Error> Return_Type_Mismatch = this->Detect_Return_Type_Mismatch();

            vector<Error> Cyclic_Inheritance = this->Detect_Cyclic_Inheritance();

            vector<Error> Invalid_Variable_Access = this->Detect_Invalid_Variable_Access();

            cout<<"\n==================== Detected Errors ====================\n";

            for(auto err : Duplicate_Variable_Error){
                err.PrintError();
                error_num++;
            }
            for(auto err : Method_Call_Signature_Mismatch){
                err.PrintError();
                error_num++;
            }
            for(auto err : Return_Type_Mismatch){
                err.PrintError();
                error_num++;
            }
            for(auto err : Cyclic_Inheritance){
                err.PrintError();
                error_num++;
            }
            for(auto err : Invalid_Variable_Access){
                err.PrintError();
                error_num++;
            }

            cout<<"\n=========================================================\n";

            cout<<"\nError count : "<<error_num<<" \n";

        }
};

// ============================= Main ======================================

int main() {

    cout << "\n";
    cout << "========================================================================\n";
    cout << "    Java-- Compiler - Error Detection\n";
    cout << "========================================================================\n\n";

    SymbolTable symbolTable;
    
    cout << "Loading symbol table from symbols.json...\n";
    if (!symbolTable.loadFromJSON("symbols.json")) {
        cerr << "[Error] Failed to load symbols.json\n";
        cerr << "Creating empty symbol table for demonstration...\n";
    }
    
    symbolTable.printSymbolsTable();

    ifstream file("input.txt");  
    if (!file.is_open()) {       
        cerr << "Error: cannot open file." << endl;
        return 1;
    }
    stringstream buffer_input;         
    buffer_input << file.rdbuf();      
    string testCode = buffer_input.str();

    symbolTable.dump();

    ErrorDetection ErrorDetector(testCode, &symbolTable);
    ErrorDetector.PrintDetectedErrors();
    
    cout << "\nThank you for using Java-- Compiler!\n";
    return 0;
}