#include <bits/stdc++.h>

using namespace std;

namespace {

struct FastInput {
    static constexpr size_t kBufferSize = 1 << 20;

    char buffer[kBufferSize];
    size_t position = 0;
    size_t size = 0;

    char nextChar() {
        if (position >= size) {
            size = fread(buffer, 1, kBufferSize, stdin);
            position = 0;
            if (size == 0) {
                return EOF;
            }
        }
        return buffer[position++];
    }

    bool nextToken(string &token, bool &quoted) {
        token.clear();
        quoted = false;

        char ch = nextChar();
        while (ch != EOF && isspace(static_cast<unsigned char>(ch))) {
            ch = nextChar();
        }
        if (ch == EOF) {
            return false;
        }

        if (ch == '"') {
            quoted = true;
            ch = nextChar();
            while (ch != EOF && ch != '"') {
                token.push_back(ch);
                ch = nextChar();
            }
            return true;
        }

        while (ch != EOF && !isspace(static_cast<unsigned char>(ch))) {
            token.push_back(ch);
            ch = nextChar();
        }
        return true;
    }
};

enum class ValueType {
    Int,
    String,
};

struct Value {
    ValueType type = ValueType::Int;
    long long integer = 0;
    string text;
};

struct Variable {
    Value value;
};

bool parseInteger(const string &token, long long &value) {
    if (token.empty()) {
        return false;
    }
    size_t index = 0;
    bool negative = false;
    if (token[index] == '+' || token[index] == '-') {
        negative = token[index] == '-';
        ++index;
    }
    if (index == token.size()) {
        return false;
    }
    long long result = 0;
    for (; index < token.size(); ++index) {
        char ch = token[index];
        if (!isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
        result = result * 10 + (ch - '0');
    }
    value = negative ? -result : result;
    return true;
}

struct ScopeState {
    unordered_set<string> names;
    vector<string> order;
};

class ScopeSimulator {
public:
    void run() {
        string token;
        bool quoted = false;
        if (!input_.nextToken(token, quoted)) {
            return;
        }

        long long commandCount = 0;
        parseInteger(token, commandCount);

        scopes_.push_back(ScopeState{});

        for (long long i = 0; i < commandCount; ++i) {
            input_.nextToken(token, quoted);
            if (token == "Indent") {
                scopes_.push_back(ScopeState{});
                continue;
            }
            if (token == "Dedent") {
                handleDedent();
                continue;
            }
            if (token == "Declare") {
                handleDeclare();
                continue;
            }
            if (token == "Add") {
                handleAdd();
                continue;
            }
            if (token == "SelfAdd") {
                handleSelfAdd();
                continue;
            }
            if (token == "Print") {
                handlePrint();
                continue;
            }
        }
    }

private:
    FastInput input_;
    unordered_map<string, vector<Variable>> table_;
    vector<ScopeState> scopes_;

    Variable *lookup(const string &name) {
        auto tableIt = table_.find(name);
        if (tableIt == table_.end() || tableIt->second.empty()) {
            return nullptr;
        }
        return &tableIt->second.back();
    }

    const Variable *lookup(const string &name) const {
        auto tableIt = table_.find(name);
        if (tableIt == table_.end() || tableIt->second.empty()) {
            return nullptr;
        }
        return &tableIt->second.back();
    }

    void invalid() {
        cout << "Invalid operation\n";
    }

    void handleDedent() {
        if (scopes_.size() <= 1) {
            invalid();
            return;
        }

        ScopeState &scope = scopes_.back();
        for (const string &name : scope.order) {
            auto tableIt = table_.find(name);
            tableIt->second.pop_back();
            if (tableIt->second.empty()) {
                table_.erase(tableIt);
            }
        }
        scopes_.pop_back();
    }

    void handleDeclare() {
        string typeToken;
        bool quoted = false;
        input_.nextToken(typeToken, quoted);

        string name;
        input_.nextToken(name, quoted);

        string valueToken;
        bool valueQuoted = false;
        input_.nextToken(valueToken, valueQuoted);

        ScopeState &scope = scopes_.back();
        if (scope.names.find(name) != scope.names.end()) {
            invalid();
            return;
        }

        Variable variable;
        if (typeToken == "int") {
            long long number = 0;
            if (valueQuoted || !parseInteger(valueToken, number)) {
                invalid();
                return;
            }
            variable.value.type = ValueType::Int;
            variable.value.integer = number;
        } else if (typeToken == "string") {
            if (!valueQuoted) {
                invalid();
                return;
            }
            variable.value.type = ValueType::String;
            variable.value.text = valueToken;
        } else {
            invalid();
            return;
        }

        scope.names.insert(name);
        scope.order.push_back(name);
        table_[name].push_back(std::move(variable));
    }

    void handleAdd() {
        string resultName;
        bool quoted = false;
        input_.nextToken(resultName, quoted);

        string leftName;
        input_.nextToken(leftName, quoted);

        string rightName;
        input_.nextToken(rightName, quoted);

        Variable *result = lookup(resultName);
        const Variable *left = lookup(leftName);
        const Variable *right = lookup(rightName);
        if (result == nullptr || left == nullptr || right == nullptr) {
            invalid();
            return;
        }
        if (result->value.type != left->value.type || left->value.type != right->value.type) {
            invalid();
            return;
        }

        if (left->value.type == ValueType::Int) {
            result->value.integer = left->value.integer + right->value.integer;
        } else {
            result->value.text = left->value.text + right->value.text;
        }
    }

    void handleSelfAdd() {
        string name;
        bool quoted = false;
        input_.nextToken(name, quoted);

        string valueToken;
        bool valueQuoted = false;
        input_.nextToken(valueToken, valueQuoted);

        Variable *variable = lookup(name);
        if (variable == nullptr) {
            invalid();
            return;
        }

        if (variable->value.type == ValueType::Int) {
            long long number = 0;
            if (valueQuoted || !parseInteger(valueToken, number)) {
                invalid();
                return;
            }
            variable->value.integer += number;
        } else {
            if (!valueQuoted) {
                invalid();
                return;
            }
            variable->value.text += valueToken;
        }
    }

    void handlePrint() {
        string name;
        bool quoted = false;
        input_.nextToken(name, quoted);

        const Variable *variable = lookup(name);
        if (variable == nullptr) {
            invalid();
            return;
        }

        cout << name << ':';
        if (variable->value.type == ValueType::Int) {
            cout << variable->value.integer << '\n';
        } else {
            cout << variable->value.text << '\n';
        }
    }
};

}  // namespace

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ScopeSimulator simulator;
    simulator.run();
    return 0;
}
