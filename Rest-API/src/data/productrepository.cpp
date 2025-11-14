#include "data/productrepository.hpp"
#include <sstream>
#include <iostream>

// Helper for SQL escaping
static std::string escapeSql(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (c == '\'') result += "''";
        else result += c;
    }
    return result;
}

std::vector<Product> ProductRepository::getAll() {
    std::vector<Product> products;
    std::string sql = "SELECT id, sku, name, description, price, stock_quantity, "
                      "category, image_url, is_active, created_at, updated_at, created_by "
                      "FROM products ORDER BY id ASC";

    for (auto& row : conn.db().query(sql)) {
        Product p{};
        if (row.count("id")) p.id = std::stoi(row.at("id"));
        if (row.count("sku")) p.sku = row.at("sku");
        if (row.count("name")) p.name = row.at("name");
        if (row.count("description")) p.description = row.at("description");
        if (row.count("price")) p.price = std::stod(row.at("price"));
        if (row.count("stock_quantity")) p.stock_quantity = std::stoi(row.at("stock_quantity"));
        if (row.count("category")) p.category = row.at("category");
        if (row.count("image_url")) p.image_url = row.at("image_url");
        if (row.count("is_active")) p.is_active = (std::stoi(row.at("is_active")) != 0);
        if (row.count("created_by")) p.created_by = std::stoi(row.at("created_by"));
        products.push_back(std::move(p));
    }
    return products;
}

std::optional<Product> ProductRepository::getById(int id) {
    std::ostringstream sql;
    sql << "SELECT id, sku, name, description, price, stock_quantity, "
        << "category, image_url, is_active, created_at, updated_at, created_by "
        << "FROM products WHERE id = " << id;

    auto rows = conn.db().query(sql.str());
    if (rows.empty()) {
        return std::nullopt;
    }

    auto& row = rows[0];
    Product p{};
    if (row.count("id")) p.id = std::stoi(row.at("id"));
    if (row.count("sku")) p.sku = row.at("sku");
    if (row.count("name")) p.name = row.at("name");
    if (row.count("description")) p.description = row.at("description");
    if (row.count("price")) p.price = std::stod(row.at("price"));
    if (row.count("stock_quantity")) p.stock_quantity = std::stoi(row.at("stock_quantity"));
    if (row.count("category")) p.category = row.at("category");
    if (row.count("image_url")) p.image_url = row.at("image_url");
    if (row.count("is_active")) p.is_active = (std::stoi(row.at("is_active")) != 0);
    if (row.count("created_by")) p.created_by = std::stoi(row.at("created_by"));

    return p;
}

std::optional<Product> ProductRepository::getBySku(const std::string& sku) {
    std::ostringstream sql;
    sql << "SELECT id, sku, name, description, price, stock_quantity, "
        << "category, image_url, is_active, created_at, updated_at, created_by "
        << "FROM products WHERE sku = '" << escapeSql(sku) << "'";

    auto rows = conn.db().query(sql.str());
    if (rows.empty()) {
        return std::nullopt;
    }

    auto& row = rows[0];
    Product p{};
    if (row.count("id")) p.id = std::stoi(row.at("id"));
    if (row.count("sku")) p.sku = row.at("sku");
    if (row.count("name")) p.name = row.at("name");
    if (row.count("description")) p.description = row.at("description");
    if (row.count("price")) p.price = std::stod(row.at("price"));
    if (row.count("stock_quantity")) p.stock_quantity = std::stoi(row.at("stock_quantity"));
    if (row.count("category")) p.category = row.at("category");
    if (row.count("image_url")) p.image_url = row.at("image_url");
    if (row.count("is_active")) p.is_active = (std::stoi(row.at("is_active")) != 0);
    if (row.count("created_by")) p.created_by = std::stoi(row.at("created_by"));

    return p;
}

std::vector<Product> ProductRepository::getByCategory(const std::string& category) {
    std::vector<Product> products;
    std::ostringstream sql;
    sql << "SELECT id, sku, name, description, price, stock_quantity, "
        << "category, image_url, is_active, created_at, updated_at, created_by "
        << "FROM products WHERE category = '" << escapeSql(category) << "' ORDER BY id ASC";

    for (auto& row : conn.db().query(sql.str())) {
        Product p{};
        if (row.count("id")) p.id = std::stoi(row.at("id"));
        if (row.count("sku")) p.sku = row.at("sku");
        if (row.count("name")) p.name = row.at("name");
        if (row.count("description")) p.description = row.at("description");
        if (row.count("price")) p.price = std::stod(row.at("price"));
        if (row.count("stock_quantity")) p.stock_quantity = std::stoi(row.at("stock_quantity"));
        if (row.count("category")) p.category = row.at("category");
        if (row.count("image_url")) p.image_url = row.at("image_url");
        if (row.count("is_active")) p.is_active = (std::stoi(row.at("is_active")) != 0);
        if (row.count("created_by")) p.created_by = std::stoi(row.at("created_by"));
        products.push_back(std::move(p));
    }
    return products;
}

Product ProductRepository::create(const Product& product) {
    std::ostringstream sql;
    sql << "INSERT INTO products(sku, name, description, price, stock_quantity, "
        << "category, image_url, is_active, created_by) VALUES('"
        << escapeSql(product.sku) << "', '"
        << escapeSql(product.name) << "', '"
        << escapeSql(product.description) << "', "
        << product.price << ", "
        << product.stock_quantity << ", '"
        << escapeSql(product.category) << "', '"
        << escapeSql(product.image_url) << "', "
        << (product.is_active ? 1 : 0) << ", "
        << product.created_by << ")";

    if (!conn.db().execute(sql.str())) {
        throw std::runtime_error("Failed to insert product");
    }

    // Get generated ID
    auto rows = conn.db().query("SELECT last_insert_rowid() as id");
    if (rows.empty()) {
        throw std::runtime_error("Failed to get inserted product ID");
    }

    Product created = product;
    created.id = std::stoi(rows[0].at("id"));
    return created;
}

void ProductRepository::update(const Product& product) {
    std::ostringstream sql;
    sql << "UPDATE products SET "
        << "sku = '" << escapeSql(product.sku) << "', "
        << "name = '" << escapeSql(product.name) << "', "
        << "description = '" << escapeSql(product.description) << "', "
        << "price = " << product.price << ", "
        << "stock_quantity = " << product.stock_quantity << ", "
        << "category = '" << escapeSql(product.category) << "', "
        << "image_url = '" << escapeSql(product.image_url) << "', "
        << "is_active = " << (product.is_active ? 1 : 0) << " "
        << "WHERE id = " << product.id;

    if (!conn.db().execute(sql.str())) {
        throw std::runtime_error("Failed to update product");
    }
}

void ProductRepository::deleteById(int id) {
    std::ostringstream sql;
    sql << "DELETE FROM products WHERE id = " << id;

    if (!conn.db().execute(sql.str())) {
        throw std::runtime_error("Failed to delete product");
    }
}

bool ProductRepository::updateStock(int id, int quantity) {
    std::ostringstream sql;
    sql << "UPDATE products SET stock_quantity = " << quantity
        << " WHERE id = " << id;

    return conn.db().execute(sql.str());
}

std::vector<Product> ProductRepository::searchProducts(const std::string& keyword) {
    std::vector<Product> products;
    std::ostringstream sql;
    sql << "SELECT id, sku, name, description, price, stock_quantity, "
        << "category, image_url, is_active, created_at, updated_at, created_by "
        << "FROM products WHERE name LIKE '%" << escapeSql(keyword) << "%' "
        << "OR description LIKE '%" << escapeSql(keyword) << "%' "
        << "ORDER BY id ASC";

    for (auto& row : conn.db().query(sql.str())) {
        Product p{};
        if (row.count("id")) p.id = std::stoi(row.at("id"));
        if (row.count("sku")) p.sku = row.at("sku");
        if (row.count("name")) p.name = row.at("name");
        if (row.count("description")) p.description = row.at("description");
        if (row.count("price")) p.price = std::stod(row.at("price"));
        if (row.count("stock_quantity")) p.stock_quantity = std::stoi(row.at("stock_quantity"));
        if (row.count("category")) p.category = row.at("category");
        if (row.count("image_url")) p.image_url = row.at("image_url");
        if (row.count("is_active")) p.is_active = (std::stoi(row.at("is_active")) != 0);
        if (row.count("created_by")) p.created_by = std::stoi(row.at("created_by"));
        products.push_back(std::move(p));
    }
    return products;
}

std::vector<Product> ProductRepository::getLowStock(int threshold) {
    std::vector<Product> products;
    std::ostringstream sql;
    sql << "SELECT id, sku, name, description, price, stock_quantity, "
        << "category, image_url, is_active, created_at, updated_at, created_by "
        << "FROM products WHERE stock_quantity < " << threshold
        << " AND stock_quantity > 0 ORDER BY stock_quantity ASC";

    for (auto& row : conn.db().query(sql.str())) {
        Product p{};
        if (row.count("id")) p.id = std::stoi(row.at("id"));
        if (row.count("sku")) p.sku = row.at("sku");
        if (row.count("name")) p.name = row.at("name");
        if (row.count("description")) p.description = row.at("description");
        if (row.count("price")) p.price = std::stod(row.at("price"));
        if (row.count("stock_quantity")) p.stock_quantity = std::stoi(row.at("stock_quantity"));
        if (row.count("category")) p.category = row.at("category");
        if (row.count("image_url")) p.image_url = row.at("image_url");
        if (row.count("is_active")) p.is_active = (std::stoi(row.at("is_active")) != 0);
        if (row.count("created_by")) p.created_by = std::stoi(row.at("created_by"));
        products.push_back(std::move(p));
    }
    return products;
}

std::vector<Product> ProductRepository::getActive() {
    std::vector<Product> products;
    std::string sql = "SELECT id, sku, name, description, price, stock_quantity, "
                      "category, image_url, is_active, created_at, updated_at, created_by "
                      "FROM products WHERE is_active = 1 ORDER BY id ASC";

    for (auto& row : conn.db().query(sql)) {
        Product p{};
        if (row.count("id")) p.id = std::stoi(row.at("id"));
        if (row.count("sku")) p.sku = row.at("sku");
        if (row.count("name")) p.name = row.at("name");
        if (row.count("description")) p.description = row.at("description");
        if (row.count("price")) p.price = std::stod(row.at("price"));
        if (row.count("stock_quantity")) p.stock_quantity = std::stoi(row.at("stock_quantity"));
        if (row.count("category")) p.category = row.at("category");
        if (row.count("image_url")) p.image_url = row.at("image_url");
        if (row.count("is_active")) p.is_active = (std::stoi(row.at("is_active")) != 0);
        if (row.count("created_by")) p.created_by = std::stoi(row.at("created_by"));
        products.push_back(std::move(p));
    }
    return products;
}

std::vector<Product> ProductRepository::getAllPaginated(int page, int limit, const std::string& sort) {
    std::vector<Product> products;
    int offset = (page - 1) * limit;

    std::ostringstream sql;
    sql << "SELECT id, sku, name, description, price, stock_quantity, "
        << "category, image_url, is_active, created_at, updated_at, created_by "
        << "FROM products ORDER BY " << escapeSql(sort) << " ASC "
        << "LIMIT " << limit << " OFFSET " << offset;

    for (auto& row : conn.db().query(sql.str())) {
        Product p{};
        if (row.count("id")) p.id = std::stoi(row.at("id"));
        if (row.count("sku")) p.sku = row.at("sku");
        if (row.count("name")) p.name = row.at("name");
        if (row.count("description")) p.description = row.at("description");
        if (row.count("price")) p.price = std::stod(row.at("price"));
        if (row.count("stock_quantity")) p.stock_quantity = std::stoi(row.at("stock_quantity"));
        if (row.count("category")) p.category = row.at("category");
        if (row.count("image_url")) p.image_url = row.at("image_url");
        if (row.count("is_active")) p.is_active = (std::stoi(row.at("is_active")) != 0);
        if (row.count("created_by")) p.created_by = std::stoi(row.at("created_by"));
        products.push_back(std::move(p));
    }
    return products;
}

int ProductRepository::getTotalCount() {
    std::string sql = "SELECT COUNT(*) as cnt FROM products";
    auto rows = conn.db().query(sql);
    if (rows.empty()) return 0;
    return std::stoi(rows[0].at("cnt"));
}

bool ProductRepository::existsBySku(const std::string& sku) {
    std::ostringstream sql;
    sql << "SELECT COUNT(*) as cnt FROM products WHERE sku = '"
        << escapeSql(sku) << "'";

    auto rows = conn.db().query(sql.str());
    if (rows.empty()) return false;

    int count = std::stoi(rows[0].at("cnt"));
    return count > 0;
}

bool ProductRepository::existsById(int id) {
    std::ostringstream sql;
    sql << "SELECT COUNT(*) as cnt FROM products WHERE id = " << id;

    auto rows = conn.db().query(sql.str());
    if (rows.empty()) return false;

    int count = std::stoi(rows[0].at("cnt"));
    return count > 0;
}
