#include "data/orderrepository.hpp"
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

std::vector<Order> OrderRepository::getAll() {
    std::vector<Order> orders;
    std::string sql = "SELECT id, user_id, total_amount, status, shipping_address, "
                      "notes, created_at, updated_at FROM orders ORDER BY created_at DESC";

    for (auto& row : conn.db().query(sql)) {
        Order o{};
        if (row.count("id")) o.id = std::stoi(row.at("id"));
        if (row.count("user_id")) o.user_id = std::stoi(row.at("user_id"));
        if (row.count("total_amount")) o.total_amount = std::stod(row.at("total_amount"));
        if (row.count("status")) o.status = row.at("status");
        if (row.count("shipping_address")) o.shipping_address = row.at("shipping_address");
        if (row.count("notes")) o.notes = row.at("notes");
        orders.push_back(std::move(o));
    }
    return orders;
}

std::optional<Order> OrderRepository::getById(int id) {
    std::ostringstream sql;
    sql << "SELECT id, user_id, total_amount, status, shipping_address, "
        << "notes, created_at, updated_at FROM orders WHERE id = " << id;

    auto rows = conn.db().query(sql.str());
    if (rows.empty()) {
        return std::nullopt;
    }

    auto& row = rows[0];
    Order o{};
    if (row.count("id")) o.id = std::stoi(row.at("id"));
    if (row.count("user_id")) o.user_id = std::stoi(row.at("user_id"));
    if (row.count("total_amount")) o.total_amount = std::stod(row.at("total_amount"));
    if (row.count("status")) o.status = row.at("status");
    if (row.count("shipping_address")) o.shipping_address = row.at("shipping_address");
    if (row.count("notes")) o.notes = row.at("notes");

    return o;
}

std::vector<Order> OrderRepository::getByUserId(int user_id) {
    std::vector<Order> orders;
    std::ostringstream sql;
    sql << "SELECT id, user_id, total_amount, status, shipping_address, "
        << "notes, created_at, updated_at FROM orders WHERE user_id = "
        << user_id << " ORDER BY created_at DESC";

    for (auto& row : conn.db().query(sql.str())) {
        Order o{};
        if (row.count("id")) o.id = std::stoi(row.at("id"));
        if (row.count("user_id")) o.user_id = std::stoi(row.at("user_id"));
        if (row.count("total_amount")) o.total_amount = std::stod(row.at("total_amount"));
        if (row.count("status")) o.status = row.at("status");
        if (row.count("shipping_address")) o.shipping_address = row.at("shipping_address");
        if (row.count("notes")) o.notes = row.at("notes");
        orders.push_back(std::move(o));
    }
    return orders;
}

Order OrderRepository::create(const Order& order) {
    std::ostringstream sql;
    sql << "INSERT INTO orders(user_id, total_amount, status, shipping_address, notes) VALUES("
        << order.user_id << ", "
        << order.total_amount << ", '"
        << escapeSql(order.status) << "', '"
        << escapeSql(order.shipping_address) << "', '"
        << escapeSql(order.notes) << "')";

    if (!conn.db().execute(sql.str())) {
        throw std::runtime_error("Failed to insert order");
    }

    // Get generated ID
    auto rows = conn.db().query("SELECT last_insert_rowid() as id");
    if (rows.empty()) {
        throw std::runtime_error("Failed to get inserted order ID");
    }

    Order created = order;
    created.id = std::stoi(rows[0].at("id"));
    return created;
}

void OrderRepository::update(const Order& order) {
    std::ostringstream sql;
    sql << "UPDATE orders SET "
        << "total_amount = " << order.total_amount << ", "
        << "status = '" << escapeSql(order.status) << "', "
        << "shipping_address = '" << escapeSql(order.shipping_address) << "', "
        << "notes = '" << escapeSql(order.notes) << "' "
        << "WHERE id = " << order.id;

    if (!conn.db().execute(sql.str())) {
        throw std::runtime_error("Failed to update order");
    }
}

void OrderRepository::updateStatus(int id, const std::string& status) {
    std::ostringstream sql;
    sql << "UPDATE orders SET status = '" << escapeSql(status) << "' WHERE id = " << id;

    if (!conn.db().execute(sql.str())) {
        throw std::runtime_error("Failed to update order status");
    }
}

void OrderRepository::deleteById(int id) {
    std::ostringstream sql;
    sql << "DELETE FROM orders WHERE id = " << id;

    if (!conn.db().execute(sql.str())) {
        throw std::runtime_error("Failed to delete order");
    }
}

std::vector<OrderItem> OrderRepository::getOrderItems(int order_id) {
    std::vector<OrderItem> items;
    std::ostringstream sql;
    sql << "SELECT id, order_id, product_id, quantity, price_at_purchase, created_at "
        << "FROM order_items WHERE order_id = " << order_id;

    for (auto& row : conn.db().query(sql.str())) {
        OrderItem item{};
        if (row.count("id")) item.id = std::stoi(row.at("id"));
        if (row.count("order_id")) item.order_id = std::stoi(row.at("order_id"));
        if (row.count("product_id")) item.product_id = std::stoi(row.at("product_id"));
        if (row.count("quantity")) item.quantity = std::stoi(row.at("quantity"));
        if (row.count("price_at_purchase")) item.price_at_purchase = std::stod(row.at("price_at_purchase"));
        items.push_back(std::move(item));
    }
    return items;
}

OrderItem OrderRepository::addOrderItem(const OrderItem& item) {
    std::ostringstream sql;
    sql << "INSERT INTO order_items(order_id, product_id, quantity, price_at_purchase) VALUES("
        << item.order_id << ", "
        << item.product_id << ", "
        << item.quantity << ", "
        << item.price_at_purchase << ")";

    if (!conn.db().execute(sql.str())) {
        throw std::runtime_error("Failed to insert order item");
    }

    // Get generated ID
    auto rows = conn.db().query("SELECT last_insert_rowid() as id");
    if (rows.empty()) {
        throw std::runtime_error("Failed to get inserted order item ID");
    }

    OrderItem created = item;
    created.id = std::stoi(rows[0].at("id"));
    return created;
}

void OrderRepository::deleteOrderItems(int order_id) {
    std::ostringstream sql;
    sql << "DELETE FROM order_items WHERE order_id = " << order_id;

    if (!conn.db().execute(sql.str())) {
        throw std::runtime_error("Failed to delete order items");
    }
}

std::vector<Order> OrderRepository::getByStatus(const std::string& status) {
    std::vector<Order> orders;
    std::ostringstream sql;
    sql << "SELECT id, user_id, total_amount, status, shipping_address, "
        << "notes, created_at, updated_at FROM orders WHERE status = '"
        << escapeSql(status) << "' ORDER BY created_at DESC";

    for (auto& row : conn.db().query(sql.str())) {
        Order o{};
        if (row.count("id")) o.id = std::stoi(row.at("id"));
        if (row.count("user_id")) o.user_id = std::stoi(row.at("user_id"));
        if (row.count("total_amount")) o.total_amount = std::stod(row.at("total_amount"));
        if (row.count("status")) o.status = row.at("status");
        if (row.count("shipping_address")) o.shipping_address = row.at("shipping_address");
        if (row.count("notes")) o.notes = row.at("notes");
        orders.push_back(std::move(o));
    }
    return orders;
}

int OrderRepository::getTotalOrderCount() {
    std::string sql = "SELECT COUNT(*) as cnt FROM orders";
    auto rows = conn.db().query(sql);
    if (rows.empty()) return 0;
    return std::stoi(rows[0].at("cnt"));
}

double OrderRepository::getTotalRevenue() {
    std::string sql = "SELECT COALESCE(SUM(total_amount), 0.0) as revenue FROM orders "
                      "WHERE status != 'cancelled'";
    auto rows = conn.db().query(sql);
    if (rows.empty()) return 0.0;
    return std::stod(rows[0].at("revenue"));
}

std::optional<Order> OrderRepository::getOrderWithItems(int id) {
    auto orderOpt = getById(id);
    if (!orderOpt.has_value()) {
        return std::nullopt;
    }

    Order order = orderOpt.value();
    order.items = getOrderItems(id);
    return order;
}

std::vector<Order> OrderRepository::getUserOrdersWithItems(int user_id) {
    std::vector<Order> orders = getByUserId(user_id);
    
    // Load items for each order
    for (auto& order : orders) {
        order.items = getOrderItems(order.id);
    }
    
    return orders;
}
