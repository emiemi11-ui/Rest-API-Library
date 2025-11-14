#pragma once
#include "data/databaseconnection.hpp"
#include "models/product.hpp"
#include <string>
#include <vector>
#include <optional>

class ProductRepository {
    DatabaseConnection& conn;
public:
    explicit ProductRepository(DatabaseConnection& c): conn(c) {}

    // CRUD operations
    std::vector<Product> getAll();
    std::optional<Product> getById(int id);
    std::optional<Product> getBySku(const std::string& sku);
    std::vector<Product> getByCategory(const std::string& category);
    Product create(const Product& product);
    void update(const Product& product);
    void deleteById(int id);

    // Stock management
    bool updateStock(int id, int quantity);

    // Search and filtering
    std::vector<Product> searchProducts(const std::string& keyword);
    std::vector<Product> getLowStock(int threshold = 10);
    std::vector<Product> getActive();

    // Pagination support
    std::vector<Product> getAllPaginated(int page, int limit, const std::string& sort = "id");
    int getTotalCount();

    // Validation
    bool existsBySku(const std::string& sku);
    bool existsById(int id);
};
