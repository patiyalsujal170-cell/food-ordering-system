# Food Ordering Management System

A menu-driven console application written in **C++11** using **Object-Oriented Programming**.
Developed as a BCA Data Science (Hons) Semester 3 OOP mini project.

> This is an **offline educational project**. Cash/UPI are demonstration options only.
> There is no online payment integration and no real-time delivery tracking.

## Introduction

The system lets a small restaurant manage its food menu and customers, take orders through a
cart, produce itemised bills, record payments, track order status and view simple reports.
All records are stored in plain text files, so data is still available after the program restarts.

## Objectives

- Apply core OOP concepts in a practical, real-world style project.
- Practise file handling, input validation and exception handling in C++.
- Build a clean, readable project that a beginner can explain in a viva.

## Features

**Food menu management** - display by category, search by name or ID, add, update name/price, delete, mark Available/Unavailable.

**Customer management** - register customers (unique auto-generated ID), display, search by ID or name; stores name and contact number.

**Ordering** - choose a customer, view menu, add multiple items with quantities, change quantity, remove items, view cart, confirm or cancel. Unavailable items and zero/negative quantities are rejected.

**Billing and payments** - itemised bill with quantity, unit price, subtotal, GST (5%) and final payable amount; Cash/UPI demo methods; Paid/Pending status.

**Order management** - list orders, search by ID, view details, update status (Placed, Preparing, Ready, Delivered, Cancelled), customer order history.

**Reports** - total orders, revenue from paid (non-cancelled) orders, pending orders, most ordered items, daily order summary.

## Technologies Used

| Item | Details |
|------|---------|
| Language | C++ (C++11 standard) |
| Compiler | g++ |
| Interface | Console, menu-driven |
| Storage | Plain text files (no SQL, no external libraries) |

## OOP Concepts Implemented

Every concept below is implemented in `main.cpp`.

| Concept | Where it is used |
|---------|------------------|
| Classes and objects | `FoodItem`, `Customer`, `OrderItem`, `Order`, `Payment`, `FoodMenu`, `CustomerManager`, `OrderManager`, `FoodOrderingSystem` |
| Encapsulation | Private data members with getters/setters; setters validate (e.g. `FoodItem::setPrice`, `OrderItem::setQuantity`) |
| Abstraction | Abstract class `Storable` (pure virtual `toRecord()` and `display()`); `Person` is also abstract |
| Inheritance | `Storable` -> `FoodItem`, `Person`, `Order`, `Payment`; `Person` -> `Customer` |
| Polymorphism | `writeRecords()` saves any `Storable*` via virtual `toRecord()`; `getRole()` overridden in `Customer` |
| Function overloading | `FoodMenu::search(int)` / `search(const string&)`; `FoodMenu::updateItem(int, string)` / `updateItem(int, double)`; `Order::addItem(FoodItem, int)` / `addItem(OrderItem)` |
| Constructor overloading | `FoodItem` (3 constructors), `Customer` (3), `Payment` (3) |
| Static data members | `Customer::nextNumber`, `Order::nextNumber`, `Payment::nextNumber` (ID counters), `Order::TAX_RATE` |
| Pointers | `FoodMenu::findById()` returns `FoodItem*`; `OrderManager` holds `CustomerManager*`; `vector<const Storable*>` in saving |
| Composition | `Order` contains its `OrderItem`s; `FoodOrderingSystem` owns `FoodMenu`, `CustomerManager`, `OrderManager` |
| Aggregation | `OrderManager` uses a `CustomerManager*` that it does not own |
| Scope resolution operator | Member functions defined outside classes (`FoodItem::display`), static members (`Order::TAX_RATE`) |
| File handling | `ifstream` / `ofstream` in `readLines()` and `writeRecords()`; automatic save and load |
| Exception handling | `try/catch` around every menu action; `invalid_argument`, `logic_error`, `runtime_error` and the custom `InputClosedException` |

## Folder Structure

```
Food-Ordering-System/
|
|-- main.cpp
|-- README.md
|-- .gitignore
|
|-- data/
|   |-- food_items.txt
|   |-- customers.txt
|   |-- orders.txt
|   `-- payments.txt
|
`-- docs/
    `-- PROJECT_OVERVIEW.md
```

## Compilation and Execution

Open a terminal **in the project folder** (the program reads `data/...` using relative paths).

```bash
g++ -std=c++11 main.cpp -o food_ordering
```

Run:

- Linux / macOS: `./food_ordering`
- Windows: `food_ordering.exe`

## Sample Menu (fictional data)

| ID | Item | Category | Price (Rs.) | Status |
|----|------|----------|-------------|--------|
| 101 | Paneer Tikka | Starters | 180.00 | Available |
| 102 | Veg Spring Rolls | Starters | 120.00 | Available |
| 201 | Margherita Pizza | Main Course | 249.00 | Available |
| 202 | White Sauce Pasta | Main Course | 199.00 | Available |
| 301 | Veg Burger | Snacks | 99.00 | Available |
| 302 | French Fries | Snacks | 79.00 | Available |
| 303 | Sandwich | Snacks | 89.00 | Available |
| 401 | Cold Coffee | Beverages | 99.00 | Available |
| 402 | Masala Lemonade | Beverages | 59.00 | Available |
| 501 | Chocolate Brownie | Desserts | 109.00 | Available |
| 502 | Vanilla Ice Cream | Desserts | 69.00 | Unavailable |

Sample customers: `C001 Aarav Mehta`, `C002 Simran Kaur`, `C003 Rohan Verma` (all fictional). Four sample orders are included.

## File Storage Explanation

Each record is **one line**; fields are separated by `|`. Inside a field, `|`, `\` and line breaks are saved as `\|`, `\\` and `\n`, so special characters never break a record.

| File | Format |
|------|--------|
| `food_items.txt` | `FoodID\|Name\|Category\|Price\|Available or Unavailable` |
| `customers.txt` | `CustomerID\|Name\|ContactNumber` |
| `orders.txt` | `OrderID\|CustomerID\|Date\|Total\|Status\|ItemCount\|` then for each item `FoodID\|Name\|Quantity\|UnitPrice` |
| `payments.txt` | `PaymentID\|OrderID\|Method\|Amount\|Paid or Pending` |

Example order line (2 items):

```
O1001|C001|2026-10-28|290.85|Delivered|2|301|Veg Burger|2|99.00|302|French Fries|1|79.00
```

Notes:
- Files are loaded on start-up and saved automatically after every change.
- A missing or empty file is treated as "no records". Corrupt lines are skipped with a warning.
- The `data/` folder must exist (it is included in the repository).
- Food names and prices are copied into each order, so old orders stay correct if the menu changes.

## Business Rules

- GST of 5% is added to the subtotal (demonstration value, `Order::TAX_RATE`).
- Revenue counts only **Paid** payments of orders that are **not Cancelled**.
- "Pending orders" are non-cancelled orders whose payment is still Pending.
- Delivered and Cancelled orders cannot be changed again.
- Quantity must be 1 to 100 per item; unavailable items cannot be ordered.

## Testing Checklist

Tick each item after you have tested it on your own machine.

- [ ] Add a food item (and reject a duplicate ID)
- [ ] Register a customer (and reject an invalid contact number)
- [ ] Search food by ID and by name
- [ ] Place an order with multiple items
- [ ] Reject zero and negative quantities
- [ ] Reject an unavailable food item
- [ ] Update cart quantity
- [ ] Remove a cart item
- [ ] Check bill calculation (subtotal, GST, final amount)
- [ ] Cancel an order and confirm it is excluded from revenue
- [ ] Update order status
- [ ] Close and restart the program; confirm records are still there
- [ ] Generate all reports

Detailed steps and expected results are in [`docs/PROJECT_OVERVIEW.md`](docs/PROJECT_OVERVIEW.md).

## Limitations

- Single-user console program; no login or roles.
- One payment record per order; no refunds (a cancelled order is simply excluded from revenue).
- Data files are plain text and can be edited by hand.
- The `data/` folder must already exist and the program must be run from the project folder.
- No real payment gateway and no delivery tracking.

## Future Enhancements

- Admin/customer login
- Discount and coupon support
- Refund handling for cancelled paid orders
- Export bills to a text file
- Unit tests and a Makefile
- Graphical interface

## Author Details

- **Name:** `<Your Name>`
- **UID:** `<Your UID>`
- **Course:** BCA Data Science (Hons), Semester 3
- **University:** `<Your University>`
- **Submitted to:** `<Faculty Name>`
- **GitHub:** `<Your GitHub Profile / Repository Link>`
