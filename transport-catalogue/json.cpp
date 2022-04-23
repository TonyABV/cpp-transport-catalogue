#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadNull(istream& input) {
    string null;
    char token;
    while (input >> token) {
        if (token == ',' || token == ']' || token == '}') {
            input.putback(token);
            break;
        }
        else if (token == EOF) {
            break;
        }
        null.push_back(token);
    }

    if (null == "ull") {
        return Node();
    }
    else
    {
        throw ParsingError("");
    }
}

map<char, char> escape_me{ {'r', '\r'}, {'n', '\n'}, {'\"', '\"'}, {'\\', '\\'}, {'t', '\t'} };

Node LoadString(istream& input) {

    string result;
    char token;
    while (true) {
        token = input.get();

        if (token == '\"') {
            return Node(move(result));
        }
        else if (token == '\n' || token == '\r' || token == EOF) {
            throw ParsingError("");\
        }
        else if (token == '\\') {
            char next;
            next = input.get();
            auto ref = escape_me.find(next);
            if (ref != escape_me.end()) {
                result.push_back((*ref).second);
                continue;
            }
            else if (next != EOF) {
                result.push_back('\\');
                char n_char = next;
                result.push_back(n_char);
                continue;
            }
        }
        else {
            result.push_back(token);
        }
    }
    throw ParsingError("");
}

Node LoadArray(istream& input) {
    Array result;
    char token;
    bool is_first = true;
    for (; input >> token && token != ']';) {
        if (is_first) {
            input.putback(token);
            is_first = false;
        }
        result.push_back(LoadNode(input));
    }
    if (token != ']') {
        throw ParsingError("");
    }
    return Node(move(result));
}

Node LoadDict(istream& input) {
    Dict result;
    char token;
    bool is_first = true;
    for (;input >> token && token != '}';) {
        if (is_first) {
            input.putback(token);
            is_first = false;
        }
        string key;
        key = LoadNode(input).AsString();

        input >> token;
        if (token != ':') {
            throw ParsingError("");
        }

        result.insert({ key, LoadNode(input) });
        key.clear();
    }

    if (token != '}') {
        throw ParsingError("");
    }

    return Node(move(result));
}

using Number = std::variant<int, double>;

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    }
    else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            }
            catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    }
    catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadBool(istream& input) {
    string boolean;
    char token;
    while (input >> token) {
        if (token == ',' || token == ']' || token == '}') {
            input.putback(token);
            break;
        }
        boolean.push_back(token);
    }

    if (boolean == "rue") {
        return Node(true);
    }
    else if (boolean == "alse") {
        return Node(false);
    }
    else {
        throw ParsingError("");
    }
}

Node LoadNode(istream& input) {
    char c;
    input >> c;
    switch (c)
    {
    case '[': return LoadArray(input);
    case '{': return LoadDict(input);
    case '"': return LoadString(input);
    case 'n': return LoadNull(input);
    case 't': return LoadBool(input);
    case 'f': return LoadBool(input);
    case EOF: throw ParsingError("");
    default:
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace

Node::Node(Document doc){
    Node node = doc.GetRoot();
    node_ = node.TakeVar();
}

Node::Node(var_Node node) : node_(node){
}

Node::Node(Array array_node) : node_(array_node){
}

const var_Node& Node::TakeVar() const
{
    return node_;
}

const Array& Node::AsArray() const {
    if (IsArray()){
        return std::get<Array>( node_);
    }else {
        throw(logic_error(""));
    }
}

const Dict& Node::AsMap() const {
    if (IsMap()) {
        return std::get<Dict>(node_);
    }
    else {
        throw(logic_error(""));
    }
}

const bool& Node::AsBool() const
{
    if (IsBool()) {
        return std::get<bool>(node_);
    }
    else {
        throw(logic_error(""));
    }
}

int Node::AsInt() const {
    if (IsInt()) {
        return std::get<int>(node_);
    }
    else {
        throw(logic_error(""));
    }
}

double Node::AsDouble() const
{
    if (IsInt()) {
        return std::get<int>(node_);
    }
    else if (IsDouble()) {
        return std::get<double>(node_);
    }
    else {
        throw(logic_error(""));
    }
}

const string& Node::AsString() const {
    if (IsString()) {
        return std::get<string>(node_);
    }
    else {
        throw(logic_error(""));
    }
}

bool Node::IsNull() const{
    return std::holds_alternative<std::nullptr_t>(node_);
}

bool Node::IsArray() const{
    return std::holds_alternative<Array>(node_);
}

bool Node::IsMap() const{
    return std::holds_alternative<Dict>(node_);
}

bool Node::IsBool() const{
    return std::holds_alternative<bool>(node_);
}

bool Node::IsInt() const{
    return std::holds_alternative<int>(node_);
}

bool Node::IsDouble() const{
    return (std::holds_alternative<double>(node_)||
            std::holds_alternative<int>(node_));
}

bool Node::IsPureDouble() const
{
    return std::holds_alternative<double>(node_);
}

bool Node::IsString() const{
    return std::holds_alternative<string>(node_);
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    const Node& node = doc.GetRoot();
    (std::visit(PrintNodeConverter{ output }, node.TakeVar()));
}

bool operator==(const Node& lhs, const Node& rhs)
{
    if (lhs.IsNull()) {
        if (rhs.IsNull()) {
            return true;
        }
        return false;
    }
    else if (lhs.IsArray()) {
        if (rhs.IsArray()) {
            return lhs.AsArray() == rhs.AsArray();
        }
        return false;
    }
    else if (lhs.IsMap()) {
        if (rhs.IsMap()) {
            return lhs.AsMap() == rhs.AsMap();
        }
        return false;
    }
    else if (lhs.IsBool()) {
        if (rhs.IsBool()) {
            return lhs.AsBool() == rhs.AsBool();
        }
        return false;
    }
    else if (lhs.IsInt()) {
        if (rhs.IsInt()) {
            return lhs.AsInt() == rhs.AsInt();
        }
        return false;
    }
    else if (lhs.IsPureDouble()) {
        if (rhs.IsPureDouble()) {
            return lhs.AsDouble() == rhs.AsDouble();
        }
        return false;
    }
    else if (lhs.IsString()) {
        if (rhs.IsString()) {
            return lhs.AsString() == rhs.AsString();
        }
    }
    return false;
}

bool operator!=(const Node& lhs, const Node& rhs)
{
    return !(lhs == rhs);
}

std::ostream& PrintNodeConverter::operator()(std::nullptr_t) const {
    out_ << "null";
    return out_;
}

std::ostream& PrintNodeConverter::operator()(Array array) const
{
    out_ << "[";

    bool is_first = true;
    for (const auto& node : array) {
        if(is_first){
            std::visit(PrintNodeConverter{ out_ }, node.TakeVar());
            is_first = false;
            continue;
        }
        out_ << ", ";
        std::visit(PrintNodeConverter{ out_ }, node.TakeVar());
    }
    out_ << "]";
    return out_;
}

std::ostream& PrintNodeConverter::operator()(Dict dict) const
{
    out_ << "{ ";
    bool is_first = true;
    for (const auto& key_node : dict) {
        if (is_first) {
            out_ << "\"" << key_node.first << "\": ";
            std::visit(PrintNodeConverter{ out_ }, key_node.second.TakeVar());
            is_first = false;
            continue;
        }
        out_ << ", ";
        out_ << "\"" << key_node.first << "\": ";
        std::visit(PrintNodeConverter{ out_ }, key_node.second.TakeVar());
    }
    out_ << "}";
    return out_;
}

std::ostream& PrintNodeConverter::operator()(bool boo) const
{
    out_ << std::boolalpha << boo;
    return out_;
}

std::ostream& PrintNodeConverter::operator()(int num) const
{
    out_  << num;
    return out_;
}

std::ostream& PrintNodeConverter::operator()(double d_num) const
{
    out_ << d_num;
    return out_;
}

std::ostream& PrintNodeConverter::operator()(std::string text) const
{

    out_ << "\"";
    for (size_t n = 0; n < text.size(); ++n) {
        char c = text[n];
        auto ref = escape_seq_.find(c);
        if (ref != escape_seq_.end()) {
            out_ << '\\';
            out_ << (*ref).second;
            continue;
        }
        out_ << c;
    }
    out_ << "\"";
    return out_;
}

}  // namespace json