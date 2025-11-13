#ifndef QUERYPARAMS_HPP
#define QUERYPARAMS_HPP

#include <string>
#include <map>
#include <vector>

namespace utils {

/**
 * @brief Query parameters parser and handler
 *
 * Supports pagination, filtering, sorting, and field selection.
 */
class QueryParams {
public:
    /**
     * @brief Constructor
     * @param query_string URL query string (e.g., "page=1&limit=10&sort=-created_at")
     */
    explicit QueryParams(const std::string& query_string);

    // Pagination
    int getPage() const { return page_; }
    int getLimit() const { return limit_; }
    int getOffset() const { return (page_ - 1) * limit_; }

    // Sorting
    bool hasSort() const { return !sort_field_.empty(); }
    std::string getSortField() const { return sort_field_; }
    bool isSortDescending() const { return sort_desc_; }

    // Filtering
    bool hasFilter(const std::string& field) const;
    std::string getFilter(const std::string& field) const;
    std::map<std::string, std::string> getAllFilters() const { return filters_; }

    // Field selection
    bool hasFields() const { return !fields_.empty(); }
    std::vector<std::string> getFields() const { return fields_; }

    // Search
    bool hasSearch() const { return !search_query_.empty(); }
    std::string getSearchQuery() const { return search_query_; }

    // Get parameter by name
    std::string get(const std::string& key, const std::string& default_value = "") const;

private:
    int page_;
    int limit_;
    std::string sort_field_;
    bool sort_desc_;
    std::map<std::string, std::string> filters_;
    std::vector<std::string> fields_;
    std::string search_query_;
    std::map<std::string, std::string> params_;

    void parse(const std::string& query_string);
    void parseSort(const std::string& sort_value);
    void parseFields(const std::string& fields_value);
    std::string urlDecode(const std::string& str);
};

/**
 * @brief Pagination response helper
 */
struct PaginationMeta {
    int page;
    int limit;
    int total;
    int total_pages;
    bool has_next;
    bool has_prev;

    std::string toJson() const;
};

} // namespace utils

#endif // QUERYPARAMS_HPP
