#include "json_builder.h"

namespace json {

json::Builder::Builder()
{
}

Node Builder::Build()
{
    if (!root_.has_value()) {
        throw std::logic_error("root_ is empty");
    }
    else if (!nodes_stack_.empty()) {
        throw std::logic_error("Stack is't empty");
    }
    return root_.value();
}

BaseContext Builder::Value(Node value)
{
    if (!root_.has_value()) {
        root_ = value;
        return *this;
    }
    else if (nodes_stack_.empty()) {
        throw std::logic_error("Stack is empty. Expected Builder::Build()");
    }
    else if (nodes_stack_.back()->IsNull()) {
        Node::Value& node = nodes_stack_.back()->GetValue();
        node = value.GetValue();
        nodes_stack_.pop_back();
        return *this;
    }
    else if (nodes_stack_.back()->IsArray()) {
        Node::Value& array = nodes_stack_.back()->GetValue();
        (std::get<Array>(array)).push_back(value);
        return *this;
    }
    else {
        throw std::logic_error("Invalid request.");
    }
}

ArrayItemContext Builder::StartArray()
{
    if (!root_.has_value()) {
        root_ = Array{};
        nodes_stack_.push_back(&root_.value());
        return *this;
    }
    else if (nodes_stack_.empty()) {
        throw std::logic_error("Stack is empty. Expected Builder::Build()");
    }
    else if (nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Last node is Dict. Expected Builder::Key()");
    }
    else if (nodes_stack_.back()->IsArray()) {
        Array& arr = std::get<Array>(nodes_stack_.back()->GetValue());
        arr.push_back(Node(Array{}));
        nodes_stack_.push_back(&arr.back());
        return *this;
    }

    nodes_stack_.back()->GetValue() = Array{};
    return *this;
}

BaseContext Builder::EndArray()
{
    if (nodes_stack_.empty()) {
        throw std::logic_error("Stack is empty");
    }
    else if (!nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Last node in stack is't Array");
    }
    nodes_stack_.pop_back();
    return *this;
}

DictItemContext Builder::StartDict()
{
    if (!root_.has_value()) {
        root_ = Dict{};
        nodes_stack_.push_back(&root_.value());
        return *this;
    }
    else if (nodes_stack_.empty()) {
        throw std::logic_error("Stack is empty. Expected Builder::Build()");
    }
    else if (nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Last node is Dict. Expected Builder::Key()");
    }
    else if (nodes_stack_.back()->IsArray()) {
        Array& array = std::get<Array>(nodes_stack_.back()->GetValue());
        (array).push_back(Dict{});
        nodes_stack_.push_back(&array.back());
        return *this;
    }
    Node::Value& node = nodes_stack_.back()->GetValue();
    node = Dict{};
    return *this;
}

BaseContext Builder::Key(std::string key)
{
    if (nodes_stack_.empty()) {
        throw std::logic_error("Stack is empty");
    }
    else if (!nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Last node is't Dict");
    }
    Dict& dict = std::get<Dict>(nodes_stack_.back()->GetValue());
    dict.insert({ key, Node{} });
    nodes_stack_.push_back(&(dict.at(key)));
    return *this;
}

BaseContext Builder::EndDict()
{
    if (nodes_stack_.empty()) {
        throw std::logic_error("Stack is empty");
    }
    else if (!nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Last node is't Dict");
    }
    nodes_stack_.pop_back();
    return *this;
}

Builder::~Builder(){}

BaseContext::BaseContext(Builder& builder) : builder_(builder) {}

BaseContext BaseContext::Value(json::Node value)
{
    return builder_.Value(value);
}

BaseContext BaseContext::StartArray()
{
    return builder_.StartArray();
}

BaseContext BaseContext::EndArray()
{
    return builder_.EndArray();
}

DictItemContext BaseContext::StartDict()
{
    BaseContext context = builder_.StartDict();
    return static_cast<DictItemContext&>(context);
}

KeyItemContext BaseContext::Key(std::string key)
{
    BaseContext context = builder_.Key(key);
    return static_cast<KeyItemContext>(context);
}

Builder& BaseContext::GetBuilder()
{
    return builder_;
}

BaseContext BaseContext::EndDict()
{
    return builder_.EndDict();
}

Node json::BaseContext::Build()
{
    return builder_.Build();
}

DictItemContext::DictItemContext(Builder& builder) : BaseContext(builder) {}
DictItemContext::DictItemContext(BaseContext& context) : BaseContext(context) {}

KeyItemContext::KeyItemContext(BaseContext& context) : BaseContext(context) {}

ItemAfterKeyContext KeyItemContext::Value(Node value)
{
    BaseContext context = GetBuilder().Value(value);
    return static_cast<ItemAfterKeyContext>(context);
}

ItemAfterKeyContext::ItemAfterKeyContext(BaseContext& context) : BaseContext(context) {}

ArrayItemContext::ArrayItemContext(Builder& builder) : BaseContext(builder) {}
ArrayItemContext::ArrayItemContext(BaseContext& context) : BaseContext(context) {}

ArrayItemContext ArrayItemContext::Value(Node value)
{
    BaseContext context = GetBuilder().Value(value);
    return static_cast<ArrayItemContext>(context);
}

}// json