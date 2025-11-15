#include "services/productservice.hpp"
#include <iostream>
#include <stdexcept>
#include <regex>

std::vector<Product> ProductService::getAllProducts() {
    std::cout << "[ProductService] getAllProducts()\n";
    return repository.getAll();
}

std::optional<Product> ProductService::getProduct(int id) {
    std::cout << "[ProductService] getProduct(" << id << ")\n";

    if (id <= 0) {
        throw std::invalid_argument("ID invalid: must be positive");
    }

    return repository.getById(id);
}

std::optional<Product> ProductService::getProductBySku(const std::string& sku) {
    std::cout << "[ProductService] getProductBySku(" << sku << ")\n";

    validateSku(sku);
    return repository.getBySku(sku);
}

Product ProductService::createProduct(const Product& product) {
    std::cout << "[ProductService] createProduct(" << product.name << ")\n";

    // Validate product data
    validateProduct(product);

    // Check if SKU already exists
    if (repository.existsBySku(product.sku)) {
        throw std::invalid_argument("SKU already exists: " + product.sku);
    }

    return repository.create(product);
}

void ProductService::updateProduct(int id, const Product& product) {
    std::cout << "[ProductService] updateProduct(" << id << ")\n";

    validateProductForUpdate(id);
    validateProduct(product);

    // Check if SKU is being changed and if it conflicts with another product
    auto existing = repository.getBySku(product.sku);
    if (existing.has_value() && existing->id != id) {
        throw std::invalid_argument("SKU is used by another product");
    }

    Product updated_product = product;
    updated_product.id = id;

    repository.update(updated_product);
}

void ProductService::deleteProduct(int id) {
    std::cout << "[ProductService] deleteProduct(" << id << ")\n";

    validateProductForUpdate(id);
    repository.deleteById(id);
}

std::vector<Product> ProductService::getProductsByCategory(const std::string& category) {
    std::cout << "[ProductService] getProductsByCategory(" << category << ")\n";

    if (category.empty()) {
        throw std::invalid_argument("Category cannot be empty");
    }

    return repository.getByCategory(category);
}

std::vector<Product> ProductService::searchProducts(const std::string& keyword) {
    std::cout << "[ProductService] searchProducts(" << keyword << ")\n";

    if (keyword.empty()) {
        throw std::invalid_argument("Search keyword cannot be empty");
    }

    if (keyword.length() < 2) {
        throw std::invalid_argument("Search keyword must be at least 2 characters");
    }

    return repository.searchProducts(keyword);
}

std::vector<Product> ProductService::getActiveProducts() {
    std::cout << "[ProductService] getActiveProducts()\n";
    return repository.getActive();
}

void ProductService::updateStock(int id, int quantity) {
    std::cout << "[ProductService] updateStock(" << id << ", " << quantity << ")\n";

    validateProductForUpdate(id);
    validateStock(quantity);

    if (!repository.updateStock(id, quantity)) {
        throw std::runtime_error("Failed to update stock");
    }
}

std::vector<Product> ProductService::getLowStock(int threshold) {
    std::cout << "[ProductService] getLowStock(" << threshold << ")\n";

    if (threshold < 0) {
        throw std::invalid_argument("Threshold must be non-negative");
    }

    return repository.getLowStock(threshold);
}

std::vector<Product> ProductService::getProductsPaginated(int page, int limit, const std::string& sort) {
    std::cout << "[ProductService] getProductsPaginated(page=" << page
              << ", limit=" << limit << ", sort=" << sort << ")\n";

    if (page <= 0) {
        throw std::invalid_argument("Page must be positive");
    }

    if (limit <= 0 || limit > 100) {
        throw std::invalid_argument("Limit must be between 1 and 100");
    }

    // Validate sort field to prevent SQL injection
    if (sort != "id" && sort != "name" && sort != "price" &&
        sort != "stock_quantity" && sort != "created_at") {
        throw std::invalid_argument("Invalid sort field");
    }

    return repository.getAllPaginated(page, limit, sort);
}

int ProductService::getTotalProductCount() {
    std::cout << "[ProductService] getTotalProductCount()\n";
    return repository.getTotalCount();
}

// ========== VALIDATION METHODS ==========

void ProductService::validateProduct(const Product& product) {
    // Validate SKU
    validateSku(product.sku);

    // Validate name
    if (product.name.empty()) {
        throw std::invalid_argument("Product name cannot be empty");
    }
    if (product.name.length() > 255) {
        throw std::invalid_argument("Product name is too long (max 255 characters)");
    }

    // Validate description
    if (product.description.length() > 1000) {
        throw std::invalid_argument("Description is too long (max 1000 characters)");
    }

    // Validate price
    validatePrice(product.price);

    // Validate stock
    validateStock(product.stock_quantity);

    // Validate category
    if (product.category.empty()) {
        throw std::invalid_argument("Category cannot be empty");
    }
    if (product.category.length() > 100) {
        throw std::invalid_argument("Category is too long (max 100 characters)");
    }

    // Validate image URL
    if (product.image_url.length() > 500) {
        throw std::invalid_argument("Image URL is too long (max 500 characters)");
    }
}

void ProductService::validateProductForUpdate(int id) {
    if (id <= 0) {
        throw std::invalid_argument("ID invalid: must be positive");
    }

    if (!repository.existsById(id)) {
        throw std::invalid_argument("Product with ID=" + std::to_string(id) + " does not exist");
    }
}

void ProductService::validateSku(const std::string& sku) {
    if (sku.empty()) {
        throw std::invalid_argument("SKU cannot be empty");
    }

    if (sku.length() > 50) {
        throw std::invalid_argument("SKU is too long (max 50 characters)");
    }

    // SKU should contain only alphanumeric characters, hyphens, and underscores
    std::regex sku_regex("^[A-Za-z0-9_-]+$");
    if (!std::regex_match(sku, sku_regex)) {
        throw std::invalid_argument("SKU contains invalid characters (only alphanumeric, -, _ allowed)");
    }
}

void ProductService::validatePrice(double price) {
    if (price < 0) {
        throw std::invalid_argument("Price cannot be negative");
    }

    if (price > 1000000.0) {
        throw std::invalid_argument("Price is too high (max 1,000,000)");
    }
}

void ProductService::validateStock(int stock) {
    if (stock < 0) {
        throw std::invalid_argument("Stock quantity cannot be negative");
    }

    if (stock > 1000000) {
        throw std::invalid_argument("Stock quantity is too high (max 1,000,000)");
    }
}
