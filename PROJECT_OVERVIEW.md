# Project Overview - Food Ordering Management System

An offline, menu-driven C++11 console application using OOP and plain-text file storage.
(Educational project: Cash/UPI are demonstration options only.)

## 1. Class Summary

| Class | Role |
|-------|------|
| `Storable` | Abstract base: `toRecord()` and `display()` that every saved record must provide |
| `FoodItem` | One menu item (ID, name, category, price, availability) |
| `FoodMenu` | Collection of food items: load/save, add, update, delete, search, display |
| `Person` | Abstract base for people (name, contact) |
| `Customer` | Derived from `Person`; adds auto-generated customer ID |
| `CustomerManager` | Collection of customers: register, search, load/save |
| `OrderItem` | One line of an order (copy of food ID, name, price, quantity) |
| `Order` | Cart/order with items, date, status, subtotal, GST and total |
| `Payment` | Payment record (method Cash/UPI, amount, Paid/Pending) |
| `OrderManager` | Orders and payments: confirm, status, pay, history, reports |
| `FoodOrderingSystem` | Console menus; owns `FoodMenu`, `CustomerManager`, `OrderManager` |

## 2. Class Relationships

```
Storable (abstract)
  |-- FoodItem
  |-- Order            (contains many OrderItem  -> composition)
  |-- Payment
  `-- Person (abstract)
        `-- Customer

FoodOrderingSystem --owns--> FoodMenu, CustomerManager, OrderManager   (composition)
OrderManager --uses--> CustomerManager*                                 (aggregation)
```

## 3. Program Flow

1. Start-up: load the four text files (missing/empty files are fine; bad lines are skipped with a warning).
2. Main menu: Food Menu, Customers, Place Order, Order Management, Billing, Reports, Exit.
3. Every change (add, update, delete, register, confirm order, status change, payment) is saved immediately.
4. Placing an order: pick customer -> add items to cart -> change/remove -> view bill -> confirm (choose Cash/UPI, Paid/Pending) or cancel (cart discarded).

## 4. File Formats

Fields are separated by `|`; `|`, `\` and newline inside a field are saved as `\|`, `\\`, `\n`.

| File | Record format |
|------|---------------|
| `food_items.txt` | `FoodID\|Name\|Category\|Price\|Available/Unavailable` |
| `customers.txt` | `CustomerID\|Name\|Contact` |
| `orders.txt` | `OrderID\|CustomerID\|Date\|Total\|Status\|ItemCount\|(FoodID\|Name\|Qty\|UnitPrice)...` |
| `payments.txt` | `PaymentID\|OrderID\|Method\|Amount\|Paid/Pending` |

ID formats: customers `C001...`, orders `O1001...`, payments `P1001...`. Counters continue from the highest saved ID after restart.

## 5. Billing Formula

```
Subtotal = sum(unit price x quantity)
GST      = Subtotal x 5%   (rounded to 2 decimals)
Final    = Subtotal + GST
```

Example: 2 x Veg Burger (99.00) + 1 x French Fries (79.00) = 277.00; GST 13.85; Final 290.85.

## 6. Sample Data

Customers: C001 Aarav Mehta, C002 Simran Kaur, C003 Rohan Verma (fictional).

| Order | Customer | Date | Items | Total | Status | Payment |
|-------|----------|------|-------|-------|--------|---------|
| O1001 | C001 | 2026-10-28 | 2 Veg Burger, 1 French Fries | 290.85 | Delivered | Cash, Paid |
| O1002 | C002 | 2026-10-29 | 1 Margherita Pizza, 2 Cold Coffee | 469.35 | Ready | UPI, Pending |
| O1003 | C003 | 2026-10-30 | 1 White Sauce Pasta, 1 Masala Lemonade | 270.90 | Preparing | UPI, Paid |
| O1004 | C001 | 2026-10-30 | 2 Chocolate Brownie, 1 Sandwich | 322.35 | Cancelled | Cash, Pending |

With the untouched sample data the Summary report should show: 4 total orders, 1 cancelled, revenue 561.75 (O1001 + O1003).

## 7. Testing Scenarios

Run from the project folder. Tick the boxes in README.md after you try them yourself.
Back up the `data/` folder first if you want to keep the original sample data.

| # | Scenario | Steps | Expected result |
|---|----------|-------|-----------------|
| 1 | Add food item | Food Menu > Add; ID 601, name "Veg Wrap", Snacks, price 110 | Item added and appears under Snacks; line appended to `food_items.txt` |
| 2 | Duplicate food ID | Add again with ID 601 | Error "Food ID already exists" |
| 3 | Register customer | Customers > Register; valid 10-digit number | New ID (e.g. C004) shown; saved in `customers.txt` |
| 4 | Invalid contact | Enter `12345` as contact | Message asking for exactly 10 digits; prompt repeats |
| 5 | Search food | Search by ID 301; by name "pizza" | Matching rows shown (name search is case-insensitive, partial) |
| 6 | Multi-item order | Place Order > C001; add 302 x2 and 201 x1; view cart | Both items listed with correct subtotal, GST and total |
| 7 | Invalid quantity | Enter 0, then -2 as quantity | "Invalid input" and prompt repeats; nothing added |
| 8 | Unavailable item | Add food ID 502 (or mark any item Unavailable first) | Error "currently unavailable"; not added |
| 9 | Update cart quantity | Cart menu > Change quantity; ID 302, new quantity 3 | Cart shows 3 x French Fries; totals recalculated |
| 10 | Remove cart item | Cart menu > Remove; ID 201 | Item disappears from cart |
| 11 | Bill calculation | View cart / bill | Check: subtotal + 5% GST = final amount |
| 12 | Cancel during ordering | Cart menu > Cancel order, answer y | Cart discarded, no new order saved |
| 13 | Cancel a saved order | Order Management > Cancel an order | Status Cancelled; excluded from revenue and top-items report |
| 14 | Update status | Order Management > Update status; choose Delivered | Status changes; Delivered/Cancelled orders cannot change again |
| 15 | Pay a pending bill | Billing > Pay pending bill (Cash or UPI) | Payment becomes Paid; revenue increases; cancelled orders cannot be paid |
| 16 | Save and load | Exit, start the program again | Foods, customers, orders and payments are still present; new IDs continue after the last one |
| 17 | Reports | Reports menu options 1 to 4 | Total orders, revenue, pending orders, top items and daily summary are shown |
| 18 | Missing/empty files | Empty one data file or rename the file; start program | Program starts with that list empty; no crash |
| 19 | Corrupt line | Add a nonsense line to `customers.txt`; start program | Warning "skipped bad record"; other records load |

## 8. Notes for Viva

- Why `Storable` is abstract: it forces every record type to know how to save itself and display itself.
- Why order lines copy name and price: later menu changes must not alter old bills.
- Why static counters: they give unique IDs without searching all records; on start-up they are moved past the highest loaded ID.
- Where is polymorphism used: `writeRecords()` calls `toRecord()` through `Storable*` for foods, customers, orders and payments.
