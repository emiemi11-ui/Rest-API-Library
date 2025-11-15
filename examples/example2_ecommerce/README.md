# Example 2: E-Commerce REST API

Exemplu complet de API REST pentru un magazin online (e-commerce) construit folosind REST API Framework.

## 📋 Caracteristici

- **Gestionare Produse**: CRUD complet pentru produse
- **Categorii**: Filtrare produse după categorie
- **Căutare**: Sistem de căutare în produse
- **Stocuri**: Gestionare automată a stocurilor
- **Bază de date**: SQLite cu schema completă
- **Validare**: Validare completă a datelor

## 🗂️ Structura Proiectului

```
example2_ecommerce/
├── include/
│   ├── models/          # Modele de date (Product, Order, User)
│   ├── controllers/     # Controllere pentru endpoint-uri
│   ├── services/        # Logica de business
│   └── data/           # Repository pattern pentru baza de date
├── src/
│   ├── controllers/    # Implementări controllere
│   ├── services/       # Implementări servicii
│   └── data/          # Implementări repositories
├── main.cpp           # Aplicația principală
└── schema.sql         # Schema bazei de date

```

## 🏗️ Arhitectură

Proiectul folosește o arhitectură în straturi (layered architecture):

1. **Controllers**: Gestionează cererile HTTP și răspunsurile
2. **Services**: Conțin logica de business și validări
3. **Repositories**: Accesează baza de date
4. **Models**: Definesc structurile de date

## 🚀 Compilare și Rulare

### Compilare din rădăcina proiectului:

```bash
mkdir build
cd build
cmake ..
make example2_ecommerce
```

### Rulare:

```bash
./example2_ecommerce
```

Serverul va porni pe `http://localhost:8080`

## 📡 API Endpoints

### Informații Generale

- `GET /` - Informații despre API
- `GET /health` - Status server

### Produse

- `GET /api/products` - Lista tuturor produselor
- `GET /api/products/:id` - Detalii produs specific
- `POST /api/products` - Creează produs nou
- `PUT /api/products/:id` - Actualizează produs
- `DELETE /api/products/:id` - Șterge produs
- `GET /api/products/category/:category` - Produse dintr-o categorie
- `GET /api/products/search/:keyword` - Caută produse
- `GET /api/products/active` - Produse active
- `PATCH /api/products/:id/stock` - Actualizează stocul

## 💡 Exemple de Utilizare

### Obține toate produsele:
```bash
curl http://localhost:8080/api/products
```

### Obține un produs specific:
```bash
curl http://localhost:8080/api/products/1
```

### Creează un produs nou:
```bash
curl -X POST http://localhost:8080/api/products \
  -H "Content-Type: application/json" \
  -d '{
    "sku": "LAPTOP-001",
    "name": "Laptop Gaming",
    "description": "Laptop performant pentru gaming",
    "price": 4999.99,
    "stock_quantity": 10,
    "category": "Electronics",
    "is_active": true
  }'
```

### Actualizează stocul:
```bash
curl -X PATCH http://localhost:8080/api/products/1/stock \
  -H "Content-Type: application/json" \
  -d '{"quantity": 25}'
```

### Caută produse:
```bash
curl http://localhost:8080/api/products/search/laptop
```

### Produse dintr-o categorie:
```bash
curl http://localhost:8080/api/products/category/Electronics
```

## 🗄️ Baza de Date

Aplicația folosește SQLite cu următoarele tabele:

- **users** - Utilizatori și autentificare
- **products** - Catalog produse
- **orders** - Comenzi
- **order_items** - Produse din comenzi

Schema completă se găsește în `schema.sql`.

## 🔧 Configurare

Baza de date SQLite (`ecommerce.db`) va fi creată automat la prima rulare.

Pentru a inițializa baza de date cu schema:

```bash
sqlite3 ecommerce.db < schema.sql
```

## 📚 Modele de Date

### Product
```cpp
struct Product {
    int id;
    std::string sku;
    std::string name;
    std::string description;
    double price;
    int stock_quantity;
    std::string category;
    std::string image_url;
    bool is_active;
    std::time_t created_at;
    std::time_t updated_at;
    int created_by;
}
```

### Order
```cpp
struct Order {
    int id;
    int user_id;
    double total_amount;
    std::string status;
    std::string shipping_address;
    std::string notes;
    std::time_t created_at;
    std::time_t updated_at;
    std::vector<OrderItem> items;
}
```

### User
```cpp
class User {
    int id;
    std::string name;
    std::string email;
    std::string password_hash;
}
```

## 🎯 Cazuri de Utilizare

Acest exemplu demonstrează:

- ✅ Arhitectură MVC (Model-View-Controller)
- ✅ Repository Pattern
- ✅ Service Layer Pattern
- ✅ Validare date
- ✅ Gestionare erori
- ✅ Operații CRUD complete
- ✅ Filtrare și căutare
- ✅ Integrare bază de date SQLite
- ✅ JSON serialization/deserialization
- ✅ RESTful API design

## 🔜 Extinderi Posibile

- Autentificare și autorizare (JWT)
- Gestionare comenzi complete
- Sistem de review-uri produse
- Wishlist utilizatori
- Coș de cumpărături
- Procesare plăți
- Email notificări
- Upload imagini produse
- Rapoarte și statistici
