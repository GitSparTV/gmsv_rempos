#include "json.h"

#include <sstream>

namespace json {

Node LoadNode(std::istream& input);
Node LoadString(std::istream& input);

Node LoadArray(std::istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }

        result.push_back(LoadNode(input));
    }

    if (!input) {
        throw ParsingError("Array is not closed");
    }

    return Node(move(result));
}

Node LoadDict(std::istream& input) {
    Dictionary result;

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        std::string key = LoadString(input).AsString();

        input >> c;

        result.emplace(move(key), LoadNode(input));
    }

    if (!input) {
        throw ParsingError("Dictionary is not closed");
    }

    return Node(move(result));
}

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());

        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

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
    if (input.peek() == '0') {
        read_char();
    }
    else {
        read_digits();
    }

    bool is_int = true;

    if (input.peek() == '.') {
        read_char();
        read_digits();

        is_int = false;
    }

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
            try {
                return std::stoi(parsed_num);
            }
            catch (...) {
            }
        }

        return std::stod(parsed_num);
    }
    catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input) {
    std::string line;

    bool escape_next = false;

    for (int symbol = input.get(); input; symbol = input.get()) {
        if (escape_next) {
            switch (symbol) {
                case 'n':
                {
                    line += '\n';
                    break;
                }
                case 'r':
                {
                    line += '\r';
                    break;
                }
                case 't':
                {
                    line += '\t';
                    break;
                }
                case '"':
                    [[fallthrough]];
                case '\\':
                {
                    line += static_cast<char>(symbol);
                    break;
                }
                default:
                {
                    throw ParsingError("Invalid escape sequence");
                    break;
                }
            }

            escape_next = false;
        }
        else if (symbol == '\\') {
            escape_next = true;
            continue;
        }
        else if (symbol == '"') {
            break;
        }
        else {
            line += static_cast<char>(symbol);
        }
    }

    if (!input) {
        throw ParsingError("String is not closed");
    }

    return Node(move(line));
}

Node LoadNode(std::istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    }
    else if (c == '{') {
        return LoadDict(input);
    }
    else if (c == '"') {
        return LoadString(input);
    }
    else if (c == 'n') {
        if (input.get() == 'u' && input.get() == 'l' && input.get() == 'l') {
            return Node{};
        }
        else {
            throw ParsingError("Unexpected end of JSON input");
        }
    }
    else if (c == 't') {
        if (input.get() == 'r' && input.get() == 'u' && input.get() == 'e') {
            return Node{true};
        }
        else {
            throw ParsingError("Unexpected end of JSON input");
        }
    }
    else if (c == 'f') {
        if (input.get() == 'a' && input.get() == 'l' && input.get() == 's' &&
            input.get() == 'e') {
            return Node{false};
        }
        else {
            throw ParsingError("Unexpected end of JSON input");
        }
    }
    else {
        input.putback(c);
        return LoadNumber(input);
    }
}

std::ostream& operator<<(std::ostream& out, const Node& node) {
    if (node.IsNull()) {
        return (out << "null");
    }
    else if (node.IsArray()) {
        out << '[';

        bool not_first = false;

        for (const Node& sub_node : node.AsArray()) {
            out << (not_first ? ", " : "") << sub_node;

            not_first = true;
        }

        return (out << ']');
    }
    else if (node.IsMap()) {
        out << '{';

        bool not_first = false;

        for (const auto& [key, sub_node] : node.AsMap()) {
            out << (not_first ? ", " : "") << "\"" << key << "\": " << sub_node;

            not_first = true;
        }

        return (out << '}');
    }
    else if (node.IsInt()) {
        return (out << node.AsInt());
    }
    else if (node.IsPureDouble()) {
        return (out << node.AsDouble());
    }
    else if (node.IsBool()) {
        return (out << (node.AsBool() ? "true" : "false"));
    }
    else if (node.IsString()) {
        out << '"';

        std::string sanitized;

        const std::string& node_string = node.AsString();
        sanitized.reserve(node_string.size());

        for (auto symbol : node_string) {
            if (symbol == '\n') {
                sanitized += "\\n";
            }
            else if (symbol == '\r') {
                sanitized += "\\r";
            }
            else if (symbol == '\t') {
                sanitized += "\\t";
            }
            else if (symbol == '\\') {
                sanitized += "\\\\";
            }
            else if (symbol == '"') {
                sanitized += "\\\"";
            }
            else {
                sanitized += symbol;
            }
        }

        return (out << sanitized << '"');
    }

    return out;
}

Document::Document(Node root) : root_(move(root)) {}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

} // namespace json