-- ================================================================
-- REST API E-COMMERCE DATABASE SCHEMA
-- Production-Ready SQLite Database
-- ================================================================

-- Drop existing tables if they exist (for fresh start)
DROP TABLE IF EXISTS api_logs;
DROP TABLE IF EXISTS sessions;
DROP TABLE IF EXISTS order_items;
DROP TABLE IF EXISTS orders;
DROP TABLE IF EXISTS products;
DROP TABLE IF EXISTS users;

-- ================================================================
-- USERS TABLE
-- Manages user accounts with authentication and authorization
-- ================================================================
CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    email TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,                    -- Hashed password (bcrypt/SHA256)
    role TEXT DEFAULT 'user' CHECK(role IN ('admin', 'user', 'guest')),
    is_active BOOLEAN DEFAULT 1,                    -- Account status
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_login TIMESTAMP
);

-- Indexes for users
CREATE INDEX idx_users_email ON users(email);
CREATE INDEX idx_users_username ON users(username);
CREATE INDEX idx_users_role ON users(role);

-- ================================================================
-- PRODUCTS TABLE
-- Product catalog with inventory tracking
-- ================================================================
CREATE TABLE products (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sku TEXT UNIQUE NOT NULL,                       -- Stock Keeping Unit
    name TEXT NOT NULL,
    description TEXT,
    price REAL NOT NULL CHECK(price >= 0),          -- Price in currency units
    stock_quantity INTEGER DEFAULT 0 CHECK(stock_quantity >= 0),
    category TEXT,                                  -- Product category
    image_url TEXT,                                 -- Product image path
    is_active BOOLEAN DEFAULT 1,                    -- Product visibility
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_by INTEGER REFERENCES users(id) ON DELETE SET NULL
);

-- Indexes for products
CREATE INDEX idx_products_sku ON products(sku);
CREATE INDEX idx_products_category ON products(category);
CREATE INDEX idx_products_active ON products(is_active);
CREATE INDEX idx_products_created_by ON products(created_by);

-- ================================================================
-- ORDERS TABLE
-- Customer orders with status tracking
-- ================================================================
CREATE TABLE orders (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    total_amount REAL NOT NULL CHECK(total_amount >= 0),
    status TEXT DEFAULT 'pending' CHECK(status IN (
        'pending', 'processing', 'shipped', 'delivered', 'cancelled'
    )),
    shipping_address TEXT,
    notes TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Indexes for orders
CREATE INDEX idx_orders_user_id ON orders(user_id);
CREATE INDEX idx_orders_status ON orders(status);
CREATE INDEX idx_orders_created_at ON orders(created_at);

-- ================================================================
-- ORDER_ITEMS TABLE
-- Individual items within orders
-- ================================================================
CREATE TABLE order_items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    order_id INTEGER NOT NULL REFERENCES orders(id) ON DELETE CASCADE,
    product_id INTEGER NOT NULL REFERENCES products(id) ON DELETE RESTRICT,
    quantity INTEGER NOT NULL CHECK(quantity > 0),
    price_at_purchase REAL NOT NULL CHECK(price_at_purchase >= 0),  -- Price when ordered
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Indexes for order_items
CREATE INDEX idx_order_items_order_id ON order_items(order_id);
CREATE INDEX idx_order_items_product_id ON order_items(product_id);

-- ================================================================
-- SESSIONS TABLE
-- JWT session management and token tracking
-- ================================================================
CREATE TABLE sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    token TEXT UNIQUE NOT NULL,                     -- JWT token
    refresh_token TEXT UNIQUE,                      -- Refresh token
    expires_at TIMESTAMP NOT NULL,
    refresh_expires_at TIMESTAMP,
    ip_address TEXT,
    user_agent TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Indexes for sessions
CREATE INDEX idx_sessions_user_id ON sessions(user_id);
CREATE INDEX idx_sessions_token ON sessions(token);
CREATE INDEX idx_sessions_expires_at ON sessions(expires_at);

-- ================================================================
-- API_LOGS TABLE
-- Request logging and analytics
-- ================================================================
CREATE TABLE api_logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    method TEXT NOT NULL,                           -- HTTP method
    path TEXT NOT NULL,                             -- Request path
    status_code INTEGER,                            -- Response status
    response_time_ms INTEGER,                       -- Response time in milliseconds
    user_id INTEGER REFERENCES users(id) ON DELETE SET NULL,
    ip_address TEXT,
    user_agent TEXT,
    error_message TEXT,                             -- Error details if any
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Indexes for api_logs
CREATE INDEX idx_api_logs_timestamp ON api_logs(timestamp);
CREATE INDEX idx_api_logs_user_id ON api_logs(user_id);
CREATE INDEX idx_api_logs_status_code ON api_logs(status_code);
CREATE INDEX idx_api_logs_path ON api_logs(path);

-- ================================================================
-- TRIGGERS
-- Automatic timestamp updates
-- ================================================================

-- Update users.updated_at on modification
CREATE TRIGGER update_users_timestamp
AFTER UPDATE ON users
FOR EACH ROW
BEGIN
    UPDATE users SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

-- Update products.updated_at on modification
CREATE TRIGGER update_products_timestamp
AFTER UPDATE ON products
FOR EACH ROW
BEGIN
    UPDATE products SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

-- Update orders.updated_at on modification
CREATE TRIGGER update_orders_timestamp
AFTER UPDATE ON orders
FOR EACH ROW
BEGIN
    UPDATE orders SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

-- ================================================================
-- INITIAL DATA (Default Admin User)
-- ================================================================

-- Default admin user (password: admin123 - CHANGE IN PRODUCTION!)
-- Password hash is SHA256("admin123")
INSERT INTO users (username, email, password_hash, role) VALUES
('admin', 'admin@example.com', '240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9', 'admin');

-- ================================================================
-- VIEWS
-- Useful views for common queries
-- ================================================================

-- Order summary view
CREATE VIEW order_summary AS
SELECT
    o.id as order_id,
    o.user_id,
    u.username,
    u.email,
    o.status,
    o.total_amount,
    COUNT(oi.id) as item_count,
    o.created_at,
    o.updated_at
FROM orders o
JOIN users u ON o.user_id = u.id
LEFT JOIN order_items oi ON o.id = oi.order_id
GROUP BY o.id;

-- Product inventory view
CREATE VIEW product_inventory AS
SELECT
    p.id,
    p.sku,
    p.name,
    p.category,
    p.price,
    p.stock_quantity,
    CASE
        WHEN p.stock_quantity = 0 THEN 'out_of_stock'
        WHEN p.stock_quantity < 10 THEN 'low_stock'
        ELSE 'in_stock'
    END as stock_status,
    p.is_active
FROM products p;

-- User statistics view
CREATE VIEW user_statistics AS
SELECT
    u.id,
    u.username,
    u.email,
    u.role,
    COUNT(DISTINCT o.id) as total_orders,
    COALESCE(SUM(o.total_amount), 0) as total_spent,
    u.created_at,
    u.last_login
FROM users u
LEFT JOIN orders o ON u.id = o.user_id
GROUP BY u.id;

-- ================================================================
-- INDEXES FOR PERFORMANCE
-- Additional composite indexes for complex queries
-- ================================================================

CREATE INDEX idx_products_active_category ON products(is_active, category);
CREATE INDEX idx_orders_user_status ON orders(user_id, status);
CREATE INDEX idx_sessions_user_token ON sessions(user_id, token);

-- ================================================================
-- DATABASE INFORMATION
-- ================================================================

-- Print database schema version
-- Version: 1.0.0
-- Description: E-commerce REST API Database
-- Created: 2025
-- ================================================================
