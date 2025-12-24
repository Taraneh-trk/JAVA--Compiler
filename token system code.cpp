
// ============================= part 1 from phase 1 - Token System ======================================

//We use token system here for finding line number of all Identifier in O(n)

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

// ============================= Line number Storing ======================================

template <typename TYPE1 = string>
class LineNumberTracker {
private:
    string buffer;
    size_t pos;
    int line;
    int col;
    vector<Token<TYPE1>> tokens;
    
    vector<unique_ptr<BaseTokenRecognizer<TYPE1>>> recognizers;
    
    void skipWhitespaceAndComments() {
        while (pos < buffer.size()) {
            if (isspace(buffer[pos])) {
                if (buffer[pos] == '\n') {
                    line++;
                    col = 1;
                } else {
                    col++;
                }
                pos++;
            }
            else if (pos + 1 < buffer.size() && buffer[pos] == '/' && buffer[pos + 1] == '/') {
                pos += 2;
                col += 2;
                while (pos < buffer.size() && buffer[pos] != '\n') {
                    pos++;
                    col++;
                }
                if (pos < buffer.size() && buffer[pos] == '\n') {
                    pos++;
                    line++;
                    col = 1;
                }
            }
            else if (pos + 1 < buffer.size() && buffer[pos] == '/' && buffer[pos + 1] == '*') {
                pos += 2;
                col += 2;
                while (pos + 1 < buffer.size()) {
                    if (buffer[pos] == '\n') {
                        line++;
                        col = 1;
                        pos++;
                    } else if (buffer[pos] == '*' && buffer[pos + 1] == '/') {
                        pos += 2;
                        col += 2;
                        break;
                    } else {
                        pos++;
                        col++;
                    }
                }
            }
            else {
                break;
            }
        }
    }
    
public:
    Lexer(const string& input) : buffer(input), pos(0), line(1), col(1) {
        recognizers.push_back(make_unique<LiteralRecognizer<TYPE1>>());
        recognizers.push_back(make_unique<KeywordRecognizer<TYPE1>>());
        recognizers.push_back(make_unique<NumberRecognizer<TYPE1>>());
        recognizers.push_back(make_unique<OperatorRecognizer<TYPE1>>());
        recognizers.push_back(make_unique<DelimiterRecognizer<TYPE1>>());
        recognizers.push_back(make_unique<IdentifierRecognizer<TYPE1>>());
    }
    
    vector<Token<TYPE1>> tokenize() {
        tokens.clear();
        pos = 0;
        line = 1;
        col = 1;
        
        while (pos < buffer.size()) {
            skipWhitespaceAndComments();
            
            if (pos >= buffer.size()) break;
            
            int startLine = line;
            int startCol = col;
            size_t startPos = pos;
            
            Token<TYPE1> token;
            bool matched = false;
            
            for (auto& recognizer : recognizers) {
                size_t tempPos = pos;
                if (recognizer->match(buffer, tempPos, token, nullptr, line, col)) {
                    int consumed = static_cast<int>(tempPos - pos);
                    pos = tempPos;
                    col += consumed;
                    matched = true;
                    tokens.push_back(token);
                    break;
                }
            }
            
            if (!matched) {
                cerr << "[Lexical Error] Unrecognized token at line " << line 
                     << ", col " << col << ": '" << buffer[pos] << "'\n";
                pos++;
                col++;
            }
        }
        
        return tokens;
    }
    
    const vector<Token<TYPE1>>& getTokens() const {
        return tokens;
    }
};