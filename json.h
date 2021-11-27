#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
using Dictionary = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final : private std::variant<std::nullptr_t, Array, Dictionary, bool, int, double, std::string> {
public:
    using variant::variant;

public:
    inline bool IsNull() const noexcept {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    inline bool IsArray() const noexcept {
        return std::holds_alternative<Array>(*this);
    }

    inline bool IsMap() const noexcept {
        return std::holds_alternative<Dictionary>(*this);
    }

    inline bool IsBool() const noexcept {
        return std::holds_alternative<bool>(*this);
    }

    inline bool IsInt() const noexcept {
        return std::holds_alternative<int>(*this);
    }

    inline bool IsDouble() const noexcept {
        return std::holds_alternative<double>(*this) ||
            std::holds_alternative<int>(*this);
    }

    inline bool IsPureDouble() const noexcept {
        return std::holds_alternative<double>(*this);
    }

    inline bool IsString() const noexcept {
        return std::holds_alternative<std::string>(*this);
    }

public:
    inline const Array& AsArray() const {
        if (IsArray()) {
            return std::get<Array>(*this);
        }
        else {
            throw std::logic_error("Not an array");
        }
    }

    inline Array& AsArray() {
        return const_cast<Array&>(const_cast<const Node*>(this)->AsArray());
    }

    inline const Dictionary& AsMap() const {
        if (IsMap()) {
            return std::get<Dictionary>(*this);
        }
        else {
            throw std::logic_error("Not a map");
        }
    }

    inline Dictionary& AsMap() {
        return const_cast<Dictionary&>(const_cast<const Node*>(this)->AsMap());
    }

    inline bool AsBool() const {
        if (IsBool()) {
            return std::get<bool>(*this);
        }
        else {
            throw std::logic_error("Not a bool");
        }
    }

    inline int AsInt() const {
        if (IsInt()) {
            return std::get<int>(*this);
        }
        else {
            throw std::logic_error("Not an int");
        }
    }

    inline double AsDouble() const {
        if (IsPureDouble()) {
            return std::get<double>(*this);
        }
        else if (IsInt()) {
            return static_cast<double>(std::get<int>(*this));
        }
        else {
            throw std::logic_error("Not a double or int");
        }
    }

    inline const std::string& AsString() const {
        if (IsString()) {
            return std::get<std::string>(*this);
        }
        else {
            throw std::logic_error("Not a string");
        }
    }

    inline std::string& AsString() {
        return const_cast<std::string&>(const_cast<const Node*>(this)->AsString());
    }

public:
    inline bool operator==(const Node& right) const noexcept {
        return static_cast<variant>(*this) == right;
    }

    inline bool operator!=(const Node& right) const noexcept {
        return !(*this == right);
    }
};

std::ostream& operator<<(std::ostream& output, const Node& node);

class Document {
public:
    Document() = default;

    explicit Document(Node root);

public:
    const Node& GetRoot() const;

public:
    inline bool operator==(const Document& right) const noexcept {
        return root_ == right.root_;
    }

    inline bool operator!=(const Document& right) const noexcept {
        return !(*this == right);
    }

private:
    Node root_;
};

Document Load(std::istream& input);

} // namespace json