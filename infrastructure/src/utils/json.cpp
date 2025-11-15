#include "../../include/utils/json.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>

namespace utils {
namespace json {

// Constructors
JSON::JSON() : type_(Type::Null), bool_value_(false), number_value_(0.0) {}

JSON::JSON(Type type) : type_(type), bool_value_(false), number_value_(0.0) {
    if (type == Type::Object) {
        object_value_ = std::map<std::string, JSON>();
    } else if (type == Type::Array) {
        array_value_ = std::vector<JSON>();
    }
}

JSON::JSON(bool value) : type_(Type::Boolean), bool_value_(value), number_value_(0.0) {}

JSON::JSON(int value) : type_(Type::Number), bool_value_(false), number_value_(static_cast<double>(value)) {}

JSON::JSON(double value) : type_(Type::Number), bool_value_(false), number_value_(value) {}

JSON::JSON(const std::string& value) : type_(Type::String), bool_value_(false), number_value_(0.0), string_value_(value) {}

JSON::JSON(const char* value) : type_(Type::String), bool_value_(false), number_value_(0.0), string_value_(value) {}

// Static factory methods
JSON JSON::object() {
    return JSON(Type::Object);
}

JSON JSON::array() {
    return JSON(Type::Array);
}

// Value getters
bool JSON::asBool() const {
    if (type_ != Type::Boolean) throw std::runtime_error("JSON value is not a boolean");
    return bool_value_;
}

int JSON::asInt() const {
    if (type_ != Type::Number) throw std::runtime_error("JSON value is not a number");
    return static_cast<int>(number_value_);
}

double JSON::asDouble() const {
    if (type_ != Type::Number) throw std::runtime_error("JSON value is not a number");
    return number_value_;
}

std::string JSON::asString() const {
    if (type_ != Type::String) throw std::runtime_error("JSON value is not a string");
    return string_value_;
}

// Object operations
JSON& JSON::operator[](const std::string& key) {
    if (type_ != Type::Object) {
        throw std::runtime_error("JSON value is not an object");
    }
    return object_value_[key];
}

const JSON& JSON::operator[](const std::string& key) const {
    if (type_ != Type::Object) {
        throw std::runtime_error("JSON value is not an object");
    }
    auto it = object_value_.find(key);
    if (it == object_value_.end()) {
        throw std::runtime_error("Key not found: " + key);
    }
    return it->second;
}

bool JSON::has(const std::string& key) const {
    if (type_ != Type::Object) return false;
    return object_value_.find(key) != object_value_.end();
}

void JSON::set(const std::string& key, const JSON& value) {
    if (type_ != Type::Object) {
        throw std::runtime_error("JSON value is not an object");
    }
    object_value_[key] = value;
}

// Array operations
JSON& JSON::operator[](size_t index) {
    if (type_ != Type::Array) {
        throw std::runtime_error("JSON value is not an array");
    }
    if (index >= array_value_.size()) {
        throw std::out_of_range("Array index out of range");
    }
    return array_value_[index];
}

const JSON& JSON::operator[](size_t index) const {
    if (type_ != Type::Array) {
        throw std::runtime_error("JSON value is not an array");
    }
    if (index >= array_value_.size()) {
        throw std::out_of_range("Array index out of range");
    }
    return array_value_[index];
}

void JSON::push(const JSON& value) {
    if (type_ != Type::Array) {
        throw std::runtime_error("JSON value is not an array");
    }
    array_value_.push_back(value);
}

size_t JSON::size() const {
    if (type_ == Type::Array) {
        return array_value_.size();
    } else if (type_ == Type::Object) {
        return object_value_.size();
    }
    return 0;
}

// Serialization
std::string JSON::toString() const {
    std::ostringstream ss;

    switch (type_) {
        case Type::Null:
            ss << "null";
            break;
        case Type::Boolean:
            ss << (bool_value_ ? "true" : "false");
            break;
        case Type::Number:
            ss << number_value_;
            break;
        case Type::String:
            ss << "\"" << escapeString(string_value_) << "\"";
            break;
        case Type::Array:
            ss << "[";
            for (size_t i = 0; i < array_value_.size(); ++i) {
                if (i > 0) ss << ",";
                ss << array_value_[i].toString();
            }
            ss << "]";
            break;
        case Type::Object:
            ss << "{";
            bool first = true;
            for (const auto& pair : object_value_) {
                if (!first) ss << ",";
                ss << "\"" << escapeString(pair.first) << "\":" << pair.second.toString();
                first = false;
            }
            ss << "}";
            break;
    }

    return ss.str();
}

std::string JSON::toStringPretty(int indent) const {
    std::ostringstream ss;
    std::string indent_str(indent * 2, ' ');
    std::string next_indent_str((indent + 1) * 2, ' ');

    switch (type_) {
        case Type::Null:
        case Type::Boolean:
        case Type::Number:
        case Type::String:
            return toString();
        case Type::Array:
            if (array_value_.empty()) {
                return "[]";
            }
            ss << "[\n";
            for (size_t i = 0; i < array_value_.size(); ++i) {
                if (i > 0) ss << ",\n";
                ss << next_indent_str << array_value_[i].toStringPretty(indent + 1);
            }
            ss << "\n" << indent_str << "]";
            break;
        case Type::Object:
            if (object_value_.empty()) {
                return "{}";
            }
            ss << "{\n";
            bool first = true;
            for (const auto& pair : object_value_) {
                if (!first) ss << ",\n";
                ss << next_indent_str << "\"" << escapeString(pair.first) << "\": "
                   << pair.second.toStringPretty(indent + 1);
                first = false;
            }
            ss << "\n" << indent_str << "}";
            break;
    }

    return ss.str();
}

// Helper: escape string
std::string JSON::escapeString(const std::string& str) {
    std::ostringstream ss;
    for (char c : str) {
        switch (c) {
            case '"':  ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\b': ss << "\\b"; break;
            case '\f': ss << "\\f"; break;
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            default:
                if (c < 0x20) {
                    ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                } else {
                    ss << c;
                }
        }
    }
    return ss.str();
}

// Parsing helper methods
void JSON::skipWhitespace(const std::string& json_str, size_t& pos) {
    while (pos < json_str.length() && std::isspace(json_str[pos])) {
        ++pos;
    }
}

JSON JSON::parse(const std::string& json_str) {
    size_t pos = 0;
    skipWhitespace(json_str, pos);
    return parseValue(json_str, pos);
}

JSON JSON::parseValue(const std::string& json_str, size_t& pos) {
    skipWhitespace(json_str, pos);

    if (pos >= json_str.length()) {
        throw std::runtime_error("Unexpected end of JSON");
    }

    char c = json_str[pos];

    if (c == '{') return parseObject(json_str, pos);
    if (c == '[') return parseArray(json_str, pos);
    if (c == '"') return parseString(json_str, pos);
    if (c == 't' || c == 'f') return parseBool(json_str, pos);
    if (c == 'n') return parseNull(json_str, pos);
    if (c == '-' || std::isdigit(c)) return parseNumber(json_str, pos);

    throw std::runtime_error("Invalid JSON");
}

JSON JSON::parseObject(const std::string& json_str, size_t& pos) {
    JSON obj = JSON::object();
    ++pos; // skip '{'

    skipWhitespace(json_str, pos);
    if (pos < json_str.length() && json_str[pos] == '}') {
        ++pos;
        return obj;
    }

    while (pos < json_str.length()) {
        skipWhitespace(json_str, pos);

        // Parse key
        if (json_str[pos] != '"') {
            throw std::runtime_error("Expected string key");
        }
        std::string key = parseString(json_str, pos).asString();

        skipWhitespace(json_str, pos);
        if (json_str[pos] != ':') {
            throw std::runtime_error("Expected ':'");
        }
        ++pos;

        // Parse value
        JSON value = parseValue(json_str, pos);
        obj.set(key, value);

        skipWhitespace(json_str, pos);
        if (json_str[pos] == '}') {
            ++pos;
            break;
        }
        if (json_str[pos] != ',') {
            throw std::runtime_error("Expected ',' or '}'");
        }
        ++pos;
    }

    return obj;
}

JSON JSON::parseArray(const std::string& json_str, size_t& pos) {
    JSON arr = JSON::array();
    ++pos; // skip '['

    skipWhitespace(json_str, pos);
    if (pos < json_str.length() && json_str[pos] == ']') {
        ++pos;
        return arr;
    }

    while (pos < json_str.length()) {
        JSON value = parseValue(json_str, pos);
        arr.push(value);

        skipWhitespace(json_str, pos);
        if (json_str[pos] == ']') {
            ++pos;
            break;
        }
        if (json_str[pos] != ',') {
            throw std::runtime_error("Expected ',' or ']'");
        }
        ++pos;
    }

    return arr;
}

JSON JSON::parseString(const std::string& json_str, size_t& pos) {
    ++pos; // skip opening '"'
    std::ostringstream ss;

    while (pos < json_str.length()) {
        char c = json_str[pos];
        if (c == '"') {
            ++pos;
            return JSON(ss.str());
        }
        if (c == '\\') {
            ++pos;
            if (pos >= json_str.length()) {
                throw std::runtime_error("Unexpected end of string");
            }
            c = json_str[pos];
            switch (c) {
                case '"': ss << '"'; break;
                case '\\': ss << '\\'; break;
                case '/': ss << '/'; break;
                case 'b': ss << '\b'; break;
                case 'f': ss << '\f'; break;
                case 'n': ss << '\n'; break;
                case 'r': ss << '\r'; break;
                case 't': ss << '\t'; break;
                default: throw std::runtime_error("Invalid escape sequence");
            }
        } else {
            ss << c;
        }
        ++pos;
    }

    throw std::runtime_error("Unterminated string");
}

JSON JSON::parseNumber(const std::string& json_str, size_t& pos) {
    size_t start = pos;

    if (json_str[pos] == '-') ++pos;

    while (pos < json_str.length() && (std::isdigit(json_str[pos]) || json_str[pos] == '.' ||
           json_str[pos] == 'e' || json_str[pos] == 'E' || json_str[pos] == '+' || json_str[pos] == '-')) {
        ++pos;
    }

    std::string num_str = json_str.substr(start, pos - start);
    return JSON(std::stod(num_str));
}

JSON JSON::parseBool(const std::string& json_str, size_t& pos) {
    if (json_str.substr(pos, 4) == "true") {
        pos += 4;
        return JSON(true);
    } else if (json_str.substr(pos, 5) == "false") {
        pos += 5;
        return JSON(false);
    }
    throw std::runtime_error("Invalid boolean");
}

JSON JSON::parseNull(const std::string& json_str, size_t& pos) {
    if (json_str.substr(pos, 4) == "null") {
        pos += 4;
        return JSON();
    }
    throw std::runtime_error("Invalid null");
}

} // namespace json
} // namespace utils
