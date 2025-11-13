#include "../../include/utils/queryparams.hpp"
#include "../../include/utils/json.hpp"
#include <sstream>
#include <algorithm>

namespace utils {

QueryParams::QueryParams(const std::string& query_string)
    : page_(1), limit_(20), sort_desc_(false) {
    parse(query_string);
}

void QueryParams::parse(const std::string& query_string) {
    if (query_string.empty()) return;

    std::istringstream stream(query_string);
    std::string pair;

    while (std::getline(stream, pair, '&')) {
        size_t eq_pos = pair.find('=');
        if (eq_pos == std::string::npos) continue;

        std::string key = urlDecode(pair.substr(0, eq_pos));
        std::string value = urlDecode(pair.substr(eq_pos + 1));

        params_[key] = value;

        // Handle special parameters
        if (key == "page") {
            try {
                page_ = std::max(1, std::stoi(value));
            } catch (...) {
                page_ = 1;
            }
        } else if (key == "limit") {
            try {
                limit_ = std::min(100, std::max(1, std::stoi(value)));
            } catch (...) {
                limit_ = 20;
            }
        } else if (key == "sort") {
            parseSort(value);
        } else if (key == "fields") {
            parseFields(value);
        } else if (key == "q" || key == "search") {
            search_query_ = value;
        } else {
            // Store as filter
            filters_[key] = value;
        }
    }
}

void QueryParams::parseSort(const std::string& sort_value) {
    if (sort_value.empty()) return;

    if (sort_value[0] == '-') {
        sort_desc_ = true;
        sort_field_ = sort_value.substr(1);
    } else if (sort_value[0] == '+') {
        sort_desc_ = false;
        sort_field_ = sort_value.substr(1);
    } else {
        sort_desc_ = false;
        sort_field_ = sort_value;
    }
}

void QueryParams::parseFields(const std::string& fields_value) {
    std::istringstream stream(fields_value);
    std::string field;

    while (std::getline(stream, field, ',')) {
        if (!field.empty()) {
            fields_.push_back(field);
        }
    }
}

bool QueryParams::hasFilter(const std::string& field) const {
    return filters_.find(field) != filters_.end();
}

std::string QueryParams::getFilter(const std::string& field) const {
    auto it = filters_.find(field);
    return (it != filters_.end()) ? it->second : "";
}

std::string QueryParams::get(const std::string& key, const std::string& default_value) const {
    auto it = params_.find(key);
    return (it != params_.end()) ? it->second : default_value;
}

std::string QueryParams::urlDecode(const std::string& str) {
    std::string result;
    result.reserve(str.length());

    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value;
            std::istringstream is(str.substr(i + 1, 2));
            if (is >> std::hex >> value) {
                result += static_cast<char>(value);
                i += 2;
            } else {
                result += str[i];
            }
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }

    return result;
}

std::string PaginationMeta::toJson() const {
    json::JSON json = json::JSON::object();
    json["page"] = page;
    json["limit"] = limit;
    json["total"] = total;
    json["total_pages"] = total_pages;
    json["has_next"] = has_next;
    json["has_prev"] = has_prev;

    return json.toString();
}

} // namespace utils
