#ifndef JSON_HPP
#define JSON_HPP

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <memory>
#include <stdexcept>

namespace utils {
namespace json {

/**
 * @brief Simple JSON parser and builder for C++
 *
 * Provides basic JSON functionality without external dependencies.
 * Supports objects, arrays, strings, numbers, booleans, and null.
 */
class JSON {
public:
    enum class Type {
        Null,
        Boolean,
        Number,
        String,
        Array,
        Object
    };

    // Constructors
    JSON();
    JSON(Type type);
    JSON(bool value);
    JSON(int value);
    JSON(double value);
    JSON(const std::string& value);
    JSON(const char* value);

    // Static factory methods
    static JSON object();
    static JSON array();
    static JSON parse(const std::string& json_str);

    // Type checking
    bool isNull() const { return type_ == Type::Null; }
    bool isBool() const { return type_ == Type::Boolean; }
    bool isNumber() const { return type_ == Type::Number; }
    bool isString() const { return type_ == Type::String; }
    bool isArray() const { return type_ == Type::Array; }
    bool isObject() const { return type_ == Type::Object; }

    // Value getters
    bool asBool() const;
    int asInt() const;
    double asDouble() const;
    std::string asString() const;

    // Object operations
    JSON& operator[](const std::string& key);
    const JSON& operator[](const std::string& key) const;
    bool has(const std::string& key) const;
    void set(const std::string& key, const JSON& value);

    // Array operations
    JSON& operator[](size_t index);
    const JSON& operator[](size_t index) const;
    void push(const JSON& value);
    size_t size() const;

    // Serialization
    std::string toString() const;
    std::string toStringPretty(int indent = 0) const;

private:
    Type type_;
    bool bool_value_;
    double number_value_;
    std::string string_value_;
    std::map<std::string, JSON> object_value_;
    std::vector<JSON> array_value_;

    // Helper methods for parsing
    static JSON parseValue(const std::string& json_str, size_t& pos);
    static JSON parseObject(const std::string& json_str, size_t& pos);
    static JSON parseArray(const std::string& json_str, size_t& pos);
    static JSON parseString(const std::string& json_str, size_t& pos);
    static JSON parseNumber(const std::string& json_str, size_t& pos);
    static JSON parseBool(const std::string& json_str, size_t& pos);
    static JSON parseNull(const std::string& json_str, size_t& pos);
    static void skipWhitespace(const std::string& json_str, size_t& pos);
    static std::string escapeString(const std::string& str);
};

} // namespace json
} // namespace utils

#endif // JSON_HPP
