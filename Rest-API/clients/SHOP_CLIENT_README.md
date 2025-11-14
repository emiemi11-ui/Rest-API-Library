# E-Commerce Shop Client - User Guide

## Overview
Interactive CLI client for browsing products, managing shopping cart, and placing orders through the REST API server. Can connect to servers running on different machines in the network.

## Building

```bash
cd /home/user/Rest-API-Library/Rest-API
mkdir -p build && cd build
cmake ..
make shop_client
```

## Usage

### Basic Usage (Local Server)
```bash
./shop_client 127.0.0.1 8080
```

### Network Usage (Remote Server)
```bash
./shop_client 192.168.1.100 8080
./shop_client 10.0.0.5 8080
```

### Default (if no arguments provided)
```bash
./shop_client
# Uses: 127.0.0.1:8080
```

## Features

### 1. Browse Products
- Paginated product listing
- Customizable page size
- Formatted table display with colors
- View product details from list

### 2. Search Products
- Search by keyword
- Real-time results
- Product details available

### 3. Filter by Category
- Filter products by category name
- Display filtered results
- Quick access to product details

### 4. Shopping Cart (In-Memory)
- Add products to cart
- View cart with subtotals
- Remove items
- Clear entire cart
- Persistent during session

### 5. Checkout
- Create orders from cart
- Add shipping address
- Automatic order placement
- Order confirmation with ID

### 6. My Orders
- View all user orders
- Order status tracking
- Detailed order information
- Order items listing

### 7. Authentication
- User registration
- User login
- Session management
- Auto-login after registration

## User Interface

### Color Coding
- **Green**: Success messages, prices, totals
- **Red**: Error messages, failures
- **Yellow**: Warnings, info messages
- **Cyan**: Prompts, headers
- **Magenta**: Cart item count
- **Bold**: Headers, important information

### Menu Structure
```
Main Menu:
  1. Browse Products
  2. Search Products
  3. Filter by Category
  4. View Cart (X items)
  5. Checkout
  6. My Orders
  7. Login/Register
  8. Exit
```

## Example Workflow

### First Time User
1. **Register/Login** (Option 7)
   - Create new account or login

2. **Browse Products** (Option 1)
   - View available products
   - Enter product ID to see details
   - Add to cart

3. **View Cart** (Option 4)
   - Review items
   - Modify quantities or remove items

4. **Checkout** (Option 5)
   - Enter shipping address
   - Confirm order

5. **My Orders** (Option 6)
   - Track order status
   - View order details

## Network Configuration

### Server Requirements
- REST API server must be running
- Server must be accessible from client machine
- Default port: 8080 (configurable)

### Firewall Configuration
If connecting from another machine, ensure:
```bash
# On server machine - allow incoming connections on port 8080
sudo ufw allow 8080/tcp
```

### Find Server IP
```bash
# On server machine
ip addr show | grep "inet "
# or
hostname -I
```

## Troubleshooting

### Cannot Connect to Server
```
[ERROR] Failed to connect to 192.168.1.100:8080
```
**Solutions:**
- Verify server is running
- Check IP address and port
- Verify network connectivity: `ping 192.168.1.100`
- Check firewall rules

### Login Required
```
[WARNING] Please login first to checkout
```
**Solution:**
- Use option 7 to login or register

### Cart is Empty
```
[WARNING] Cart is empty. Add products first!
```
**Solution:**
- Browse or search products (options 1-3)
- View product details and add to cart

## Technical Details

### HTTP Communication
- Uses plain TCP sockets
- HTTP/1.1 protocol
- Keep-alive connections
- Automatic reconnection on failure

### JSON Parsing
- Manual JSON extraction
- No external dependencies
- Handles nested objects and arrays

### Session Management
- In-memory user_id storage
- Cart persists during session
- Cleared on exit

### Dependencies
- Standard C++ libraries only
- POSIX sockets (Linux/Unix)
- No external libraries required

## Advanced Usage

### Testing on Local Network
1. **Start server on machine A:**
   ```bash
   ./rest_api
   ```

2. **Find IP of machine A:**
   ```bash
   hostname -I
   # Example output: 192.168.1.100
   ```

3. **Connect from machine B:**
   ```bash
   ./shop_client 192.168.1.100 8080
   ```

### Multiple Clients
- Multiple clients can connect simultaneously
- Each maintains independent cart
- Orders are user-specific after login

## Exit
- Use option 8 to exit gracefully
- Cart data is lost on exit
- User must checkout before exiting to save orders
