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
#include <stack>

using namespace std;

typedef long long int ll;

// ============================= JSON Parser ======================================
class SimpleJSON {
    public:
        bool parseAntlrData(const string content_recive, vector<unordered_map<string, vector<unordered_map<string, string>>>>& datas, const string& filename="") {
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

            // Initialize the result vector with 4 empty maps
            datas.clear();
            datas.resize(4);

            // Map category names to indices
            unordered_map<string, int> categoryIndex = {
                {"1_variable_declarations", 0},
                {"2_method_calls", 1},
                {"3_method_declarations", 2},
                {"4_variable_usages", 3}
            };

            size_t pos = 0;

            // Find the root object
            size_t rootStart = content.find('{');
            if (rootStart == string::npos) {
                cerr << "[Error] No JSON object found\n";
                return false;
            }

            // Parse the root object
            size_t rootEnd = findMatchingBrace(content, rootStart);
            if (rootEnd == string::npos) {
                cerr << "[Error] Malformed JSON: unmatched braces\n";
                return false;
            }

            string rootStr = content.substr(rootStart + 1, rootEnd - rootStart - 1);
            pos = 0;

            while (pos < rootStr.size()) {
                // Find key (category name)
                while (pos < rootStr.size() && isspace(rootStr[pos])) pos++;
                if (pos >= rootStr.size()) break;

                size_t keyStart = rootStr.find('"', pos);
                if (keyStart == string::npos) break;

                size_t keyEnd = rootStr.find('"', keyStart + 1);
                if (keyEnd == string::npos) break;

                string category = rootStr.substr(keyStart + 1, keyEnd - keyStart - 1);

                // Find colon
                size_t colonPos = rootStr.find(':', keyEnd);
                if (colonPos == string::npos) break;

                // Find value (array)
                size_t valueStart = colonPos + 1;
                while (valueStart < rootStr.size() && isspace(rootStr[valueStart])) valueStart++;
                if (valueStart >= rootStr.size() || rootStr[valueStart] != '[') {
                    cerr << "[Error] Expected array for category: " << category << "\n";
                    break;
                }

                size_t arrayEnd = findMatchingBracket(rootStr, valueStart);
                if (arrayEnd == string::npos) break;

                string arrayStr = rootStr.substr(valueStart + 1, arrayEnd - valueStart - 1);

                // Parse the array of objects
                vector<unordered_map<string, string>> arrayData;
                if (!parseArrayWithSpecialHandling(arrayStr, arrayData)) {
                    cerr << "[Warning] Failed to parse array for category: " << category << "\n";
                }

                // Store in the appropriate category
                if (categoryIndex.count(category)) {
                    int idx = categoryIndex[category];
                    datas[idx][category] = arrayData;
                    cout << "[Info] Parsed " << arrayData.size() << " items for category: " << category << "\n";
                } else {
                    cerr << "[Warning] Unknown category: " << category << "\n";
                }

                pos = arrayEnd + 1;

                // Find next comma or end
                size_t commaPos = rootStr.find(',', pos);
                if (commaPos == string::npos) break;
                pos = commaPos + 1;
            }

            return true;
        }

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
        static bool parseArrayWithSpecialHandling(const string& arrayStr, vector<unordered_map<string, string>>& arrayData) {
            size_t pos = 0;

            while (pos < arrayStr.size()) {
                while (pos < arrayStr.size() && isspace(arrayStr[pos])) pos++;
                if (pos >= arrayStr.size()) break;

                if (arrayStr[pos] == '{') {
                    size_t objEnd = findMatchingBrace(arrayStr, pos);
                    if (objEnd == string::npos) break;

                    string objStr = arrayStr.substr(pos + 1, objEnd - pos - 1);
                    unordered_map<string, string> obj;

                    if (!parseObjectEnhanced(objStr, obj)) {
                        pos = objEnd + 1;
                        continue;
                    }

                    if (!obj.empty()) {
                        arrayData.push_back(obj);
                    }

                    pos = objEnd + 1;
                } else {
                    // Skip non-object elements
                    size_t nextComma = arrayStr.find(',', pos);
                    if (nextComma == string::npos) break;
                    pos = nextComma + 1;
                }

                // Find next comma
                size_t commaPos = arrayStr.find(',', pos);
                if (commaPos == string::npos) break;
                pos = commaPos + 1;
            }

            return true;
        }

        static bool parseObjectEnhanced(const string& objStr, unordered_map<string, string>& obj) {
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
                    // Array value - parse array elements with special handling
                    size_t arrayEnd = findMatchingBracket(objStr, valueStart);
                    if (arrayEnd == string::npos) break;

                    // Extract array content
                    string arrayContent = objStr.substr(valueStart + 1, arrayEnd - valueStart - 1);

                    // Parse array elements
                    vector<string> arrayElements;
                    size_t elemPos = 0;
                    while (elemPos < arrayContent.size()) {
                        while (elemPos < arrayContent.size() && isspace(arrayContent[elemPos])) elemPos++;
                        if (elemPos >= arrayContent.size()) break;

                        if (arrayContent[elemPos] == '"') {
                            // String element
                            size_t elemEnd = arrayContent.find('"', elemPos + 1);
                            if (elemEnd == string::npos) break;
                            string element = arrayContent.substr(elemPos + 1, elemEnd - elemPos - 1);
                            arrayElements.push_back(element);
                            elemPos = elemEnd + 1;
                        } else if (arrayContent[elemPos] == '[') {
                            // Nested array - store as JSON string
                            size_t nestedArrayEnd = findMatchingBracket(arrayContent, elemPos);
                            if (nestedArrayEnd == string::npos) break;
                            string element = arrayContent.substr(elemPos, nestedArrayEnd - elemPos + 1);
                            arrayElements.push_back(element);
                            elemPos = nestedArrayEnd + 1;
                        } else {
                            // Non-string element (number, boolean, null)
                            size_t elemEnd = arrayContent.find_first_of(",]", elemPos);
                            if (elemEnd == string::npos) elemEnd = arrayContent.size();
                            string element = arrayContent.substr(elemPos, elemEnd - elemPos);
                            // Trim whitespace
                            element.erase(0, element.find_first_not_of(" \t\n\r"));
                            element.erase(element.find_last_not_of(" \t\n\r") + 1);
                            if (!element.empty()) {
                                arrayElements.push_back(element);
                            }
                            elemPos = elemEnd;
                        }

                        // Skip comma
                        size_t commaPos = arrayContent.find(',', elemPos);
                        if (commaPos == string::npos) break;
                        elemPos = commaPos + 1;
                    }

                    // Store array as a pipe-separated string
                    ostringstream oss;
                    for (size_t i = 0; i < arrayElements.size(); i++) {
                        if (i > 0) oss << "|";
                        oss << arrayElements[i];
                    }
                    value = oss.str();
                    pos = arrayEnd + 1;
                } else if (objStr[valueStart] == '{') {
                    // Object value - store as JSON string
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

        static size_t findMatchingBrace(const string& str, size_t start) {
            int count = 1;
            for (size_t i = start + 1; i < str.size(); i++) {
                if (str[i] == '{') count++;
                else if (str[i] == '}') count--;

                if (count == 0) return i;
            }
            return string::npos;
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
            string symbol_line = sym["line"];
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
                classInfo.line = stoi(symbol_line);
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
                intInfo.line = stoi(symbol_line);
                intInfo.IndexInSymbolTable = to_string(i);

                Symbol s(intInfo, kind);
                success = insertSymbol(s);
            }
            else if (kind == IdentifierKind::Method) {
                MethodInfo methodInfo;
                methodInfo.name = name;
                methodInfo.id_kind = kind;
                methodInfo.scope = scopeName;
                methodInfo.line = stoi(symbol_line);
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
                ctorInfo.line = stoi(symbol_line);
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
                varInfo.line = stoi(symbol_line);
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
                fieldInfo.line = stoi(symbol_line);
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
                paramInfo.line = stoi(symbol_line);
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

    Symbol lookup(const string& identName,const string& scope) const {
        auto it = move_to_scope(scope);

        if (!it) {
            cout << "\n[Error] Scope '" << scope << "' not found\n";
            return Symbol();
        }

        bool flag=false;
        auto table_scope = it->getSymbols();
        for(auto& [scope_name,scope_member] : table_scope){
            if(scope_name==identName){
                // cout<<'\n';
                // scope_member.print();
                // cout<<'\n';
                return scope_member;
                flag=true;
                break;
            }
        }
        if (!flag) {
            cout << "\n[Not Found] Symbol '" << identName << "' not found in symbol table\n";
        }

        return Symbol();
    }
};

//

class AntlrParseData{
    public:
        vector<unordered_map<string, string>> variableDeclarations;  // variable_declarations (variable_name, declared_scope, declared_line)
        vector<unordered_map<string, string>> methodCalls;  // method_usage (method_name, declared_scope, call_line, arg_count, arg_types_inOrder)
        vector<unordered_map<string, string>> methodDeclarations;  // method_declaration (method_name, declared_scope, declared_line, actual_return_type)
        vector<unordered_map<string, string>> variableUsages;  // variable_usages (variable_name, usage_scope, declared_scope, usage_line)

        bool loadFromJSON(string filename){
            vector<unordered_map<string, vector<unordered_map<string, string>>>> parsedData;
            SimpleJSON parser;

            if (!parser.parseAntlrData("", parsedData, filename)) {
                cerr << "[Error] Failed to parse JSON file: " << filename << "\n";
                return false;
            }

            if (parsedData.size() > 0 && parsedData[0].count("1_variable_declarations")) {
                variableDeclarations = parsedData[0]["1_variable_declarations"];
                cout << "[Info] Loaded " << variableDeclarations.size() << " variable declarations\n";
            }

            if (parsedData.size() > 1 && parsedData[1].count("2_method_calls")) {
                methodCalls = parsedData[1]["2_method_calls"];
                cout << "[Info] Loaded " << methodCalls.size() << " method calls\n";
            }

            if (parsedData.size() > 2 && parsedData[2].count("3_method_declarations")) {
                methodDeclarations = parsedData[2]["3_method_declarations"];
                cout << "[Info] Loaded " << methodDeclarations.size() << " method declarations\n";
            }

            if (parsedData.size() > 3 && parsedData[3].count("4_variable_usages")) {
                variableUsages = parsedData[3]["4_variable_usages"];
                cout << "[Info] Loaded " << variableUsages.size() << " variable usages\n";
            }

            return true;
        }

        void printVariableDeclarations() const {
            cout << "\n==================== Variable Declarations ====================\n";
            for (const auto& decl : variableDeclarations) {
                cout << "Variable: " << decl.at("variable_name")
                    << " Type: " << decl.at("type")
                    << " Scope: " << decl.at("full_scope")
                    << " Line: " << decl.at("line") << "\n";
            }
        }

        void printMethodCalls() const {
            cout << "\n==================== Method Calls ====================\n";
            for (const auto& call : methodCalls) {
                cout << "Method: " << call.at("method_name")
                    << " Called at: " << call.at("call_location")
                    << " Line: " << call.at("line")
                    << " Args: " << call.at("argument_count") << "\n";
            }
        }

        void printMethodDeclarations() const {
            cout << "\n==================== Method Declarations ====================\n";
            for (const auto& decl : methodDeclarations) {
                cout << "Method: " << decl.at("method_name")
                    << " Scope: " << decl.at("full_scope")
                    << " Return: " << decl.at("declared_return_type")
                    << " Line: " << decl.at("line") << "\n";
            }
        }

        void printVariableUsages() const {
            cout << "\n==================== Variable Usages ====================\n";
            for (const auto& usage : variableUsages) {
                cout << "Variable: " << usage.at("variable_name")
                    << " Used at: " << usage.at("usage_location")
                    << " Line: " << usage.at("line")
                    << " Declared at: " << usage.at("declared_at") << "\n";
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

        size_t get_line() const {
            return this->error_line;
        }

        void PrintError(){
            cout<<"\n[Error] Error happened in line "<< this->error_line<<'\n';
            cout<<"Error Type is {  " << tostring(this->error_type) <<"  }  \n";
        }
};

// Hash function for pair<string,string> to use in our unordered_map
struct PairHash {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const {
        auto h1 = hash<T1>{}(p.first);
        auto h2 = hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

class ErrorDetection {
    protected:
        string buffer;
        size_t pos;
        SymbolTable* symbol_table;
        AntlrParseData SementicData;

    public:
        ErrorDetection(const string buffer ,SymbolTable* symbol_table, AntlrParseData sementicData = AntlrParseData()){
            this->buffer = buffer;
            this->symbol_table = symbol_table;
            this->SementicData = sementicData;
            this->pos = 0;
        }

        vector<Error> Detect_Duplicate_Variable_In_Scope(){
            vector<Error> ans;
            size_t error_num=0;

            // solution 1 :
            /*
                This section was added to the symbol table module because the project specification
                 states that duplicate variables must not be printed in the symbol table.
                In this part, the results of the checks performed within that module are used.
            */
            // for(auto err : InProcessError){
            //     ans.push_back(Error(err.first,ErrorType::DuplicateVariableInScope));
            //     error_num++;
            // }

            // solution 2 :

            unordered_map<pair<string,string>, size_t, PairHash> varCount; // (varName, scope) -> counting the number of declarations
            unordered_map<pair<string,string>, size_t, PairHash> varLine;  // (varName, scope) -> first declaration line

            for (const auto& decl : SementicData.variableDeclarations) {
                string varName = decl.at("variable_name");
                string scope = decl.at("full_scope");
                size_t line = stoi(decl.at("line"));

                pair<string,string> key = make_pair(varName, scope);

                if (varCount.find(key) == varCount.end()) {
                    varCount[key] = 0;
                    varLine[key] = line;
                }

                varCount[key]++;

                if (varCount[key]>1) {
                    ans.push_back(Error(line, ErrorType::DuplicateVariableInScope));
                    error_num++;
                }
            }

            return ans;
        }

        static vector<string> parsePipeSeparated(const string& str) {
            vector<string> result;
            if (str.empty()) return result;

            stringstream ss(str);
            string token;
            while (getline(ss, token, '|')) {
                if (!token.empty()) {
                    result.push_back(token);
                }
            }
            return result;
        }

        vector<Error> Detect_Method_Call_Signature_Mismatch(){
            vector<Error> ans;
            size_t error_num=0;

            for (const auto& call : SementicData.methodCalls) {
                string methodName = call.at("method_name");
                string declaredScope = call.at("declared_at");
                size_t callLine = stoi(call.at("line"));
                size_t argCount = stoi(call.at("argument_count"));
                vector<string> argTypes;
                if(argCount!=0){
                    argTypes = parsePipeSeparated(call.at("argument_types"));
                }

                bool foundDeclaration = false;

                // Method declaration not found
                if(declaredScope=="None"){
                    ans.push_back(Error(callLine, ErrorType::MethodCallSignatureMismatch));
                    error_num++;
                    continue;
                }
                string classScope = declaredScope;
                size_t lastColon = classScope.rfind("::");
                if (lastColon != string::npos) {
                    classScope = classScope.substr(0, lastColon);  // Remove "::methodName"
                }
                if (classScope.find("GLOBAL::") == 0) {
                    classScope = classScope.substr(8); // Remove "GLOBAL::"
                }
                // if(classScope=="GLOBAL"){
                //     classScope="";
                // }
                declaredScope = classScope;


                Symbol methodInSymbolTable = this->symbol_table->lookup(methodName,declaredScope);

                if (methodInSymbolTable.getName() != "") {
                    if (methodInSymbolTable.kind == IdentifierKind::Method) {
                        auto methodData = dynamic_pointer_cast<MethodInfo>(methodInSymbolTable.data);
                        if (methodData) {
                            size_t declaredParamCount = methodData->parameters.size();

                            if (declaredParamCount != static_cast<size_t>(argCount)) {
                                ans.push_back(Error(callLine, ErrorType::MethodCallSignatureMismatch));
                                error_num++;
                            } else {
                                if(argCount==0){
                                    foundDeclaration = true;
                                    continue;
                                }
                                vector<ParameterInfo> declaredParams = methodData->parameters;
                                for (size_t i = 0; i < declaredParamCount; i++) {
                                    if (declaredParams[i].type.name != argTypes[i]) {
                                        ans.push_back(Error(callLine, ErrorType::MethodCallSignatureMismatch));
                                        error_num++;
                                        break;
                                    }
                                }
                            }
                            foundDeclaration = true;
                        }
                    }
                }

            }

            return ans;
        }

        vector<Error> Detect_Return_Type_Mismatch() {
            vector<Error> ans;
            size_t error_num = 0;

            for (const auto& method : SementicData.methodDeclarations) {

                if (!method.count("method_name") || !method.count("full_scope") || !method.count("line"))
                    continue;

                string methodName = method.at("method_name");
                int startLine = stoi(method.at("line"));
                int returnLine = stoi(method.at("return_line"));
                string actualReturnTypes;
                if(method.count("actual_return_types")!=0){
                    actualReturnTypes = method.at("actual_return_types"); 
                }else{
                    actualReturnTypes = "void";
                }

                string fullScope = method.at("full_scope");
                if (fullScope.rfind("GLOBAL::", 0) == 0)
                    fullScope = fullScope.substr(8);


                size_t pos = fullScope.rfind("::");
                if (pos == string::npos) continue;

                string classScope = fullScope.substr(0, pos);

                // solution 1
                Symbol methodSym = symbol_table->lookup(methodName, classScope);

                if (methodSym.getName() != "") {
                    if (methodSym.kind == IdentifierKind::Method) {
                        auto methodData = dynamic_pointer_cast<MethodInfo>(methodSym.data);
                        if (methodData) {
                            string declaredReturnType = methodData->returnType.name;
                            
                            if (declaredReturnType != actualReturnTypes) {
                                int selected_line;
                                if(returnLine!=-1){
                                    selected_line = returnLine;
                                }else{
                                    selected_line = startLine;
                                }
                                ans.push_back(Error(selected_line, ErrorType::ReturnTypeMismatch));
                                error_num++;
                            }
                        }
                    }
                }


                // solution 2
                // string declaredReturnType = method.at("declared_return_type");
                // if (declaredReturnType != actualReturnTypes) {
                //     int selected_line;
                //     if(returnLine!=-1){
                //         selected_line = returnLine;
                //     }else{
                //         selected_line = startLine;
                //     }
                //     ans.push_back(Error(selected_line, ErrorType::ReturnTypeMismatch));
                //     error_num++;
                // }
            }

            //solution 3
            // struct MethodCtx {
            //     string methodName;
            //     string classScope;
            //     string methodScope;
            //     string returnType;
            //     size_t startLine;
            //     bool valid;
            // };

            // unordered_map<size_t, MethodCtx> methods;

            // for (const auto& m : SementicData.methodDeclarations) {

            //     if (!m.count("method_name") ||
            //         !m.count("full_scope") ||
            //         !m.count("line"))
            //         continue;

            //     MethodCtx ctx;
            //     ctx.methodName = m.at("method_name");
            //     ctx.startLine = stoll(m.at("line"));
            //     ctx.valid = false;

            //     string fullScope = m.at("full_scope");
            //     if (fullScope.rfind("GLOBAL::", 0) == 0)
            //         fullScope = fullScope.substr(8);

            //     ctx.methodScope = fullScope;

            //     size_t pos = fullScope.rfind("::");
            //     if (pos == string::npos) continue;

            //     ctx.classScope = fullScope.substr(0, pos);

            //     Symbol methodSym =
            //         symbol_table->lookup(ctx.methodName, ctx.classScope);

            //     if (methodSym.getName() == "" ||
            //         methodSym.kind != IdentifierKind::Method)
            //         continue;

            //     auto mi = dynamic_pointer_cast<MethodInfo>(methodSym.data);
            //     if (!mi) continue;

            //     ctx.returnType = mi->returnType.name;
            //     ctx.valid = true;

            //     methods[ctx.startLine] = ctx;
            // }

            // stringstream ss(buffer);
            // string line;
            // size_t lineNo = 0;

            // bool inMethod = false;
            // int braceDepth = 0;
            // MethodCtx cur;

            // while (getline(ss, line)) {
            //     lineNo++;

            //     if (!inMethod) {
            //         auto it = methods.find(lineNo);
            //         if (it != methods.end() && it->second.valid) {
            //             cur = it->second;
            //             inMethod = true;
            //             braceDepth = 0;
            //         }
            //     }

            //     if (!inMethod) continue;

            //     braceDepth += count(line.begin(), line.end(), '{');
            //     braceDepth -= count(line.begin(), line.end(), '}');

            //     size_t p = line.find("return");
            //     string retExpr = "";

            //     if (p != string::npos) {
            //         if (!(p > 0 && isalnum(line[p - 1]))) {
            //             p += 6;
            //             while (p < line.size() && isspace(line[p])) p++;
            //             if (p < line.size()) {
            //                 size_t end = line.find(';', p);
            //                 if (end != string::npos) {
            //                     retExpr = line.substr(p, end - p);
            //                     retExpr.erase(0, retExpr.find_first_not_of(" \t"));
            //                     retExpr.erase(retExpr.find_last_not_of(" \t;") + 1);
            //                 }
            //             }
            //         }
            //     }

            //     if (!retExpr.empty() || line.find("return") != string::npos) {

            //         if (retExpr.empty()) {
            //             if (cur.returnType != "void") {
            //                 ans.push_back(Error(lineNo, ErrorType::ReturnTypeMismatch));
            //                 error_num++;
            //             }
            //         }
            //         else {
            //             if (cur.returnType == "void") {
            //                 ans.push_back(Error(lineNo, ErrorType::ReturnTypeMismatch));
            //                 error_num++;
            //             }
            //             else {
            //                 string litType = "";

            //                 if (retExpr == "true" || retExpr == "false")
            //                     litType = "boolean";
            //                 else {
            //                     bool isInt = !retExpr.empty();
            //                     for (char c : retExpr) {
            //                         if (!isdigit(c)) { isInt = false; break; }
            //                     }
            //                     if (isInt) litType = "int";
            //                     else if (retExpr.size() >= 2 &&
            //                              retExpr.front() == '"' &&
            //                              retExpr.back() == '"')
            //                         litType = "String";
            //                 }

            //                 if (!litType.empty()) {
            //                     if (litType != cur.returnType) {
            //                         ans.push_back(Error(lineNo, ErrorType::ReturnTypeMismatch));
            //                         error_num++;
            //                     }
            //                 }
            //                 else {
            //                     Symbol retSym =
            //                         symbol_table->lookup(retExpr, cur.methodScope);

            //                     if (retSym.getName() != "") {
            //                         string actualType = "";

            //                         if (retSym.kind == IdentifierKind::Variable) {
            //                             auto v = dynamic_pointer_cast<VariableInfo>(retSym.data);
            //                             if (v) actualType = v->type.name;
            //                         }
            //                         else if (retSym.kind == IdentifierKind::Parameter) {
            //                             auto p = dynamic_pointer_cast<ParameterInfo>(retSym.data);
            //                             if (p) actualType = p->type.name;
            //                         }

            //                         if (!actualType.empty() &&
            //                             actualType != cur.returnType) {
            //                             ans.push_back(Error(lineNo, ErrorType::ReturnTypeMismatch));
            //                             error_num++;
            //                         }
            //                     }
            //                 }
            //             }
            //         }
            //     }

            //     if (braceDepth <= 0) {
            //         inMethod = false;
            //     }
            // }

            return ans;
        }

        vector<Error> Detect_Cyclic_Inheritance(){
            vector<Error> ans;
            size_t error_num=0;

            unordered_set<string> reported;

            Scope* globalScope = symbol_table->getGlobalScope();

            vector<Scope*> stack;
            stack.push_back(globalScope);

            while (!stack.empty()) {
                Scope* currentScope = stack.back();
                stack.pop_back();

                for (const auto& [name, sym] : currentScope->getSymbols()) {

                    if (sym.kind != IdentifierKind::Class)
                        continue;

                    ClassInfo* classInfo = dynamic_cast<ClassInfo*>(sym.data.get());
                    if (!classInfo)
                        continue;

                    unordered_set<string> visited;
                    string currentClass = classInfo->name;

                    while (true) {
                        if (visited.count(currentClass)) {
                            if (!reported.count(currentClass)) {
                                ans.push_back(
                                    Error(classInfo->line,
                                          ErrorType::CyclicInheritance)
                                );
                                error_num++;
                                reported.insert(currentClass);
                            }
                            break;
                        }

                        visited.insert(currentClass);

                        Symbol* parentSym =
                            symbol_table->getGlobalScope()
                                ->lookup(classInfo->parentClass);

                        if (!parentSym ||
                            parentSym->kind != IdentifierKind::Class)
                            break;

                        ClassInfo* parentInfo = dynamic_cast<ClassInfo*>(parentSym->data.get());
                        if (!parentInfo)
                            break;

                        if (parentInfo->parentClass.empty() ||
                            parentInfo->parentClass == "null")
                            break;

                        currentClass = parentInfo->name;
                        classInfo = parentInfo;
                    }
                }

                for (Scope* child : currentScope->getAllChildren()) {
                    stack.push_back(child);
                }
            }

            return ans;
        }

        bool isparent(const string& declaredScope, const string& usageScope) {
            string parent = normalizeScope(declaredScope);
            string child = normalizeScope(usageScope);

            // parent is prefix of child
            if (child.find(parent) == 0) {
                if (child.length() == parent.length()) {
                    return true; // Same scope
                }
                // Check if next character is "::"
                if (child[parent.length()] == ':' && child[parent.length() + 1] == ':') {
                    return true;
                }
            }
            return false;
        }
        string normalizeScope(const string& scope) {
            string result = scope;
            // Remove GLOBAL::
            if (result.find("GLOBAL::") == 0) {
                result = result.substr(8);
            }
            return result;
        }
        vector<Error> Detect_Invalid_Variable_Access(){
            vector<Error> ans;
            size_t error_num=0;

            for (const auto& usage : SementicData.variableUsages) {
                string varName = usage.at("variable_name");
                string usageLocation = usage.at("usage_location");
                size_t usageLine = stoi(usage.at("line"));
                size_t declaredLine = stoi(usage.at("declared_line"));
                string declaredAt = usage.at("declared_at");

                if (declaredAt == "None" || declaredAt.empty() || (declaredLine==-1 || declaredLine>usageLine)) {
                    ans.emplace_back(usageLine, ErrorType::InvalidVariableAccess);
                    error_num++;
                    continue;
                }
                // solution 1
                if (!isparent(declaredAt,usageLocation)) {
                    ans.push_back(Error(usageLine, ErrorType::InvalidVariableAccess));
                    error_num++;
                }

                /*
                if (usageLocation.find(declaredAt) == 0) {


                    if (usageLocation == declaredAt) {
                        continue;
                    }

                    string expectedPrefix = declaredAt + "::";
                    if (usageLocation.find(expectedPrefix) == 0) {
                        continue;
                    }
                }

                bool found = false;

                Scope* usageScope = symbol_table->move_to_scope(usageLocation);

                if (usageScope) {
                    Symbol* sym = usageScope->lookup(varName);
                    if (sym && !sym->getName().empty()) {
                        found = true;
                    }
                }

                if (!found) {
                    Symbol sym = symbol_table->lookup(varName, declaredAt);
                    if (!sym.getName().empty()) {
                        found = false;
                    }
                }

                if (!found) {
                    ans.emplace_back(usageLine, ErrorType::InvalidVariableAccess);
                    error_num++;
                }
                */

        }

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

            vector<Error> all_errors;

            for(auto err : Duplicate_Variable_Error){
                // err.PrintError();
                all_errors.push_back(err);
                error_num++;
            }
            for(auto err : Method_Call_Signature_Mismatch){
                // err.PrintError();
                all_errors.push_back(err);
                error_num++;
            }
            for(auto err : Return_Type_Mismatch){
                // err.PrintError();
                all_errors.push_back(err);
                error_num++;
            }
            for(auto err : Cyclic_Inheritance){
                // err.PrintError();
                all_errors.push_back(err);
                error_num++;
            }
            for(auto err : Invalid_Variable_Access){
                // err.PrintError();
                all_errors.push_back(err);
                error_num++;
            }

            // Sort errors by line number
            sort(all_errors.begin(), all_errors.end(), [](const Error& a, const Error& b) { return a.get_line()<b.get_line(); });
            for(auto err : all_errors){
                err.PrintError();
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

    AntlrParseData antlrDatas;
    if(!antlrDatas.loadFromJSON("semantic_info.json")){
        cerr << "[Error] Failed to load semantic_info.json\n";
    }

    ErrorDetection ErrorDetector(testCode, &symbolTable, antlrDatas);
    ErrorDetector.PrintDetectedErrors();

    cout << "\nThank you for using Java-- Compiler!\n";
    return 0;
}
