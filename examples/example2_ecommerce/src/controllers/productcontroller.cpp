#include "controllers/productcontroller.hpp"
#include <iostream>
#include <sstream>

void ProductController::setRawRequest(const std::string& raw) {
    raw_request = raw;
}

std::string ProductController::extractBody(const std::string& raw_req) {
    // Find body after \r\n\r\n
    size_t pos = raw_req.find("\r\n\r\n");
    if (pos == std::string::npos) {
        return "";
    }
    return raw_req.substr(pos + 4);
}

std::string ProductController::jsonResponse(int status, const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << status;

    switch (status) {
        case 200: response << " OK"; break;
        case 201: response << " Created"; break;
        case 204: response << " No Content"; break;
        case 400: response << " Bad Request"; break;
        case 401: response << " Unauthorized"; break;
        case 404: response << " Not Found"; break;
        case 500: response << " Internal Server Error"; break;
        default: response << " Unknown"; break;
    }

    response << "\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}

std::string ProductController::getQueryParam(const HttpRequest& req, const std::string& param, const std::string& defaultValue) {
    // Extract query parameters from target
    size_t query_pos = req.target.find('?');
    if (query_pos == std::string::npos) {
        return defaultValue;
    }

    std::string query = req.target.substr(query_pos + 1);
    std::string search = param + "=";
    size_t param_pos = query.find(search);

    if (param_pos == std::string::npos) {
        return defaultValue;
    }

    param_pos += search.length();
    size_t end_pos = query.find('&', param_pos);

    if (end_pos == std::string::npos) {
        return query.substr(param_pos);
    }

    return query.substr(param_pos, end_pos - param_pos);
}

std::string ProductController::getAll(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[ProductController] GET /api/products\n";

    try {
        // Check for query parameters
        std::string page_str = getQueryParam(req, "page", "0");
        std::string limit_str = getQueryParam(req, "limit", "0");
        std::string sort = getQueryParam(req, "sort", "id");
        std::string category = getQueryParam(req, "category", "");

        std::vector<Product> products;

        // Filter by category if specified
        if (!category.empty()) {
            products = service.getProductsByCategory(category);
        }
        // Paginated results if page and limit are specified
        else if (page_str != "0" && limit_str != "0") {
            int page = std::stoi(page_str);
            int limit = std::stoi(limit_str);
            products = service.getProductsPaginated(page, limit, sort);
        }
        // Otherwise return all products
        else {
            products = service.getAllProducts();
        }

        // Build JSON array
        std::ostringstream json;
        json << "{\"products\":[";
        for (size_t i = 0; i < products.size(); ++i) {
            json << products[i].toJson();
            if (i < products.size() - 1) {
                json << ",";
            }
        }
        json << "],\"total\":" << service.getTotalProductCount() << "}";

        return jsonResponse(200, json.str());
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

std::string ProductController::getById(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[ProductController] GET /api/products/:id\n";

    try {
        auto it = params.find("id");
        if (it == params.end()) {
            return jsonResponse(400, "{\"error\":\"Missing ID\"}");
        }

        int id = std::stoi(it->second);
        auto product = service.getProduct(id);

        if (!product.has_value()) {
            std::ostringstream error;
            error << "{\"error\":\"Product with ID=" << id << " not found\"}";
            return jsonResponse(404, error.str());
        }

        return jsonResponse(200, product->toJson());
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

std::string ProductController::search(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[ProductController] GET /api/products/search\n";

    try {
        std::string keyword = getQueryParam(req, "q", "");

        if (keyword.empty()) {
            return jsonResponse(400, "{\"error\":\"Missing search keyword (q parameter)\"}");
        }

        auto products = service.searchProducts(keyword);

        // Build JSON array
        std::ostringstream json;
        json << "{\"products\":[";
        for (size_t i = 0; i < products.size(); ++i) {
            json << products[i].toJson();
            if (i < products.size() - 1) {
                json << ",";
            }
        }
        json << "],\"count\":" << products.size() << "}";

        return jsonResponse(200, json.str());
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

std::string ProductController::getByCategory(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[ProductController] GET /api/products/category/:category\n";

    try {
        auto it = params.find("category");
        if (it == params.end()) {
            return jsonResponse(400, "{\"error\":\"Missing category\"}");
        }

        std::string category = it->second;
        auto products = service.getProductsByCategory(category);

        // Build JSON array
        std::ostringstream json;
        json << "{\"products\":[";
        for (size_t i = 0; i < products.size(); ++i) {
            json << products[i].toJson();
            if (i < products.size() - 1) {
                json << ",";
            }
        }
        json << "],\"category\":\"" << category << "\",\"count\":" << products.size() << "}";

        return jsonResponse(200, json.str());
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

std::string ProductController::getLowStock(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[ProductController] GET /api/products/low-stock\n";

    try {
        std::string threshold_str = getQueryParam(req, "threshold", "10");
        int threshold = std::stoi(threshold_str);

        auto products = service.getLowStock(threshold);

        // Build JSON array
        std::ostringstream json;
        json << "{\"products\":[";
        for (size_t i = 0; i < products.size(); ++i) {
            json << products[i].toJson();
            if (i < products.size() - 1) {
                json << ",";
            }
        }
        json << "],\"threshold\":" << threshold << ",\"count\":" << products.size() << "}";

        return jsonResponse(200, json.str());
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

std::string ProductController::getActive(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[ProductController] GET /api/products/active\n";

    try {
        auto products = service.getActiveProducts();

        // Build JSON array
        std::ostringstream json;
        json << "{\"products\":[";
        for (size_t i = 0; i < products.size(); ++i) {
            json << products[i].toJson();
            if (i < products.size() - 1) {
                json << ",";
            }
        }
        json << "],\"count\":" << products.size() << "}";

        return jsonResponse(200, json.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

std::string ProductController::create(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[ProductController] POST /api/products\n";

    try {
        // Extract body
        std::string body = extractBody(raw_request);
        std::cout << "[ProductController] Body: " << body << "\n";

        if (body.empty()) {
            return jsonResponse(400, "{\"error\":\"Missing body\"}");
        }

        // Parse JSON
        Product product = Product::fromJson(body);

        // Create product
        Product created = service.createProduct(product);

        return jsonResponse(201, created.toJson());
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

std::string ProductController::update(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[ProductController] PUT /api/products/:id\n";

    try {
        // Extract ID
        auto it = params.find("id");
        if (it == params.end()) {
            return jsonResponse(400, "{\"error\":\"Missing ID\"}");
        }
        int id = std::stoi(it->second);

        // Extract body
        std::string body = extractBody(raw_request);
        if (body.empty()) {
            return jsonResponse(400, "{\"error\":\"Missing body\"}");
        }

        // Parse JSON
        Product product = Product::fromJson(body);

        // Update
        service.updateProduct(id, product);

        // Get updated product
        auto updated = service.getProduct(id);
        if (updated.has_value()) {
            return jsonResponse(200, updated->toJson());
        } else {
            return jsonResponse(200, "{\"message\":\"Product updated successfully\"}");
        }
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

std::string ProductController::updateStock(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[ProductController] PATCH /api/products/:id/stock\n";

    try {
        // Extract ID
        auto it = params.find("id");
        if (it == params.end()) {
            return jsonResponse(400, "{\"error\":\"Missing ID\"}");
        }
        int id = std::stoi(it->second);

        // Extract body
        std::string body = extractBody(raw_request);
        if (body.empty()) {
            return jsonResponse(400, "{\"error\":\"Missing body\"}");
        }

        // Extract stock_quantity from JSON
        std::string search = "\"stock_quantity\":";
        size_t pos = body.find(search);
        if (pos == std::string::npos) {
            return jsonResponse(400, "{\"error\":\"Missing stock_quantity in body\"}");
        }

        pos += search.length();
        // Skip whitespace
        while (pos < body.length() && std::isspace(body[pos])) pos++;
        int quantity = std::stoi(body.substr(pos));

        // Update stock
        service.updateStock(id, quantity);

        // Get updated product
        auto updated = service.getProduct(id);
        if (updated.has_value()) {
            return jsonResponse(200, updated->toJson());
        } else {
            return jsonResponse(200, "{\"message\":\"Stock updated successfully\"}");
        }
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

std::string ProductController::remove(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[ProductController] DELETE /api/products/:id\n";

    try {
        // Extract ID
        auto it = params.find("id");
        if (it == params.end()) {
            return jsonResponse(400, "{\"error\":\"Missing ID\"}");
        }
        int id = std::stoi(it->second);

        // Delete
        service.deleteProduct(id);

        return jsonResponse(200, "{\"message\":\"Product deleted successfully\"}");
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}
