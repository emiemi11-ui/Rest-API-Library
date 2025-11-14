-- ================================================================
-- SEED DATA FOR E-COMMERCE REST API
-- Demo data for testing and demonstration
-- ================================================================

-- ================================================================
-- USERS
-- Sample users with different roles
-- ================================================================

-- Additional users (password: user123 for all)
-- Password hash: SHA256("user123")
INSERT INTO users (username, email, password_hash, role) VALUES
('john_doe', 'john@example.com', 'c888c9ce9e098d5864d3ded6ebcc140a12142263bace3a23a36f9905f12bd64a', 'user'),
('jane_smith', 'jane@example.com', 'c888c9ce9e098d5864d3ded6ebcc140a12142263bace3a23a36f9905f12bd64a', 'user'),
('bob_wilson', 'bob@example.com', 'c888c9ce9e098d5864d3ded6ebcc140a12142263bace3a23a36f9905f12bd64a', 'user'),
('alice_brown', 'alice@example.com', 'c888c9ce9e098d5864d3ded6ebcc140a12142263bace3a23a36f9905f12bd64a', 'user'),
('moderator', 'mod@example.com', 'c888c9ce9e098d5864d3ded6ebcc140a12142263bace3a23a36f9905f12bd64a', 'admin');

-- ================================================================
-- PRODUCTS
-- Sample product catalog across different categories
-- ================================================================

-- Electronics
INSERT INTO products (sku, name, description, price, stock_quantity, category, is_active, created_by) VALUES
('ELEC-001', 'Laptop Pro 15"', 'High-performance laptop with 16GB RAM and 512GB SSD', 1299.99, 15, 'Electronics', 1, 1),
('ELEC-002', 'Wireless Mouse', 'Ergonomic wireless mouse with 6 buttons', 29.99, 50, 'Electronics', 1, 1),
('ELEC-003', 'Mechanical Keyboard', 'RGB mechanical gaming keyboard', 89.99, 30, 'Electronics', 1, 1),
('ELEC-004', '27" Monitor 4K', 'Ultra HD 4K monitor with HDR support', 449.99, 20, 'Electronics', 1, 1),
('ELEC-005', 'Webcam HD', '1080p webcam with microphone', 59.99, 40, 'Electronics', 1, 1),
('ELEC-006', 'USB-C Hub', '7-in-1 USB-C hub with HDMI and ethernet', 39.99, 60, 'Electronics', 1, 1),
('ELEC-007', 'Noise Cancelling Headphones', 'Premium wireless headphones with ANC', 249.99, 25, 'Electronics', 1, 1),
('ELEC-008', 'Portable SSD 1TB', 'Fast external SSD with USB 3.2', 119.99, 35, 'Electronics', 1, 1);

-- Books
INSERT INTO products (sku, name, description, price, stock_quantity, category, is_active, created_by) VALUES
('BOOK-001', 'Clean Code', 'A Handbook of Agile Software Craftsmanship', 39.99, 100, 'Books', 1, 1),
('BOOK-002', 'Design Patterns', 'Elements of Reusable Object-Oriented Software', 49.99, 80, 'Books', 1, 1),
('BOOK-003', 'The Pragmatic Programmer', 'Your Journey To Mastery', 44.99, 90, 'Books', 1, 1),
('BOOK-004', 'Introduction to Algorithms', 'Comprehensive guide to algorithms', 89.99, 50, 'Books', 1, 1),
('BOOK-005', 'Code Complete', 'A Practical Handbook of Software Construction', 54.99, 70, 'Books', 1, 1);

-- Clothing
INSERT INTO products (sku, name, description, price, stock_quantity, category, is_active, created_by) VALUES
('CLTH-001', 'T-Shirt Classic', '100% cotton t-shirt in various colors', 19.99, 200, 'Clothing', 1, 1),
('CLTH-002', 'Jeans Slim Fit', 'Premium denim jeans', 59.99, 100, 'Clothing', 1, 1),
('CLTH-003', 'Hoodie Premium', 'Comfortable hoodie with kangaroo pocket', 49.99, 80, 'Clothing', 1, 1),
('CLTH-004', 'Sneakers Running', 'Lightweight running shoes', 79.99, 60, 'Clothing', 1, 1),
('CLTH-005', 'Winter Jacket', 'Warm winter jacket with hood', 129.99, 40, 'Clothing', 1, 1);

-- Home & Garden
INSERT INTO products (sku, name, description, price, stock_quantity, category, is_active, created_by) VALUES
('HOME-001', 'Coffee Maker', 'Programmable coffee maker 12-cup', 79.99, 45, 'Home', 1, 1),
('HOME-002', 'Blender Pro', 'High-speed blender with multiple settings', 99.99, 35, 'Home', 1, 1),
('HOME-003', 'Vacuum Cleaner', 'Cordless vacuum with HEPA filter', 249.99, 20, 'Home', 1, 1),
('HOME-004', 'Air Purifier', 'Smart air purifier with app control', 199.99, 30, 'Home', 1, 1),
('HOME-005', 'Desk Lamp LED', 'Adjustable LED desk lamp', 39.99, 70, 'Home', 1, 1);

-- Sports & Outdoors
INSERT INTO products (sku, name, description, price, stock_quantity, category, is_active, created_by) VALUES
('SPRT-001', 'Yoga Mat Premium', 'Non-slip yoga mat with carrying strap', 29.99, 100, 'Sports', 1, 1),
('SPRT-002', 'Dumbbells Set', 'Adjustable dumbbells 5-25 lbs', 149.99, 30, 'Sports', 1, 1),
('SPRT-003', 'Resistance Bands', 'Set of 5 resistance bands', 24.99, 90, 'Sports', 1, 1),
('SPRT-004', 'Water Bottle', 'Insulated water bottle 32oz', 19.99, 150, 'Sports', 1, 1),
('SPRT-005', 'Camping Tent', '4-person waterproof camping tent', 199.99, 15, 'Sports', 1, 1);

-- Low stock products (for testing inventory alerts)
INSERT INTO products (sku, name, description, price, stock_quantity, category, is_active, created_by) VALUES
('LOW-001', 'Limited Edition Watch', 'Luxury smartwatch - limited stock', 399.99, 5, 'Electronics', 1, 1),
('LOW-002', 'Rare Book Collection', 'Collector''s edition book set', 299.99, 3, 'Books', 1, 1),
('OUT-001', 'Sold Out Item', 'Currently out of stock', 99.99, 0, 'Electronics', 1, 1);

-- ================================================================
-- ORDERS
-- Sample orders with different statuses
-- ================================================================

-- John's orders
INSERT INTO orders (user_id, total_amount, status, shipping_address, notes) VALUES
(2, 1389.97, 'delivered', '123 Main St, New York, NY 10001', 'Please leave at door'),
(2, 129.97, 'shipped', '123 Main St, New York, NY 10001', NULL),
(2, 89.99, 'processing', '123 Main St, New York, NY 10001', 'Gift wrap requested');

-- Jane's orders
INSERT INTO orders (user_id, total_amount, status, shipping_address, notes) VALUES
(3, 249.99, 'delivered', '456 Oak Ave, Los Angeles, CA 90001', NULL),
(3, 449.99, 'pending', '456 Oak Ave, Los Angeles, CA 90001', 'Call before delivery');

-- Bob's orders
INSERT INTO orders (user_id, total_amount, status, shipping_address, notes) VALUES
(4, 199.98, 'delivered', '789 Pine Rd, Chicago, IL 60601', NULL),
(4, 59.99, 'cancelled', '789 Pine Rd, Chicago, IL 60601', 'Customer cancelled');

-- Alice's orders
INSERT INTO orders (user_id, total_amount, status, shipping_address, notes) VALUES
(5, 1299.99, 'processing', '321 Elm St, Houston, TX 77001', 'Signature required'),
(5, 154.96, 'delivered', '321 Elm St, Houston, TX 77001', NULL);

-- ================================================================
-- ORDER_ITEMS
-- Items for each order
-- ================================================================

-- Order 1 (John - delivered)
INSERT INTO order_items (order_id, product_id, quantity, price_at_purchase) VALUES
(1, 1, 1, 1299.99),  -- Laptop
(1, 2, 1, 29.99),    -- Mouse
(1, 3, 1, 59.99);    -- Keyboard (price changed since order)

-- Order 2 (John - shipped)
INSERT INTO order_items (order_id, product_id, quantity, price_at_purchase) VALUES
(2, 19, 1, 129.99);  -- Winter Jacket

-- Order 3 (John - processing)
INSERT INTO order_items (order_id, product_id, quantity, price_at_purchase) VALUES
(3, 4, 1, 89.99);    -- Monitor (ordered at different price)

-- Order 4 (Jane - delivered)
INSERT INTO order_items (order_id, product_id, quantity, price_at_purchase) VALUES
(4, 7, 1, 249.99);   -- Headphones

-- Order 5 (Jane - pending)
INSERT INTO order_items (order_id, product_id, quantity, price_at_purchase) VALUES
(5, 4, 1, 449.99);   -- Monitor

-- Order 6 (Bob - delivered)
INSERT INTO order_items (order_id, product_id, quantity, price_at_purchase) VALUES
(6, 9, 1, 39.99),    -- Clean Code book
(6, 10, 1, 49.99),   -- Design Patterns book
(6, 11, 1, 44.99),   -- Pragmatic Programmer
(6, 26, 2, 24.99);   -- Resistance Bands (qty: 2)

-- Order 7 (Bob - cancelled)
INSERT INTO order_items (order_id, product_id, quantity, price_at_purchase) VALUES
(7, 15, 1, 59.99);   -- Jeans

-- Order 8 (Alice - processing)
INSERT INTO order_items (order_id, product_id, quantity, price_at_purchase) VALUES
(8, 1, 1, 1299.99);  -- Laptop

-- Order 9 (Alice - delivered)
INSERT INTO order_items (order_id, product_id, quantity, price_at_purchase) VALUES
(9, 24, 1, 29.99),   -- Yoga Mat
(9, 25, 1, 24.99),   -- Dumbbells (ordered at different price)
(9, 27, 5, 19.99);   -- Water Bottles (qty: 5)

-- ================================================================
-- SESSIONS
-- Sample active sessions
-- ================================================================

INSERT INTO sessions (user_id, token, expires_at, ip_address, user_agent) VALUES
(1, 'admin_token_12345', datetime('now', '+1 day'), '192.168.1.100', 'Mozilla/5.0'),
(2, 'john_token_67890', datetime('now', '+1 day'), '192.168.1.101', 'Chrome/90.0'),
(3, 'jane_token_11111', datetime('now', '+1 day'), '192.168.1.102', 'Firefox/88.0');

-- ================================================================
-- API_LOGS
-- Sample API request logs
-- ================================================================

INSERT INTO api_logs (method, path, status_code, response_time_ms, user_id, ip_address, timestamp) VALUES
('GET', '/api/products', 200, 45, NULL, '192.168.1.100', datetime('now', '-2 hours')),
('POST', '/api/auth/login', 200, 120, 1, '192.168.1.100', datetime('now', '-2 hours')),
('GET', '/api/products/1', 200, 15, 1, '192.168.1.100', datetime('now', '-1 hour')),
('POST', '/api/orders', 201, 250, 2, '192.168.1.101', datetime('now', '-1 hour')),
('GET', '/api/users', 200, 35, 1, '192.168.1.100', datetime('now', '-30 minutes')),
('PUT', '/api/products/5', 200, 80, 1, '192.168.1.100', datetime('now', '-15 minutes')),
('GET', '/health', 200, 5, NULL, '192.168.1.105', datetime('now', '-5 minutes')),
('GET', '/api/orders/1', 200, 25, 2, '192.168.1.101', datetime('now', '-2 minutes')),
('GET', '/api/products?category=Electronics', 200, 55, NULL, '192.168.1.106', datetime('now', '-1 minute'));

-- ================================================================
-- DATABASE STATISTICS
-- ================================================================

SELECT 'Database seeding completed!' as status;
SELECT COUNT(*) as total_users FROM users;
SELECT COUNT(*) as total_products FROM products;
SELECT COUNT(*) as total_orders FROM orders;
SELECT COUNT(*) as total_order_items FROM order_items;
SELECT COUNT(*) as total_sessions FROM sessions;
SELECT COUNT(*) as total_api_logs FROM api_logs;
