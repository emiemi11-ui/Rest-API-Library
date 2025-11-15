#pragma once
#include "data/productrepository.hpp"
#include "models/product.hpp"
#include <vector>
#include <string>
#include <optional>

class ProductService {
private:
    ProductRepository& repository;

    // Validation helpers
    void validateProduct(const Product& product);
    void validateProductForUpdate(int id);
    void validateSku(const std::string& sku);
    void validatePrice(double price);
    void validateStock(int stock);

public:
    explicit ProductService(ProductRepository& repository) : repository(repository) {}

    // Product CRUD operations
    std::vector<Product> getAllProducts();
    std::optional<Product> getProduct(int id);
    std::optional<Product> getProductBySku(const std::string& sku);
    Product createProduct(const Product& product);
    void updateProduct(int id, const Product& product);
    void deleteProduct(int id);

    // Category and search
    std::vector<Product> getProductsByCategory(const std::string& category);
    std::vector<Product> searchProducts(const std::string& keyword);
    std::vector<Product> getActiveProducts();

    // Stock management
    void updateStock(int id, int quantity);
    std::vector<Product> getLowStock(int threshold = 10);

    // Pagination
    std::vector<Product> getProductsPaginated(int page, int limit, const std::string& sort = "id");
    int getTotalProductCount();
};
