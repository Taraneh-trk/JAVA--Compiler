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

// ============================= part 1 from phase 1 - Token System ======================================

//We use token system here for reading the source code and find needed parts

enum class TokenType {
    Unknown,
    Identifier,
    Operator,
    Keyword,
    Delimiter,
    Number,
    StringLiteral,
    CharLiteral,
    BooleanLiteral,
    NullLiteral
};

template <typename TYPE1>
struct Token{
    protected:
        pair<TokenType, TYPE1> token;
        int line;    
        int column;
        size_t pos;
    public:
        Token() : token(), line(0), column(0), pos(0) {}
        Token(TokenType token_type, TYPE1 lexeme, int l = 0, int c = 0, size_t p = 0) {
            this->token.first = token_type;
            this->token.second = lexeme;
            this->line = l;
            this->column = c;
            this->pos = p;
        }
        TokenType getType() const {
            return token.first;
        }
        TYPE1 getLexeme() const {
            return token.second;
        }
        int getLine() const { 
            return line; 
        }
        int getColumn() const { 
            return column; 
        }
        
        friend ostream& operator<<(ostream& os, const Token& t) {
            os << "(" << Token::typeToString(t.token.first) << ", " << t.token.second << ")";
            return os;
        }
        bool isValid() const {
            return this->token.first != TokenType::Unknown;
        }
        static string typeToString(TokenType t) {
            switch (t) {
                case TokenType::Identifier: return "Identifier";
                case TokenType::Number: return "Number";
                case TokenType::Operator: return "Operator";
                case TokenType::Keyword: return "Keyword";
                case TokenType::Delimiter: return "Delimiter";
                case TokenType::StringLiteral: return "StringLiteral";
                case TokenType::CharLiteral: return "CharLiteral";
                case TokenType::BooleanLiteral: return "BooleanLiteral";
                case TokenType::NullLiteral: return "NullLiteral";
                case TokenType::Unknown: return "Unknown";
                default: return "Invalid";
            }
        }
};

class SymbolTable;

template <typename TYPE1>
class BaseTokenRecognizer {
    public:
        virtual ~BaseTokenRecognizer() = default;
        virtual bool match(const string& buffer, size_t& pos, Token<TYPE1>& constructed_token, SymbolTable* symTable = nullptr, int line = 1, int col = 1) = 0;
};

template <typename TYPE1 = string>
class IdentifierRecognizer : public BaseTokenRecognizer<TYPE1> {
    public:
        bool match(const string& buffer, size_t& pos, Token<TYPE1>& constructed_token, SymbolTable* symTable = nullptr, int line = 1, int col = 1) override {
            size_t start = pos;
            string lexeme;
            
            if (pos >= buffer.size() || !(isalpha(static_cast<unsigned char>(buffer[pos])) || buffer[pos] == '_' || buffer[pos] == '$'))
                return false;
                
            lexeme += buffer[pos++];
            while (pos < buffer.size() && (isalnum(static_cast<unsigned char>(buffer[pos])) || buffer[pos] == '_' || buffer[pos] == '$')) {
                lexeme += buffer[pos++];
            }
            
            constructed_token = Token<TYPE1>(TokenType::Identifier, static_cast<TYPE1>(lexeme), line, col, start);
            return true;
        }
};

template <typename TYPE1 = string>
class OperatorRecognizer : public BaseTokenRecognizer<TYPE1> {
    private:
        const unordered_set<string> operators = {
            "+", "-", "/", "*", "%", "**",
            "==", "!=", ">=", "<=", ">", "<",
            "&&", "||", "!",
            "=",
            ".length"
        };
        size_t maxLen;
    public:
        OperatorRecognizer() {
            maxLen = 0;
            for (const auto& op : operators) {
                if (op.size() > maxLen) 
                    maxLen = op.size();
            }
        }
        bool match(const string& buffer, size_t& pos, Token<TYPE1>& constructed_token, SymbolTable* symTable = nullptr, int line = 1, int col = 1) override {
            size_t start = pos;
            if (pos >= buffer.size()) 
                return false;
                
            size_t lenToCheck = min(maxLen, buffer.size() - pos);
            for (int len = static_cast<int>(lenToCheck); len >= 1; --len) {
                string sub = buffer.substr(pos, static_cast<size_t>(len));
                if (operators.count(sub)) {
                    pos += static_cast<size_t>(len);
                    constructed_token = Token<TYPE1>(TokenType::Operator, static_cast<TYPE1>(sub), line, col, start);
                    return true;
                }
            }
            return false;
        }
};

template <typename TYPE1 = string>
class KeywordRecognizer : public BaseTokenRecognizer<TYPE1> {
    private:
        const unordered_set<string> keywords = {
            "class", "interface", "extends", "implements",
            "public", "private", "protected", "internal",
            "static", "void", "abstract", "if", "else",
            "while", "for", "break", "continue",
            "return", "new", "this", "import",
            "print", "read",
            "true", "false", "null"
        };
        const unordered_set<string> types = {
            "boolean", "int", "char", "String"
        };
    public:
        bool match(const string& buffer, size_t& pos, Token<TYPE1>& constructed_token, SymbolTable* symTable = nullptr, int line = 1, int col = 1) override {
            size_t start = pos;
            if (pos >= buffer.size() || !(isalpha(static_cast<unsigned char>(buffer[pos])) || buffer[pos] == '_'))
                return false;
                
            string lexeme;
            lexeme += buffer[pos++];
            while (pos < buffer.size() && (isalnum(static_cast<unsigned char>(buffer[pos])) || buffer[pos] == '_')) {
                lexeme += buffer[pos++];
            }
            
            if (keywords.count(lexeme) || types.count(lexeme)) {
                constructed_token = Token<TYPE1>(TokenType::Keyword, static_cast<TYPE1>(lexeme), line, col, start);
                return true;
            }
            
            pos = start;
            return false;
        }
};

template <typename TYPE1 = string>
class DelimiterRecognizer : public BaseTokenRecognizer<TYPE1> {
    private:
        const unordered_set<string> delimiters = {
            ";", ",", "(", ")", "{", "}", "[", "]", ".","@"
        };
    public:
        bool match(const string& buffer, size_t& pos, Token<TYPE1>& constructed_token, SymbolTable* symTable = nullptr, int line = 1, int col = 1) override {
            if (pos >= buffer.size()) 
                return false;
                
            string lexeme(1, buffer[pos]);
            if (delimiters.count(lexeme)) {
                constructed_token = Token<TYPE1>(TokenType::Delimiter, static_cast<TYPE1>(lexeme), line, col, pos);
                pos++;
                return true;
            }
            return false;
        }
};

template <typename TYPE1 = string>
class NumberRecognizer : public BaseTokenRecognizer<TYPE1> {
    public:
        bool match(const string& buffer, size_t& pos, Token<TYPE1>& constructed_token, SymbolTable* symTable = nullptr, int line = 1, int col = 1) override {
            size_t start = pos;
            if (pos >= buffer.size() || !isdigit(static_cast<unsigned char>(buffer[pos]))) 
                return false;
                
            string lexeme;
            bool hasUnderscore = false;
            
            while (pos < buffer.size()) {
                char c = buffer[pos];
                if (isdigit(static_cast<unsigned char>(c))) {
                    lexeme += c;
                    pos++;
                } else if (c == '_') {
                    if (lexeme.empty() || pos + 1 >= buffer.size() || !isdigit(static_cast<unsigned char>(buffer[pos + 1]))) {
                        pos = start;
                        return false;
                    }
                    hasUnderscore = true;
                    lexeme += c;
                    pos++;
                } else if (c == 'l' || c == 'L') {
                    lexeme += c;
                    pos++;
                    break;
                } else {
                    break;
                }
            }
            
            if (pos < buffer.size() && isalpha(static_cast<unsigned char>(buffer[pos]))) {
                pos = start;
                return false;
            }
            
            if (!lexeme.empty()) {
                constructed_token = Token<TYPE1>(TokenType::Number, static_cast<TYPE1>(lexeme), line, col, start);
                return true;
            }
            
            pos = start;
            return false;
        }
};

template <typename TYPE1 = string>
class LiteralRecognizer : public BaseTokenRecognizer<TYPE1> {
    private:
        bool startsWith(const string& buffer, size_t pos, const string& prefix) const {
            if (pos + prefix.size() > buffer.size()) 
                return false;
            if (buffer.compare(pos, prefix.size(), prefix) != 0) 
                return false;
                
            size_t next = pos + prefix.size();
            if (next < buffer.size()) {
                char c = buffer[next];
                if (isalnum(static_cast<unsigned char>(c)) || c == '_') 
                    return false;
            }
            return true;
        }
    public:
        bool match(const string& buffer, size_t& pos, Token<TYPE1>& constructed_token, SymbolTable* symTable = nullptr, int line = 1, int col = 1) override {
            size_t start = pos;
            if (pos >= buffer.size()) 
                return false;
                
            char c = buffer[pos];
            
            if (c == '"') {
                pos++;
                string content;
                bool escape = false;
                
                while (pos < buffer.size()) {
                    char ch = buffer[pos];
                    
                    if (escape) {
                        content += ch;
                        escape = false;
                        pos++;
                    } else if (ch == '\\') {
                        escape = true;
                        content += ch;
                        pos++;
                    } else if (ch == '"') {
                        pos++;
                        constructed_token = Token<TYPE1>(TokenType::StringLiteral, static_cast<TYPE1>("\"" + content + "\""), line, col, start);
                        return true;
                    } else {
                        content += ch;
                        pos++;
                    }
                }
                
                pos = start;
                return false;
            } 
            else if (c == '\'') {
                pos++;
                if (pos >= buffer.size()) { 
                    pos = start; 
                    return false; 
                }
                    
                string content;
                bool escape = false;
                
                if (buffer[pos] == '\\') {
                    escape = true;
                    content += buffer[pos];
                    pos++;
                    if (pos >= buffer.size()) { 
                        pos = start; 
                        return false; 
                    }
                    content += buffer[pos];
                    pos++;
                } else {
                    char ch = buffer[pos];
                    if (ch == '\'' || ch == '\\') {
                        pos = start;
                        return false;
                    }
                    content += ch;
                    pos++;
                }
                
                if (pos < buffer.size() && buffer[pos] == '\'') {
                    pos++;
                    constructed_token = Token<TYPE1>(TokenType::CharLiteral, static_cast<TYPE1>("'" + content + "'"), line, col, start);
                    return true;
                }
                
                pos = start;
                return false;
            } 
            else if (startsWith(buffer, pos, "true")) {
                pos += 4;
                constructed_token = Token<TYPE1>(TokenType::BooleanLiteral, static_cast<TYPE1>("true"), line, col, start);
                return true;
            } 
            else if (startsWith(buffer, pos, "false")) {
                pos += 5;
                constructed_token = Token<TYPE1>(TokenType::BooleanLiteral, static_cast<TYPE1>("false"), line, col, start);
                return true;
            } 
            else if (startsWith(buffer, pos, "null")) {
                pos += 4;
                constructed_token = Token<TYPE1>(TokenType::NullLiteral, static_cast<TYPE1>("null"), line, col, start);
                return true;
            }
            
            return false;
        }
};

template <typename TYPE1 = string>
class Lexer {
    private:
        vector<unique_ptr<BaseTokenRecognizer<TYPE1>>> recognizers;
        SymbolTable* symbolTable;
    public:
        Lexer(SymbolTable* symTable = nullptr) : symbolTable(symTable) {
            recognizers.emplace_back(make_unique<LiteralRecognizer<TYPE1>>());
            recognizers.emplace_back(make_unique<KeywordRecognizer<TYPE1>>());
            recognizers.emplace_back(make_unique<OperatorRecognizer<TYPE1>>());
            recognizers.emplace_back(make_unique<NumberRecognizer<TYPE1>>());
            recognizers.emplace_back(make_unique<DelimiterRecognizer<TYPE1>>());
            recognizers.emplace_back(make_unique<IdentifierRecognizer<TYPE1>>());
        }
        
        vector<Token<TYPE1>> tokenize(const string& input) {
            vector<Token<TYPE1>> tokens;
            size_t pos = 0;
            int line = 1, col = 1;

            while (pos < input.size()) {
                char c = input[pos];

                // Handle whitespace
                if (isspace(static_cast<unsigned char>(c))) {
                    if (c == '\n') { 
                        line++; 
                        col = 1; 
                    } else {
                        col++;
                    }
                    pos++;
                    continue;
                }

                // Handle comments
                if (c == '/' && pos + 1 < input.size()) {
                    // Single line comment
                    if (input[pos + 1] == '/') {
                        pos += 2;
                        col += 2;
                        while (pos < input.size() && input[pos] != '\n') {
                            pos++;
                            col++;
                        }
                        continue;
                    } 
                    // Multi-line comment
                    else if (input[pos + 1] == '*') {
                        pos += 2;
                        col += 2;
                        while (pos + 1 < input.size() && !(input[pos] == '*' && input[pos + 1] == '/')) {
                            if (input[pos] == '\n') { 
                                line++; 
                                col = 1; 
                            } else {
                                col++;
                            }
                            pos++;
                        }
                        if (pos + 1 < input.size()) {
                            pos += 2;
                            col += 2;
                        }
                        continue;
                    }
                }

                bool matched = false;
                size_t startPos = pos;
                int startCol = col;
                
                for (auto& rec : recognizers) {
                    Token<TYPE1> tok;
                    size_t tempPos = pos;
                    if (rec->match(input, tempPos, tok, symbolTable, line, startCol)) {
                        tokens.push_back(tok);
                        col += static_cast<int>(tempPos - pos);
                        pos = tempPos;
                        matched = true;
                        break;
                    }
                }

                if (!matched) {
                    cerr << "[Error] Invalid token '" << input[pos] 
                         << "' at line " << line << ", column " << col << "\n";
                    tokens.emplace_back(TokenType::Unknown, string(1, input[pos]), line, col, pos);
                    pos++; 
                    col++;
                }
            }

            return tokens;
        }
};
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
        unordered_map<pair<string,string>,int,PairHash> VarName_DeclaredScope_Count; // { (variable_name,full_scope) : number of the variable in that scope }
        unordered_map<pair<string,string>,pair<pair<vector<string>,int>,string>,PairHash> Method; // { (method_name,full_scope) : ( ( parameters_type<> , parameters_count ) , return_type ) }
        unordered_map<string,string> VarName_UsedScope; // { variable_name : full_scope }
        unordered_map<string,string> VarName_DeclaredScope; // { variable_name : full_scope }

        void FillDeclaredVariables() {
            Scope* globalScope = symbol_table->getGlobalScope();
            TraverseScope(globalScope);
        }
        
        void TraverseScope(Scope* scope) {
            if (!scope) 
                return;
            
            const auto& symbols = scope->getSymbols();
            string fullScopePath = scope->getFullPath();
            
            for (const auto& [name, symbol] : symbols) {
                IdentifierKind kind = symbol.kind;
                
                if (kind == IdentifierKind::Variable ||  kind == IdentifierKind::Field ||  kind == IdentifierKind::Parameter) {
                    string varName = symbol.getName();
                    string varScope = symbol.getScope();
                    
                    pair<string, string> key = make_pair(varName, varScope);
                    VarName_DeclaredScope_Count[key]++;
                    
                    // declared scope (first occurrence)
                    if (VarName_DeclaredScope.find(varName) == VarName_DeclaredScope.end()) {
                        VarName_DeclaredScope[varName] = varScope;
                    }
                }
            }
            
            auto children = scope->getAllChildren();
            for (auto* child : children) {
                TraverseScope(child);
            }
        }

        string InferArgumentType(const vector<Token<string>>& tokens, size_t pos) {
            if (pos >= tokens.size()) 
                return "";
            
            Token<string> argToken = tokens[pos];
            TokenType tokenType = argToken.getType();
            
            // types for "abcd" or 1234 or ...
            if (tokenType == TokenType::Number) {
                return "int";
            } else if (tokenType == TokenType::StringLiteral) {
                return "String";
            } else if (tokenType == TokenType::CharLiteral) {
                return "char";
            } else if (tokenType == TokenType::BooleanLiteral) {
                return "boolean";
            } else if (tokenType == TokenType::NullLiteral) {
                return "null";
            } else if (tokenType == TokenType::Identifier) { // types for variables  --> use symbol table
                string identName = argToken.getLexeme();
                Symbol* sym = symbol_table->lookupSymbol(identName); //******
                
                if (sym) {
                    if (sym->kind == IdentifierKind::Variable) {
                        auto varData = dynamic_pointer_cast<VariableInfo>(sym->data);
                        if (varData) 
                            return varData->type.name;
                    } else if (sym->kind == IdentifierKind::Field) {
                        auto fieldData = dynamic_pointer_cast<FieldInfo>(sym->data);
                        if (fieldData) 
                            return fieldData->type.name;
                    } else if (sym->kind == IdentifierKind::Parameter) {
                        auto paramData = dynamic_pointer_cast<ParameterInfo>(sym->data);
                        if (paramData) 
                            return paramData->type.name;
                    }
                }
                
                // Default form is the identifier itself (could be a class name)
                return identName;
            }
            
            return "unknown";
        }

        void AnalyzeTokens(const vector<Token<string>>& tokens) {
            string currentScope = "GLOBAL";
            stack<string> scopeStack;
            scopeStack.push(currentScope);
            
            for (size_t i = 0; i < tokens.size(); i++) {
                const auto& token = tokens[i];
                string lexeme = token.getLexeme();
                TokenType type = token.getType();
                
                // save scope changes
                if (type == TokenType::Keyword) {
                    if (lexeme == "class" || lexeme == "interface") {
                        if (i + 1 < tokens.size() && tokens[i+1].getType() == TokenType::Identifier) {
                            string className = tokens[i+1].getLexeme();
                            currentScope = scopeStack.top() + "::" + className;
                            scopeStack.push(currentScope);
                        }
                    }
                }
                
                // save method scope
                if (type == TokenType::Identifier && i + 1 < tokens.size()) {
                    Token<string> nextToken = tokens[i+1];
                    
                    // method call or method declaration
                    if (nextToken.getType() == TokenType::Delimiter && nextToken.getLexeme() == "(") { 
                        bool isMethodCall = true;
                        
                        // method declaration
                        if (i > 0) {
                            Token<string> prevToken = tokens[i-1];
                            if (prevToken.getType() == TokenType::Keyword) {
                                string kw = prevToken.getLexeme();
                                if (kw == "void" || kw == "int" || kw == "boolean" || 
                                    kw == "char" || kw == "String" || kw == "public" || 
                                    kw == "private" || kw == "protected" || kw == "static") {
                                    isMethodCall = false;
                                }
                            }
                            // custom class type
                            else if (prevToken.getType() == TokenType::Identifier) {
                                isMethodCall = false;
                            }
                        }
                        
                        if (isMethodCall) {
                            string methodName = lexeme;
                            vector<string> argTypes;
                            int argCount = 0;
                            
                            size_t j = i + 2; // skip method name and '('
                            int parenDepth = 1;
                            bool expectingArg = true;
                            
                            while (j < tokens.size() && parenDepth > 0) {
                                Token<string> argToken = tokens[j];
                                
                                if (argToken.getType() == TokenType::Delimiter) {
                                    if (argToken.getLexeme() == "(") {//*********
                                        parenDepth++;
                                    } else if (argToken.getLexeme() == ")") {
                                        parenDepth--;
                                        if (parenDepth == 0) 
                                            break;
                                    } else if (argToken.getLexeme() == "," && parenDepth == 1) {
                                        expectingArg = true;
                                        j++;
                                        continue;
                                    }
                                }
                                
                                // detect argument and infer its type
                                if (expectingArg && parenDepth == 1) {
                                    string argType = InferArgumentType(tokens, j);
                                    if (!argType.empty()) {
                                        argTypes.push_back(argType);
                                        argCount++;
                                        expectingArg = false;
                                    }
                                }
                                
                                j++;
                            }
                            
                            pair<string, string> key = make_pair(methodName, currentScope);
                            // return type ?????????????????
                            Method[key] = make_pair(make_pair(argTypes, argCount), "");
                            
                        } else {
                            string methodName = lexeme;
                            string methodScope = scopeStack.top() + "::" + methodName;
                            scopeStack.push(methodScope);
                        }
                    }
                }
                
                // exit scope with '}'
                if (type == TokenType::Delimiter && lexeme == "}") {
                    if (scopeStack.size() > 1) {
                        scopeStack.pop();
                        currentScope = scopeStack.top();
                    }
                }
                
                // variable
                if (type == TokenType::Identifier) {
                    bool isUsage = false;
                    
                    // use in operations
                    if (i + 1 < tokens.size()) {
                        Token<string> nextToken = tokens[i+1];
                        
                        // skip if this is a method call (***)
                        if (nextToken.getType() == TokenType::Delimiter && nextToken.getLexeme() == "(") {
                            continue;
                        }
                        
                        // Use in arithmetic/logical operations or assignment
                        if (nextToken.getType() == TokenType::Operator) {
                            string op = nextToken.getLexeme();
                            if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%" ||
                                op == "==" || op == "!=" || op == ">" || op == "<" || 
                                op == ">=" || op == "<=" || op == "&&" || op == "||") {
                                isUsage = true;
                            }
                        }
                        
                        // Use as function argument
                        if (nextToken.getType() == TokenType::Delimiter && 
                            (nextToken.getLexeme() == "," || nextToken.getLexeme() == ")")) {
                            // are we inside a function call (check argument list) ?
                            if (i > 0) {
                                for (int k = i - 1; k >= 0 && k >= (int)i - 10; k--) {
                                    if (tokens[k].getLexeme() == "(") {
                                        isUsage = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    
                    // variable is on right side for an assignment
                    if (i > 1) {
                        Token<string> prevToken = tokens[i-1];
                        if (prevToken.getType() == TokenType::Operator && prevToken.getLexeme() == "=") {
                            isUsage = true;
                        }
                    }
                    
                    // use variable in conditions or return statements *******
                    if (i > 0) {
                        Token<string> prevToken = tokens[i-1];
                        if (prevToken.getType() == TokenType::Keyword) {
                            string kw = prevToken.getLexeme();
                            if (kw == "if" || kw == "while" || kw == "return") {
                                isUsage = true;
                            }
                        }
                        
                        // Inside parentheses (condition or function call)
                        if (prevToken.getType() == TokenType::Delimiter && prevToken.getLexeme() == "(") {
                            isUsage = true;
                        }
                    }
                    
                    if (isUsage) {
                        Symbol* sym = symbol_table->lookupSymbol(lexeme);//********
                        if (sym && (sym->kind == IdentifierKind::Variable ||  sym->kind == IdentifierKind::Field ||   sym->kind == IdentifierKind::Parameter)) {
                            if (VarName_UsedScope.find(lexeme) == VarName_UsedScope.end()) {
                                VarName_UsedScope[lexeme] = currentScope;
                            }
                        }
                    }
                }
            }
        }

    public:
        ErrorDetection(const string buffer ,SymbolTable* symbol_table){
            this->buffer = buffer;
            this->symbol_table = symbol_table;
            this->pos = 0;
            this->ReadSourceCode();
        }

        void PrintAnalysis() {
            cout << "\n==================== Variable Declaration Analysis ====================\n";
            cout << "VarName_DeclaredScope_Count:\n";
            for (const auto& [key, count] : VarName_DeclaredScope_Count) {
                cout << "  Variable: " << key.first << " in scope: " << key.second 
                     << " - Count: " << count << "\n";
            }
            
            cout << "\n==================== Methods Information ====================\n";
            for (const auto& [key, value] : Method) {
                cout << "  Method: " << key.first << " in scope: " << key.second << "\n";
                cout << "    Parameters (" << value.first.second << "): ";
                for (const auto& paramType : value.first.first) {
                    cout << paramType << " ";
                }
                cout << "\n    Return Type: " << value.second << "\n";
            }
            
            cout << "\n==================== Variable Usage Analysis ====================\n";
            cout << "VarName_DeclaredScope:\n";
            for (const auto& [varName, scope] : VarName_DeclaredScope) {
                cout << "  Variable: " << varName << " declared in: " << scope << "\n";
            }
            
            cout << "\nVarName_UsedScope:\n";
            for (const auto& [varName, scope] : VarName_UsedScope) {
                cout << "  Variable: " << varName << " used in: " << scope << "\n";
            }
            cout << "=====================================================================\n\n";
        }

        void ReadSourceCode(){
            // filling VarName_DeclaredScope_Count and VarName_DeclaredScope from Symbol Table
            FillDeclaredVariables();//******** VarName_DeclaredScope_Count --> problem
            
            // analyze source code for variable usage and method calls
            Lexer<string> lexer(symbol_table);
            auto tokens = lexer.tokenize(buffer);
            
            AnalyzeTokens(tokens);

            //*******
            PrintAnalysis();
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

            // unordered_map<string, string> parentOf;
            // unordered_map<string, size_t> classLine;

            // for (const auto& sym : symTable.symbolsData) {
            //     if (sym.at("symbolType") == "class") {

            //         string className = sym.at("name");
            //         string parent = "null";

            //         if (sym.count("parent") && sym.at("parent") != "null")
            //             parent = sym.at("parent");

            //         parentOf[className] = parent;
            //         classLine[className] = stoi(sym.at("line"));
            //     }
            // }

            // unordered_set<string> reported;

            // for (const auto& [cls, _] : parentOf) {
            //     unordered_set<string> visited;
            //     string current = cls;

            //     while (parentOf.count(current) && parentOf[current] != "null") {
            //         if (visited.count(current)) {
            //             if (!reported.count(current)) {
            //                 ans.push_back(Error(classLine[current], ErrorType::CyclicInheritance));
            //                 error_num++;
            //                 reported.insert(current);
            //             }
            //             break;
            //         }

            //         visited.insert(current);
            //         current = parentOf[current];
            //     }
            // }

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
