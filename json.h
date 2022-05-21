#pragma once

#include <cmath>
//#include <deque>
#include <execution>
#include <iomanip>
#include <iostream>
#include <map>
//#include <unordered_map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace json {

class Node;
class Document;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
//using Deq = std::deque<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final
    :private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>
{
public:
    using variant::variant;
    using Value = variant;

    //const VarNode& TakeVar()const;
    const Value& GetValue() const;
    Value& GetValue();

    const Array& AsArray() const;
    const Dict& AsMap() const;
    const bool& AsBool() const;
    int AsInt() const;
    double AsDouble() const;
    const std::string& AsString() const;

    bool IsNull() const;
    bool IsArray()const;
    bool IsDict() const;
    bool IsBool() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsString() const;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Node& doc, std::ostream& output);

bool operator==(const Node& lhs, const Node& rhs);
bool operator!=(const Node& lhs, const Node& rhs);

struct PrintNodeConverter
{
    PrintNodeConverter(std::ostream& out) :out_(out) {}

    std::ostream& operator()(std::nullptr_t) const;
    std::ostream& operator()(const Array& array) const;
    std::ostream& operator()(const Dict& dict) const;
    std::ostream& operator()(bool boo)const;
    std::ostream& operator()(int num) const;
    std::ostream& operator()(double num) const;
    std::ostream& operator()(const std::string& text)const;

private:
    std::ostream& out_;
    std::map<char, char> escape_seq_{ {'\r', 'r'}, {'\n', 'n'}, {'\"', '\"'}, {'\\', '\\'} };
};

}  // namespace json