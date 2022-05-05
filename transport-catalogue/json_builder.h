#pragma once
#include <optional>
#include <vector>

#include "json.h"

namespace json {

class BaseContext;
class DictItemContext;
class KeyItemContext;
class ItemAfterKeyContext;
class ArrayItemContext;

class Builder
{
public:
    Builder();

    Node Build();

    BaseContext Value(Node value);

    ArrayItemContext StartArray();
    BaseContext EndArray();

    DictItemContext StartDict();
    BaseContext Key(std::string key);
    BaseContext EndDict();

    ~Builder();
private:
    std::optional<Node> root_;
    std::vector<Node*> nodes_stack_{};
};

class BaseContext {
public:
    BaseContext(Builder& builder);

    Node Build();

    BaseContext Value(Node value);

    BaseContext StartArray();
    BaseContext EndArray();

    DictItemContext StartDict();
    KeyItemContext Key(std::string key);
    BaseContext EndDict();

    Builder& GetBuilder();

    ~BaseContext() = default;
private:
    Builder& builder_;
};

class DictItemContext : public BaseContext {
public:
    DictItemContext(Builder& builder);
    DictItemContext(BaseContext& context);

    Node Build() = delete;
    BaseContext Value() = delete;
    BaseContext StartArray() = delete;
    BaseContext EndArray() = delete;
    BaseContext StartDict() = delete;
};

class KeyItemContext : public BaseContext {
public:
    KeyItemContext(BaseContext& context);

    ItemAfterKeyContext Value(Node value);

    Node Build() = delete;
    BaseContext EndArray() = delete;
    BaseContext Key() = delete;
    BaseContext EndDict() = delete;
};

class ItemAfterKeyContext : public BaseContext {
public:
    ItemAfterKeyContext(BaseContext& context);

    Node Build() = delete;
    BaseContext Value(Node value) = delete;
    BaseContext StartArray() = delete;
    BaseContext EndArray() = delete;
    DictItemContext StartDict() = delete;
};

class ArrayItemContext :public BaseContext {
public:
    ArrayItemContext(Builder& builder);
    ArrayItemContext(BaseContext& context);

    ArrayItemContext Value(Node value);

    Node Build() = delete;
    KeyItemContext Key(std::string key) = delete;
    BaseContext EndDict() = delete;
};

}// json