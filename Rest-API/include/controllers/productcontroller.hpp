#pragma once
#include "services/productservice.hpp"
#include "http/request.hpp"
#include <string>
#include <map>

class ProductController {
private:
    ProductService& service;

    // Helper to extract body from request
    std::string extractBody(const std::string& raw_request);

    // Helper to build HTTP JSON response
    std::string jsonResponse(int status, const std::string& body);

    // Helper to extract query parameters
    std::string getQueryParam(const HttpRequest& req, const std::string& param, const std::string& defaultValue = "");

public:
    explicit ProductController(ProductService& service) : service(service) {}

    // GET /api/products - Get all products (with pagination/filtering)
    std::string getAll(const HttpRequest& req, const std::map<std::string, std::string>& params);

    // GET /api/products/:id - Get product by ID
    std::string getById(const HttpRequest& req, const std::map<std::string, std::string>& params);

    // GET /api/products/search?q=keyword - Search products
    std::string search(const HttpRequest& req, const std::map<std::string, std::string>& params);

    // GET /api/products/category/:category - Get products by category
    std::string getByCategory(const HttpRequest& req, const std::map<std::string, std::string>& params);

    // GET /api/products/low-stock - Get low stock products
    std::string getLowStock(const HttpRequest& req, const std::map<std::string, std::string>& params);

    // GET /api/products/active - Get active products
    std::string getActive(const HttpRequest& req, const std::map<std::string, std::string>& params);

    // POST /api/products - Create new product (admin only)
    std::string create(const HttpRequest& req, const std::map<std::string, std::string>& params);

    // PUT /api/products/:id - Update product (admin only)
    std::string update(const HttpRequest& req, const std::map<std::string, std::string>& params);

    // PATCH /api/products/:id/stock - Update product stock
    std::string updateStock(const HttpRequest& req, const std::map<std::string, std::string>& params);

    // DELETE /api/products/:id - Delete product (admin only)
    std::string remove(const HttpRequest& req, const std::map<std::string, std::string>& params);

    // Set raw request for parsing body
    void setRawRequest(const std::string& raw);

private:
    std::string raw_request;
};
