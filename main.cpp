/*
 * ============================================================
 *  Food Ordering Management System
 *  BCA Data Science (Hons) - Semester 3 - OOP Mini Project
 * ============================================================
 *  Language : C++11            Compiler : g++
 *  Interface: Menu-driven console application
 *  Storage  : Plain text files inside the data/ folder
 *
 *  Compile  : g++ -std=c++11 main.cpp -o food_ordering
 *  Run      : ./food_ordering   (Linux/macOS)
 *             food_ordering.exe (Windows)
 *  NOTE     : Run the program from the project folder so that the
 *             relative path "data/..." can be found.
 *
 *  This is an OFFLINE educational application. Payment methods
 *  (Cash / UPI) are for demonstration only - no real payment.
 * ============================================================
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;

// ============================================================
// SECTION 1: Constants, exceptions and helper functions
// ============================================================

const string FOOD_FILE     = "data/food_items.txt";
const string CUSTOMER_FILE = "data/customers.txt";
const string ORDER_FILE    = "data/orders.txt";
const string PAYMENT_FILE  = "data/payments.txt";

const int MAX_QTY = 100;   // maximum quantity of one item in an order

// Thrown when the input stream ends (e.g. Ctrl+D / Ctrl+Z) so that
// the program can exit cleanly instead of looping forever.
class InputClosedException : public runtime_error {
public:
    InputClosedException() : runtime_error("Input stream closed") {}
};

// List of valid order statuses
const vector<string>& orderStatuses() {
    static vector<string> s;
    if (s.empty()) {
        s.push_back("Placed");
        s.push_back("Preparing");
        s.push_back("Ready");
        s.push_back("Delivered");
        s.push_back("Cancelled");
    }
    return s;
}

bool isValidStatus(const string& st) {
    const vector<string>& v = orderStatuses();
    for (size_t i = 0; i < v.size(); ++i)
        if (v[i] == st) return true;
    return false;
}

// Default food categories
const vector<string>& defaultCategories() {
    static vector<string> c;
    if (c.empty()) {
        c.push_back("Starters");
        c.push_back("Main Course");
        c.push_back("Snacks");
        c.push_back("Beverages");
        c.push_back("Desserts");
    }
    return c;
}

string trim(const string& s) {
    size_t a = 0, b = s.size();
    while (a < b && isspace((unsigned char)s[a])) ++a;
    while (b > a && isspace((unsigned char)s[b - 1])) --b;
    return s.substr(a, b - a);
}

string toLower(string s) {
    for (size_t i = 0; i < s.size(); ++i) s[i] = (char)tolower((unsigned char)s[i]);
    return s;
}

string toUpper(string s) {
    for (size_t i = 0; i < s.size(); ++i) s[i] = (char)toupper((unsigned char)s[i]);
    return s;
}

double roundMoney(double v) {
    return floor(v * 100.0 + 0.5) / 100.0;
}

// Money as text with exactly two decimals, e.g. 99.00
string money(double v) {
    ostringstream o;
    o << fixed << setprecision(2) << v;
    return o.str();
}

// Numeric part of an ID such as "C003" -> 3, "O1005" -> 1005
int numberPart(const string& id) {
    if (id.size() < 2) return 0;
    return atoi(id.c_str() + 1);
}

int parseInt(const string& s) {
    if (s.empty()) throw invalid_argument("Empty number");
    char* end = 0;
    long v = strtol(s.c_str(), &end, 10);
    if (*end != '\0' || v > 2000000000L || v < -2000000000L)
        throw invalid_argument("Invalid integer: " + s);
    return (int)v;
}

double parseDouble(const string& s) {
    if (s.empty()) throw invalid_argument("Empty number");
    char* end = 0;
    double v = strtod(s.c_str(), &end);
    if (*end != '\0') throw invalid_argument("Invalid number: " + s);
    return v;
}

string todayDate() {
    time_t t = time(0);
    tm* lt = localtime(&t);
    char buf[16];
    strftime(buf, sizeof(buf), "%Y-%m-%d", lt);
    return string(buf);
}

// ---------- Safe text-file record format ----------
// Fields are separated by '|'. Inside a field the characters
// '|' , '\' and newline are escaped as \| , \\ and \n so that
// special characters never break a record.

string escapeField(const string& s) {
    string out;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '\\')      out += "\\\\";
        else if (c == '|')  out += "\\|";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') { /* ignore */ }
        else                out += c;
    }
    return out;
}

vector<string> splitRecord(const string& line) {
    vector<string> fields;
    string cur;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '\\' && i + 1 < line.size()) {
            char n = line[i + 1];
            if (n == 'n') cur += '\n';
            else          cur += n;
            ++i;
        } else if (c == '|') {
            fields.push_back(cur);
            cur.clear();
        } else {
            cur += c;
        }
    }
    fields.push_back(cur);
    return fields;
}

// Reads all non-empty lines of a file. A missing file gives an empty list.
vector<string> readLines(const string& path) {
    vector<string> lines;
    ifstream in(path.c_str());
    if (!in) return lines;
    string l;
    while (getline(in, l)) {
        l = trim(l);
        if (!l.empty()) lines.push_back(l);
    }
    return lines;
}

// ---------- Validated console input ----------

string readLine(const string& prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) throw InputClosedException();
    return trim(s);
}

int readInt(const string& prompt, int minV, int maxV) {
    while (true) {
        string s = readLine(prompt);
        try {
            int v = parseInt(s);
            if (v >= minV && v <= maxV) return v;
        } catch (const invalid_argument&) {
        }
        cout << "  Invalid input. Enter a whole number from " << minV << " to " << maxV << ".\n";
    }
}

double readPrice(const string& prompt) {
    while (true) {
        string s = readLine(prompt);
        try {
            double v = parseDouble(s);
            if (v > 0 && v <= 100000) return roundMoney(v);
        } catch (const invalid_argument&) {
        }
        cout << "  Invalid price. Enter a number greater than 0 (maximum 100000).\n";
    }
}

string readNonEmpty(const string& prompt, size_t maxLen) {
    while (true) {
        string s = readLine(prompt);
        if (!s.empty() && s.size() <= maxLen) return s;
        cout << "  Input cannot be empty and must be at most " << maxLen << " characters.\n";
    }
}

string readPhone(const string& prompt) {
    while (true) {
        string s = readLine(prompt);
        bool ok = (s.size() == 10);
        for (size_t i = 0; ok && i < s.size(); ++i)
            if (!isdigit((unsigned char)s[i])) ok = false;
        if (ok) return s;
        cout << "  Contact number must contain exactly 10 digits.\n";
    }
}

bool readYesNo(const string& prompt) {
    while (true) {
        string s = toLower(readLine(prompt));
        if (s == "y" || s == "yes") return true;
        if (s == "n" || s == "no")  return false;
        cout << "  Please enter y or n.\n";
    }
}

string chooseCategory() {
    const vector<string>& cats = defaultCategories();
    cout << "Select category:\n";
    for (size_t i = 0; i < cats.size(); ++i)
        cout << "  " << (i + 1) << ". " << cats[i] << "\n";
    int c = readInt("Category number: ", 1, (int)cats.size());
    return cats[c - 1];
}

string choosePaymentMethod() {
    cout << "Payment method (demonstration only):\n  1. Cash\n  2. UPI\n";
    int c = readInt("Choice: ", 1, 2);
    return (c == 1) ? "Cash" : "UPI";
}

// ============================================================
// SECTION 2: Abstract base class (Abstraction)
// ============================================================
// Every record that is saved to a text file derives from Storable.
// It is abstract: it cannot be instantiated, it only defines the
// functions that all records must provide.

class Storable {
public:
    virtual ~Storable() {}
    virtual string toRecord() const = 0;   // one line for the text file
    virtual void display() const = 0;      // print on screen
};

// Polymorphism: this one function saves ANY kind of record. The
// correct toRecord() (FoodItem, Customer, Order or Payment) is
// chosen at run time through the Storable pointer.
void writeRecords(const string& path, const vector<const Storable*>& records) {
    ofstream out(path.c_str());
    if (!out)
        throw runtime_error("Cannot write to " + path +
                            " (run the program from the project folder; the data folder must exist)");
    for (size_t i = 0; i < records.size(); ++i)
        out << records[i]->toRecord() << "\n";
}

// ============================================================
// SECTION 3: FoodItem and FoodMenu
// ============================================================

class FoodItem : public Storable {
private:
    int    id;
    string name;
    string category;
    double price;
    bool   available;

public:
    // Constructor overloading
    FoodItem();
    FoodItem(int itemId, const string& itemName, const string& itemCategory, double itemPrice);
    FoodItem(int itemId, const string& itemName, const string& itemCategory, double itemPrice, bool isAvailable);

    int           getId() const        { return id; }
    const string& getName() const      { return name; }
    const string& getCategory() const  { return category; }
    double        getPrice() const     { return price; }
    bool          isAvailable() const  { return available; }

    void setName(const string& newName);
    void setPrice(double newPrice);
    void setAvailable(bool a)          { available = a; }

    string toRecord() const override;
    void   display() const override;

    static FoodItem fromRecord(const string& line);
};

FoodItem::FoodItem() : id(0), name(""), category(""), price(0.0), available(true) {}

FoodItem::FoodItem(int itemId, const string& itemName, const string& itemCategory, double itemPrice)
    : id(itemId), category(itemCategory), available(true) {
    if (itemId <= 0) throw invalid_argument("Food ID must be positive.");
    if (itemCategory.empty()) throw invalid_argument("Category cannot be empty.");
    setName(itemName);
    setPrice(itemPrice);
}

FoodItem::FoodItem(int itemId, const string& itemName, const string& itemCategory, double itemPrice, bool isAvailable)
    : id(itemId), category(itemCategory), available(isAvailable) {
    if (itemId <= 0) throw invalid_argument("Food ID must be positive.");
    if (itemCategory.empty()) throw invalid_argument("Category cannot be empty.");
    setName(itemName);
    setPrice(itemPrice);
}

void FoodItem::setName(const string& newName) {
    if (trim(newName).empty()) throw invalid_argument("Food name cannot be empty.");
    name = trim(newName);
}

void FoodItem::setPrice(double newPrice) {
    if (!(newPrice > 0 && newPrice <= 100000))
        throw invalid_argument("Price must be greater than 0 (maximum 100000).");
    price = roundMoney(newPrice);
}

string FoodItem::toRecord() const {
    ostringstream o;
    o << id << '|' << escapeField(name) << '|' << escapeField(category) << '|'
      << money(price) << '|' << (available ? "Available" : "Unavailable");
    return o.str();
}

void FoodItem::display() const {
    cout << left << setw(6) << id << setw(24) << name << setw(14) << category
         << right << setw(9) << money(price) << "   " << left
         << (available ? "Available" : "Unavailable") << "\n";
}

FoodItem FoodItem::fromRecord(const string& line) {
    vector<string> f = splitRecord(line);
    if (f.size() != 5) throw invalid_argument("food record needs 5 fields");
    bool avail;
    if (f[4] == "Available")        avail = true;
    else if (f[4] == "Unavailable") avail = false;
    else throw invalid_argument("availability must be Available or Unavailable");
    return FoodItem(parseInt(f[0]), f[1], f[2], parseDouble(f[3]), avail);
}

// ------------------------------------------------------------

class FoodMenu {
private:
    vector<FoodItem> items;
    int indexOf(int id) const;
    void printHeader() const;

public:
    void load();
    void save() const;

    bool exists(int id) const { return indexOf(id) != -1; }
    int  size() const         { return (int)items.size(); }

    FoodItem* findById(int id);                 // returns pointer or nullptr
    void addItem(const FoodItem& item);
    void updateItem(int id, const string& newName);   // overloaded: update name
    void updateItem(int id, double newPrice);         // overloaded: update price
    void removeItem(int id);
    void setAvailability(int id, bool available);

    void displayAll() const;
    void search(int id) const;                  // overloaded: search by ID
    void search(const string& keyword) const;   // overloaded: search by name
};

int FoodMenu::indexOf(int id) const {
    for (size_t i = 0; i < items.size(); ++i)
        if (items[i].getId() == id) return (int)i;
    return -1;
}

void FoodMenu::printHeader() const {
    cout << left << setw(6) << "ID" << setw(24) << "Name" << setw(14) << "Category"
         << right << setw(9) << "Price" << "   " << left << "Status" << "\n";
    cout << string(66, '-') << "\n";
}

void FoodMenu::load() {
    items.clear();
    vector<string> lines = readLines(FOOD_FILE);
    for (size_t i = 0; i < lines.size(); ++i) {
        try {
            FoodItem f = FoodItem::fromRecord(lines[i]);
            if (exists(f.getId())) {
                cout << "Warning: duplicate food ID " << f.getId() << " skipped.\n";
                continue;
            }
            items.push_back(f);
        } catch (const exception& e) {
            cout << "Warning: skipped bad record " << (i + 1) << " in " << FOOD_FILE
                 << " (" << e.what() << ")\n";
        }
    }
}

void FoodMenu::save() const {
    vector<const Storable*> recs;
    for (size_t i = 0; i < items.size(); ++i) recs.push_back(&items[i]);
    writeRecords(FOOD_FILE, recs);
}

FoodItem* FoodMenu::findById(int id) {
    int i = indexOf(id);
    return (i == -1) ? nullptr : &items[i];
}

void FoodMenu::addItem(const FoodItem& item) {
    if (exists(item.getId())) throw invalid_argument("Food ID already exists.");
    items.push_back(item);
    save();
}

void FoodMenu::updateItem(int id, const string& newName) {
    FoodItem* f = findById(id);
    if (!f) throw invalid_argument("Food item not found.");
    f->setName(newName);
    save();
}

void FoodMenu::updateItem(int id, double newPrice) {
    FoodItem* f = findById(id);
    if (!f) throw invalid_argument("Food item not found.");
    f->setPrice(newPrice);
    save();
}

void FoodMenu::removeItem(int id) {
    int i = indexOf(id);
    if (i == -1) throw invalid_argument("Food item not found.");
    items.erase(items.begin() + i);
    save();
}

void FoodMenu::setAvailability(int id, bool available) {
    FoodItem* f = findById(id);
    if (!f) throw invalid_argument("Food item not found.");
    f->setAvailable(available);
    save();
}

void FoodMenu::displayAll() const {
    if (items.empty()) {
        cout << "The menu is empty.\n";
        return;
    }
    cout << "\n=========== FOOD MENU ===========\n";
    printHeader();

    // known categories first, then any other categories found in the data
    vector<string> cats = defaultCategories();
    for (size_t i = 0; i < items.size(); ++i)
        if (find(cats.begin(), cats.end(), items[i].getCategory()) == cats.end())
            cats.push_back(items[i].getCategory());

    for (size_t c = 0; c < cats.size(); ++c) {
        bool headingShown = false;
        for (size_t i = 0; i < items.size(); ++i) {
            if (items[i].getCategory() != cats[c]) continue;
            if (!headingShown) {
                cout << "[" << cats[c] << "]\n";
                headingShown = true;
            }
            items[i].display();
        }
    }
}

void FoodMenu::search(int id) const {
    int i = indexOf(id);
    if (i == -1) {
        cout << "No food item found with ID " << id << ".\n";
        return;
    }
    printHeader();
    items[i].display();
}

void FoodMenu::search(const string& keyword) const {
    string key = toLower(keyword);
    bool found = false;
    for (size_t i = 0; i < items.size(); ++i) {
        if (toLower(items[i].getName()).find(key) != string::npos) {
            if (!found) printHeader();
            items[i].display();
            found = true;
        }
    }
    if (!found) cout << "No food item found matching \"" << keyword << "\".\n";
}

// ============================================================
// SECTION 4: Person (base) -> Customer (derived)  (Inheritance)
// ============================================================

class Person : public Storable {
protected:
    string name;
    string contact;

public:
    Person() {}
    Person(const string& n, const string& c) : name(n), contact(c) {}
    virtual ~Person() {}

    const string& getName() const    { return name; }
    const string& getContact() const { return contact; }

    // Overridden in derived classes (polymorphism)
    virtual string getRole() const { return "Person"; }
};

class Customer : public Person {
private:
    string customerId;
    static int nextNumber;          // static data member: next free customer number
    static string makeId();

public:
    // Constructor overloading
    Customer() {}
    Customer(const string& n, const string& c);                       // new customer: ID generated
    Customer(const string& id, const string& n, const string& c);     // loaded from file

    const string& getCustomerId() const { return customerId; }
    string getRole() const override     { return "Customer"; }

    string toRecord() const override;
    void   display() const override;

    static Customer fromRecord(const string& line);
    static void syncCounter(const string& id);   // keep counter ahead of loaded IDs
};

int Customer::nextNumber = 1;

string Customer::makeId() {
    ostringstream o;
    o << "C" << setw(3) << setfill('0') << nextNumber++;
    return o.str();
}

Customer::Customer(const string& n, const string& c) : Person(n, c), customerId(makeId()) {}

Customer::Customer(const string& id, const string& n, const string& c) : Person(n, c), customerId(id) {}

string Customer::toRecord() const {
    return escapeField(customerId) + "|" + escapeField(name) + "|" + escapeField(contact);
}

void Customer::display() const {
    cout << left << setw(8) << customerId << setw(32) << name << setw(12) << contact << "\n";
}

Customer Customer::fromRecord(const string& line) {
    vector<string> f = splitRecord(line);
    if (f.size() != 3) throw invalid_argument("customer record needs 3 fields");
    if (f[0].empty() || f[1].empty() || f[2].empty()) throw invalid_argument("empty field");
    return Customer(f[0], f[1], f[2]);
}

void Customer::syncCounter(const string& id) {
    int n = numberPart(id);
    if (n >= nextNumber) nextNumber = n + 1;
}

// ------------------------------------------------------------

class CustomerManager {
private:
    vector<Customer> customers;
    void printHeader() const;

public:
    void load();
    void save() const;

    int  size() const { return (int)customers.size(); }
    bool exists(const string& id) const;
    Customer* findById(const string& id);

    string registerCustomer(const string& name, const string& contact);
    void displayAll() const;
    void searchById(const string& id) const;
    void searchByName(const string& keyword) const;
};

void CustomerManager::printHeader() const {
    cout << left << setw(8) << "ID" << setw(32) << "Name" << setw(12) << "Contact" << "\n";
    cout << string(52, '-') << "\n";
}

void CustomerManager::load() {
    customers.clear();
    vector<string> lines = readLines(CUSTOMER_FILE);
    for (size_t i = 0; i < lines.size(); ++i) {
        try {
            Customer c = Customer::fromRecord(lines[i]);
            if (exists(c.getCustomerId())) {
                cout << "Warning: duplicate customer ID " << c.getCustomerId() << " skipped.\n";
                continue;
            }
            customers.push_back(c);
            Customer::syncCounter(c.getCustomerId());
        } catch (const exception& e) {
            cout << "Warning: skipped bad record " << (i + 1) << " in " << CUSTOMER_FILE
                 << " (" << e.what() << ")\n";
        }
    }
}

void CustomerManager::save() const {
    vector<const Storable*> recs;
    for (size_t i = 0; i < customers.size(); ++i) recs.push_back(&customers[i]);
    writeRecords(CUSTOMER_FILE, recs);
}

bool CustomerManager::exists(const string& id) const {
    for (size_t i = 0; i < customers.size(); ++i)
        if (customers[i].getCustomerId() == id) return true;
    return false;
}

Customer* CustomerManager::findById(const string& id) {
    for (size_t i = 0; i < customers.size(); ++i)
        if (customers[i].getCustomerId() == id) return &customers[i];
    return nullptr;
}

string CustomerManager::registerCustomer(const string& name, const string& contact) {
    Customer c(name, contact);                     // ID is generated here
    if (exists(c.getCustomerId()))
        throw logic_error("Duplicate customer ID generated.");
    customers.push_back(c);
    save();
    return c.getCustomerId();
}

void CustomerManager::displayAll() const {
    if (customers.empty()) {
        cout << "No customers registered yet.\n";
        return;
    }
    printHeader();
    for (size_t i = 0; i < customers.size(); ++i) customers[i].display();
}

void CustomerManager::searchById(const string& id) const {
    for (size_t i = 0; i < customers.size(); ++i) {
        if (customers[i].getCustomerId() == id) {
            printHeader();
            customers[i].display();
            return;
        }
    }
    cout << "No customer found with ID " << id << ".\n";
}

void CustomerManager::searchByName(const string& keyword) const {
    string key = toLower(keyword);
    bool found = false;
    for (size_t i = 0; i < customers.size(); ++i) {
        if (toLower(customers[i].getName()).find(key) != string::npos) {
            if (!found) printHeader();
            customers[i].display();
            found = true;
        }
    }
    if (!found) cout << "No customer found matching \"" << keyword << "\".\n";
}

// ============================================================
// SECTION 5: OrderItem and Order  (Composition)
// ============================================================

// One line of an order. The food name and price are COPIED into the
// order so that old orders stay correct even if the menu changes later.
class OrderItem {
private:
    int    foodId;
    string name;
    double unitPrice;
    int    quantity;

public:
    OrderItem() : foodId(0), unitPrice(0.0), quantity(1) {}
    OrderItem(int id, const string& itemName, double price, int qty)
        : foodId(id), name(itemName), unitPrice(price), quantity(1) {
        if (id <= 0) throw invalid_argument("Invalid food ID in order item.");
        if (price <= 0) throw invalid_argument("Invalid price in order item.");
        setQuantity(qty);
    }

    int           getFoodId() const    { return foodId; }
    const string& getName() const      { return name; }
    double        getUnitPrice() const { return unitPrice; }
    int           getQuantity() const  { return quantity; }

    void setQuantity(int q) {
        if (q <= 0) throw invalid_argument("Quantity must be at least 1.");
        if (q > MAX_QTY) throw invalid_argument("Quantity cannot be more than 100.");
        quantity = q;
    }
    double getSubtotal() const { return roundMoney(unitPrice * quantity); }
};

// ------------------------------------------------------------

class Order : public Storable {
private:
    string customerIdValue;
    string orderId;
    string date;
    vector<OrderItem> items;       // composition: items live and die with the order
    string status;
    static int nextNumber;         // static data member: next free order number
    int itemIndex(int foodId) const;

public:
    static const double TAX_RATE;  // static constant: GST 5% (demonstration)

    Order() : status("Placed") {}
    explicit Order(const string& customerId)
        : customerIdValue(customerId), date(todayDate()), status("Placed") {}

    void assignNewId();            // called when the order is confirmed

    const string& getOrderId() const    { return orderId; }
    const string& getCustomerId() const { return customerIdValue; }
    const string& getDate() const       { return date; }
    const string& getStatus() const     { return status; }
    const vector<OrderItem>& getItems() const { return items; }
    void setStatus(const string& s)     { status = s; }
    bool isEmpty() const                { return items.empty(); }

    // Function overloading: add from the menu, or add a ready-made line
    void addItem(const FoodItem& food, int qty);
    void addItem(const OrderItem& item);

    bool removeItem(int foodId);
    bool changeQuantity(int foodId, int qty);
    bool hasItem(int foodId) const { return itemIndex(foodId) != -1; }

    double getSubtotal() const;
    double getTax() const;
    double getTotal() const;

    void displayBill() const;
    void displayRow(const string& paymentStatus) const;

    string toRecord() const override;
    void   display() const override;

    static Order fromRecord(const string& line);
    static void  syncCounter(const string& id);
};

int Order::nextNumber = 1001;
const double Order::TAX_RATE = 0.05;

void Order::assignNewId() {
    ostringstream o;
    o << "O" << nextNumber++;
    orderId = o.str();
}

int Order::itemIndex(int foodId) const {
    for (size_t i = 0; i < items.size(); ++i)
        if (items[i].getFoodId() == foodId) return (int)i;
    return -1;
}

void Order::addItem(const FoodItem& food, int qty) {
    if (qty <= 0) throw invalid_argument("Quantity must be at least 1.");
    if (!food.isAvailable()) throw invalid_argument(food.getName() + " is currently unavailable.");
    int i = itemIndex(food.getId());
    if (i != -1) {
        items[i].setQuantity(items[i].getQuantity() + qty);    // already in cart: add quantity
    } else {
        items.push_back(OrderItem(food.getId(), food.getName(), food.getPrice(), qty));
    }
}

void Order::addItem(const OrderItem& item) {
    if (itemIndex(item.getFoodId()) != -1) throw invalid_argument("Duplicate item in order.");
    items.push_back(item);
}

bool Order::removeItem(int foodId) {
    int i = itemIndex(foodId);
    if (i == -1) return false;
    items.erase(items.begin() + i);
    return true;
}

bool Order::changeQuantity(int foodId, int qty) {
    int i = itemIndex(foodId);
    if (i == -1) return false;
    items[i].setQuantity(qty);
    return true;
}

double Order::getSubtotal() const {
    double sum = 0.0;
    for (size_t i = 0; i < items.size(); ++i) sum += items[i].getSubtotal();
    return roundMoney(sum);
}

double Order::getTax() const   { return roundMoney(getSubtotal() * TAX_RATE); }
double Order::getTotal() const { return roundMoney(getSubtotal() + getTax()); }

void Order::displayBill() const {
    const string line(59, '-');
    cout << line << "\n";
    cout << left << setw(6) << "ID" << setw(24) << "Item" << right << setw(5) << "Qty"
         << setw(11) << "Price" << setw(13) << "Amount" << "\n";
    cout << line << "\n";
    for (size_t i = 0; i < items.size(); ++i) {
        cout << left << setw(6) << items[i].getFoodId() << setw(24) << items[i].getName()
             << right << setw(5) << items[i].getQuantity()
             << setw(11) << money(items[i].getUnitPrice())
             << setw(13) << money(items[i].getSubtotal()) << "\n";
    }
    cout << line << "\n";
    ostringstream gst;
    gst << "GST (" << (int)(TAX_RATE * 100 + 0.5) << "%)";
    cout << left << setw(46) << "Subtotal" << right << setw(13) << money(getSubtotal()) << "\n";
    cout << left << setw(46) << gst.str() << right << setw(13) << money(getTax()) << "\n";
    cout << left << setw(46) << "FINAL PAYABLE AMOUNT (Rs.)" << right << setw(13) << money(getTotal()) << "\n";
    cout << line << "\n" << left;
}

void Order::displayRow(const string& paymentStatus) const {
    cout << left << setw(9) << orderId << setw(9) << customerIdValue << setw(12) << date
         << setw(7) << items.size() << right << setw(11) << money(getTotal()) << "  " << left
         << setw(11) << status << paymentStatus << "\n";
}

void Order::display() const {
    cout << "\nOrder ID    : " << orderId
         << "\nCustomer ID : " << customerIdValue
         << "\nDate        : " << date
         << "\nStatus      : " << status << "\n";
    displayBill();
}

string Order::toRecord() const {
    ostringstream o;
    o << escapeField(orderId) << '|' << escapeField(customerIdValue) << '|' << escapeField(date) << '|'
      << money(getTotal()) << '|' << escapeField(status) << '|' << items.size();
    for (size_t i = 0; i < items.size(); ++i) {
        o << '|' << items[i].getFoodId() << '|' << escapeField(items[i].getName()) << '|'
          << items[i].getQuantity() << '|' << money(items[i].getUnitPrice());
    }
    return o.str();
}

// Record layout: id|customerId|date|total|status|itemCount| then per item: foodId|name|qty|unitPrice
Order Order::fromRecord(const string& line) {
    vector<string> f = splitRecord(line);
    if (f.size() < 6) throw invalid_argument("order record has too few fields");
    int count = parseInt(f[5]);
    if (count < 0 || f.size() != (size_t)(6 + count * 4))
        throw invalid_argument("item fields do not match item count");
    if (f[0].empty() || f[1].empty() || f[2].empty()) throw invalid_argument("empty field");
    if (!isValidStatus(f[4])) throw invalid_argument("unknown order status");

    Order o;
    o.orderId = f[0];
    o.customerIdValue = f[1];
    o.date = f[2];
    o.status = f[4];
    for (int i = 0; i < count; ++i) {
        size_t b = 6 + (size_t)i * 4;
        o.addItem(OrderItem(parseInt(f[b]), f[b + 1], parseDouble(f[b + 3]), parseInt(f[b + 2])));
    }
    return o;
}

void Order::syncCounter(const string& id) {
    int n = numberPart(id);
    if (n >= nextNumber) nextNumber = n + 1;
}

// ============================================================
// SECTION 6: Payment
// ============================================================

class Payment : public Storable {
private:
    string paymentId;
    string orderId;
    string method;     // Cash or UPI
    double amount;
    string status;     // Paid or Pending
    static int nextNumber;

    void setMethod(const string& m);
    void setStatus(const string& s);

public:
    // Constructor overloading
    Payment() : amount(0.0), status("Pending") {}
    Payment(const string& ordId, const string& payMethod, double payAmount, const string& payStatus);
    Payment(const string& payId, const string& ordId, const string& payMethod, double payAmount, const string& payStatus);

    const string& getPaymentId() const { return paymentId; }
    const string& getOrderId() const   { return orderId; }
    const string& getMethod() const    { return method; }
    double        getAmount() const    { return amount; }
    const string& getStatus() const    { return status; }

    void markPaid(const string& payMethod) {
        setMethod(payMethod);
        status = "Paid";
    }

    string toRecord() const override;
    void   display() const override;

    static Payment fromRecord(const string& line);
    static void    syncCounter(const string& id);
};

int Payment::nextNumber = 1001;

void Payment::setMethod(const string& m) {
    if (m != "Cash" && m != "UPI") throw invalid_argument("Payment method must be Cash or UPI.");
    method = m;
}

void Payment::setStatus(const string& s) {
    if (s != "Paid" && s != "Pending") throw invalid_argument("Payment status must be Paid or Pending.");
    status = s;
}

Payment::Payment(const string& ordId, const string& payMethod, double payAmount, const string& payStatus)
    : orderId(ordId), amount(payAmount) {
    ostringstream o;
    o << "P" << nextNumber++;
    paymentId = o.str();
    setMethod(payMethod);
    setStatus(payStatus);
}

Payment::Payment(const string& payId, const string& ordId, const string& payMethod, double payAmount, const string& payStatus)
    : paymentId(payId), orderId(ordId), amount(payAmount) {
    setMethod(payMethod);
    setStatus(payStatus);
}

string Payment::toRecord() const {
    return escapeField(paymentId) + "|" + escapeField(orderId) + "|" + escapeField(method) + "|" +
           money(amount) + "|" + escapeField(status);
}

void Payment::display() const {
    cout << left << setw(10) << paymentId << setw(10) << orderId << setw(8) << method
         << right << setw(11) << money(amount) << "  " << left << status << "\n";
}

Payment Payment::fromRecord(const string& line) {
    vector<string> f = splitRecord(line);
    if (f.size() != 5) throw invalid_argument("payment record needs 5 fields");
    if (f[0].empty() || f[1].empty()) throw invalid_argument("empty ID");
    return Payment(f[0], f[1], f[2], parseDouble(f[3]), f[4]);
}

void Payment::syncCounter(const string& id) {
    int n = numberPart(id);
    if (n >= nextNumber) nextNumber = n + 1;
}

// ============================================================
// SECTION 7: OrderManager (orders, payments and reports)
// ============================================================

// Helper for sorting "most ordered" items (highest quantity first)
bool byQuantityDesc(const pair<string, int>& a, const pair<string, int>& b) {
    if (a.second != b.second) return a.second > b.second;
    return a.first < b.first;
}

struct DaySummary {
    int orders;
    int cancelled;
    double revenue;
    DaySummary() : orders(0), cancelled(0), revenue(0.0) {}
};

class OrderManager {
private:
    vector<Order>   orders;
    vector<Payment> payments;
    CustomerManager* customers;     // aggregation: uses the customer manager but does not own it

    int orderIndex(const string& id) const;
    int paymentIndex(const string& orderId) const;
    string paymentStatusOf(const string& orderId) const;
    void printOrderHeader() const;

public:
    explicit OrderManager(CustomerManager* cm) : customers(cm) {}

    void load();
    void save() const;

    string confirmOrder(Order& cart, const string& method, bool paidNow);
    void   updateStatus(const string& orderId, const string& newStatus);
    void   payForOrder(const string& orderId, const string& method);

    bool hasOrder(const string& orderId) const { return orderIndex(orderId) != -1; }

    void displayAllOrders() const;
    void displayOrderDetails(const string& orderId) const;
    void displayBillFor(const string& orderId) const;
    void displayCustomerHistory(const string& customerId) const;
    void displayAllPayments() const;

    // Reports
    int    totalOrders() const { return (int)orders.size(); }
    double totalRevenue() const;
    void displaySummaryReport() const;
    void displayPendingOrders() const;
    void displayTopItems() const;
    void displayDailySummary() const;
};

int OrderManager::orderIndex(const string& id) const {
    for (size_t i = 0; i < orders.size(); ++i)
        if (orders[i].getOrderId() == id) return (int)i;
    return -1;
}

int OrderManager::paymentIndex(const string& orderId) const {
    for (size_t i = 0; i < payments.size(); ++i)
        if (payments[i].getOrderId() == orderId) return (int)i;
    return -1;
}

string OrderManager::paymentStatusOf(const string& orderId) const {
    int p = paymentIndex(orderId);
    return (p == -1) ? "Pending" : payments[p].getStatus();
}

void OrderManager::printOrderHeader() const {
    cout << left << setw(9) << "Order" << setw(9) << "Customer" << setw(12) << "Date"
         << setw(7) << "Items" << right << setw(11) << "Total" << "  " << left
         << setw(11) << "Status" << "Payment" << "\n";
    cout << string(70, '-') << "\n";
}

void OrderManager::load() {
    orders.clear();
    payments.clear();

    vector<string> lines = readLines(ORDER_FILE);
    for (size_t i = 0; i < lines.size(); ++i) {
        try {
            Order o = Order::fromRecord(lines[i]);
            if (orderIndex(o.getOrderId()) != -1) {
                cout << "Warning: duplicate order ID " << o.getOrderId() << " skipped.\n";
                continue;
            }
            orders.push_back(o);
            Order::syncCounter(o.getOrderId());
        } catch (const exception& e) {
            cout << "Warning: skipped bad record " << (i + 1) << " in " << ORDER_FILE
                 << " (" << e.what() << ")\n";
        }
    }

    lines = readLines(PAYMENT_FILE);
    for (size_t i = 0; i < lines.size(); ++i) {
        try {
            Payment p = Payment::fromRecord(lines[i]);
            if (paymentIndex(p.getOrderId()) != -1) {
                cout << "Warning: duplicate payment for order " << p.getOrderId() << " skipped.\n";
                continue;
            }
            payments.push_back(p);
            Payment::syncCounter(p.getPaymentId());
        } catch (const exception& e) {
            cout << "Warning: skipped bad record " << (i + 1) << " in " << PAYMENT_FILE
                 << " (" << e.what() << ")\n";
        }
    }
}

void OrderManager::save() const {
    vector<const Storable*> recs;
    for (size_t i = 0; i < orders.size(); ++i) recs.push_back(&orders[i]);
    writeRecords(ORDER_FILE, recs);

    recs.clear();
    for (size_t i = 0; i < payments.size(); ++i) recs.push_back(&payments[i]);
    writeRecords(PAYMENT_FILE, recs);
}

string OrderManager::confirmOrder(Order& cart, const string& method, bool paidNow) {
    if (cart.isEmpty()) throw logic_error("Cannot confirm an empty cart.");
    cart.assignNewId();
    if (orderIndex(cart.getOrderId()) != -1) throw logic_error("Duplicate order ID generated.");
    orders.push_back(cart);
    payments.push_back(Payment(cart.getOrderId(), method, cart.getTotal(), paidNow ? "Paid" : "Pending"));
    save();
    return cart.getOrderId();
}

void OrderManager::updateStatus(const string& orderId, const string& newStatus) {
    int i = orderIndex(orderId);
    if (i == -1) throw invalid_argument("Order not found: " + orderId);
    if (!isValidStatus(newStatus)) throw invalid_argument("Invalid order status.");
    string current = orders[i].getStatus();
    if (current == "Cancelled" || current == "Delivered")
        throw invalid_argument("Order " + orderId + " is already " + current + " and cannot be changed.");
    orders[i].setStatus(newStatus);
    save();
}

void OrderManager::payForOrder(const string& orderId, const string& method) {
    int i = orderIndex(orderId);
    if (i == -1) throw invalid_argument("Order not found: " + orderId);
    if (orders[i].getStatus() == "Cancelled") throw invalid_argument("A cancelled order cannot be paid.");
    int p = paymentIndex(orderId);
    if (p == -1) throw invalid_argument("No payment record exists for this order.");
    if (payments[p].getStatus() == "Paid") throw invalid_argument("This order is already paid.");
    payments[p].markPaid(method);
    save();
}

void OrderManager::displayAllOrders() const {
    if (orders.empty()) {
        cout << "No orders have been placed yet.\n";
        return;
    }
    printOrderHeader();
    for (size_t i = 0; i < orders.size(); ++i)
        orders[i].displayRow(paymentStatusOf(orders[i].getOrderId()));
}

void OrderManager::displayOrderDetails(const string& orderId) const {
    int i = orderIndex(orderId);
    if (i == -1) {
        cout << "No order found with ID " << orderId << ".\n";
        return;
    }
    orders[i].display();
    Customer* c = customers->findById(orders[i].getCustomerId());
    cout << "Customer    : " << (c ? c->getName() : string("(customer record not found)")) << "\n";
    int p = paymentIndex(orderId);
    if (p != -1)
        cout << "Payment     : " << payments[p].getMethod() << " - " << payments[p].getStatus() << "\n";
}

void OrderManager::displayBillFor(const string& orderId) const {
    int i = orderIndex(orderId);
    if (i == -1) {
        cout << "No order found with ID " << orderId << ".\n";
        return;
    }
    cout << "\n============ BILL - Order " << orderId << " ============\n";
    cout << "Date: " << orders[i].getDate() << "   Status: " << orders[i].getStatus() << "\n";
    orders[i].displayBill();
    int p = paymentIndex(orderId);
    if (p != -1)
        cout << "Payment method: " << payments[p].getMethod()
             << "   Payment status: " << payments[p].getStatus() << "\n";
}

void OrderManager::displayCustomerHistory(const string& customerId) const {
    bool found = false;
    for (size_t i = 0; i < orders.size(); ++i) {
        if (orders[i].getCustomerId() != customerId) continue;
        if (!found) printOrderHeader();
        orders[i].displayRow(paymentStatusOf(orders[i].getOrderId()));
        found = true;
    }
    if (!found) cout << "This customer has no orders yet.\n";
}

void OrderManager::displayAllPayments() const {
    if (payments.empty()) {
        cout << "No payment records yet.\n";
        return;
    }
    cout << left << setw(10) << "Payment" << setw(10) << "Order" << setw(8) << "Method"
         << right << setw(11) << "Amount" << "  " << left << "Status" << "\n";
    cout << string(50, '-') << "\n";
    for (size_t i = 0; i < payments.size(); ++i) payments[i].display();
}

// ----- Reports -----

// Revenue = payments marked Paid for orders that are NOT cancelled.
double OrderManager::totalRevenue() const {
    double sum = 0.0;
    for (size_t i = 0; i < orders.size(); ++i) {
        if (orders[i].getStatus() == "Cancelled") continue;
        int p = paymentIndex(orders[i].getOrderId());
        if (p != -1 && payments[p].getStatus() == "Paid") sum += payments[p].getAmount();
    }
    return roundMoney(sum);
}

void OrderManager::displaySummaryReport() const {
    int cancelled = 0, delivered = 0, pending = 0;
    double pendingAmount = 0.0;
    for (size_t i = 0; i < orders.size(); ++i) {
        const string& st = orders[i].getStatus();
        if (st == "Cancelled") { ++cancelled; continue; }
        if (st == "Delivered") ++delivered;
        if (paymentStatusOf(orders[i].getOrderId()) != "Paid") {
            ++pending;
            pendingAmount += orders[i].getTotal();
        }
    }
    cout << "\n------ SUMMARY REPORT ------\n";
    cout << "Total orders             : " << totalOrders() << "\n";
    cout << "Cancelled orders         : " << cancelled << "\n";
    cout << "Delivered orders         : " << delivered << "\n";
    cout << "Orders with payment due  : " << pending << " (Rs. " << money(pendingAmount) << ")\n";
    cout << "Total revenue (paid)     : Rs. " << money(totalRevenue()) << "\n";
}

void OrderManager::displayPendingOrders() const {
    bool found = false;
    for (size_t i = 0; i < orders.size(); ++i) {
        if (orders[i].getStatus() == "Cancelled") continue;
        if (paymentStatusOf(orders[i].getOrderId()) == "Paid") continue;
        if (!found) {
            cout << "\nPending orders (payment not received, not cancelled):\n";
            printOrderHeader();
        }
        orders[i].displayRow("Pending");
        found = true;
    }
    if (!found) cout << "There are no pending orders.\n";
}

void OrderManager::displayTopItems() const {
    map<string, int> qtyByName;
    for (size_t i = 0; i < orders.size(); ++i) {
        if (orders[i].getStatus() == "Cancelled") continue;
        const vector<OrderItem>& its = orders[i].getItems();
        for (size_t j = 0; j < its.size(); ++j) qtyByName[its[j].getName()] += its[j].getQuantity();
    }
    if (qtyByName.empty()) {
        cout << "No ordered items to report yet.\n";
        return;
    }
    vector<pair<string, int> > v(qtyByName.begin(), qtyByName.end());
    sort(v.begin(), v.end(), byQuantityDesc);

    cout << "\nMost frequently ordered items (cancelled orders excluded):\n";
    cout << left << setw(6) << "Rank" << setw(26) << "Item" << "Quantity" << "\n";
    cout << string(40, '-') << "\n";
    for (size_t i = 0; i < v.size() && i < 5; ++i)
        cout << left << setw(6) << (i + 1) << setw(26) << v[i].first << v[i].second << "\n";
}

void OrderManager::displayDailySummary() const {
    if (orders.empty()) {
        cout << "No orders to summarise.\n";
        return;
    }
    map<string, DaySummary> days;
    for (size_t i = 0; i < orders.size(); ++i) {
        DaySummary& d = days[orders[i].getDate()];
        d.orders++;
        if (orders[i].getStatus() == "Cancelled") {
            d.cancelled++;
            continue;
        }
        int p = paymentIndex(orders[i].getOrderId());
        if (p != -1 && payments[p].getStatus() == "Paid") d.revenue += payments[p].getAmount();
    }
    cout << "\nDaily order summary:\n";
    cout << left << setw(13) << "Date" << setw(9) << "Orders" << setw(11) << "Cancelled"
         << right << setw(14) << "Revenue (Rs.)" << "\n";
    cout << string(47, '-') << "\n";
    for (map<string, DaySummary>::const_iterator it = days.begin(); it != days.end(); ++it) {
        cout << left << setw(13) << it->first << setw(9) << it->second.orders
             << setw(11) << it->second.cancelled << right << setw(14) << money(it->second.revenue) << "\n";
    }
    cout << left;
}

// ============================================================
// SECTION 8: FoodOrderingSystem (menus and user interface)
// ============================================================

class FoodOrderingSystem {
private:
    // Composition: the system owns these objects.
    // (Declaration order matters: 'customers' must exist before 'orders'.)
    FoodMenu        menu;
    CustomerManager customers;
    OrderManager    orders;

    // menus
    void mainMenu();
    void foodMenuMenu();
    void customerMenu();
    void orderManagementMenu();
    void billingMenu();
    void reportsMenu();

    // food actions
    void searchFood();
    void addFood();
    void updateFood();
    void deleteFood();
    void changeAvailability();

    // customer actions
    void registerCustomer();
    void searchCustomer();

    // ordering
    void placeOrder();
    void addToCart(Order& cart);
    void changeCartQuantity(Order& cart);
    void removeFromCart(Order& cart);
    void showCart(const Order& cart);
    bool confirmCart(Order& cart);

    // order management / billing
    void searchOrder();
    void updateOrderStatus();
    void cancelOrder();
    void customerHistory();
    void viewBill();
    void payPendingBill();

    string askOrderId();

public:
    FoodOrderingSystem() : orders(&customers) {}   // aggregation pointer given to OrderManager
    void run();
};

void FoodOrderingSystem::run() {
    cout << "============================================\n";
    cout << "     FOOD ORDERING MANAGEMENT SYSTEM\n";
    cout << "  (Offline educational console application)\n";
    cout << "============================================\n";
    menu.load();
    customers.load();
    orders.load();
    cout << "Loaded " << menu.size() << " food item(s), " << customers.size()
         << " customer(s) and " << orders.totalOrders() << " order(s).\n";

    try {
        mainMenu();
    } catch (const InputClosedException&) {
        cout << "\nInput closed. Exiting.\n";
    }
}

void FoodOrderingSystem::mainMenu() {
    bool running = true;
    while (running) {
        cout << "\n=========== MAIN MENU ===========\n"
             << "1. Food Menu Management\n"
             << "2. Customer Management\n"
             << "3. Place New Order\n"
             << "4. Order Management\n"
             << "5. Billing and Payments\n"
             << "6. Reports\n"
             << "0. Exit\n";
        try {
            int ch = readInt("Enter choice: ", 0, 6);
            switch (ch) {
                case 1: foodMenuMenu(); break;
                case 2: customerMenu(); break;
                case 3: placeOrder(); break;
                case 4: orderManagementMenu(); break;
                case 5: billingMenu(); break;
                case 6: reportsMenu(); break;
                case 0: running = false; cout << "All records are saved. Goodbye!\n"; break;
            }
        } catch (const InputClosedException&) {
            throw;
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }
}

// ---------------- Food menu management ----------------

void FoodOrderingSystem::foodMenuMenu() {
    bool back = false;
    while (!back) {
        cout << "\n--- FOOD MENU MANAGEMENT ---\n"
             << "1. Display menu\n"
             << "2. Search food (by ID or name)\n"
             << "3. Add new food item\n"
             << "4. Update food name / price\n"
             << "5. Delete food item\n"
             << "6. Mark Available / Unavailable\n"
             << "0. Back\n";
        try {
            int ch = readInt("Enter choice: ", 0, 6);
            switch (ch) {
                case 1: menu.displayAll(); break;
                case 2: searchFood(); break;
                case 3: addFood(); break;
                case 4: updateFood(); break;
                case 5: deleteFood(); break;
                case 6: changeAvailability(); break;
                case 0: back = true; break;
            }
        } catch (const InputClosedException&) {
            throw;
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }
}

void FoodOrderingSystem::searchFood() {
    cout << "Search by: 1. ID   2. Name\n";
    int ch = readInt("Choice: ", 1, 2);
    if (ch == 1) menu.search(readInt("Enter food ID: ", 1, 999999));          // int overload
    else         menu.search(readNonEmpty("Enter name (or part of it): ", 40)); // string overload
}

void FoodOrderingSystem::addFood() {
    int id = readInt("Enter new food ID (number): ", 1, 999999);
    if (menu.exists(id)) throw invalid_argument("Food ID already exists. Choose a different ID.");
    string name = readNonEmpty("Food name: ", 23);
    string category = chooseCategory();
    double price = readPrice("Price (Rs.): ");
    menu.addItem(FoodItem(id, name, category, price));
    cout << "Food item added and saved.\n";
}

void FoodOrderingSystem::updateFood() {
    int id = readInt("Enter food ID to update: ", 1, 999999);
    FoodItem* f = menu.findById(id);
    if (!f) throw invalid_argument("Food item not found.");
    cout << "Current: " << f->getName() << " - Rs. " << money(f->getPrice()) << "\n";
    cout << "Update: 1. Name   2. Price   3. Both\n";
    int ch = readInt("Choice: ", 1, 3);
    if (ch == 1 || ch == 3) menu.updateItem(id, readNonEmpty("New name: ", 23));   // string overload
    if (ch == 2 || ch == 3) menu.updateItem(id, readPrice("New price (Rs.): "));   // double overload
    cout << "Food item updated and saved.\n";
}

void FoodOrderingSystem::deleteFood() {
    int id = readInt("Enter food ID to delete: ", 1, 999999);
    FoodItem* f = menu.findById(id);
    if (!f) throw invalid_argument("Food item not found.");
    if (readYesNo("Delete \"" + f->getName() + "\"? (y/n): ")) {
        menu.removeItem(id);
        cout << "Food item deleted. (Old orders keep their saved item names and prices.)\n";
    } else {
        cout << "Deletion cancelled.\n";
    }
}

void FoodOrderingSystem::changeAvailability() {
    int id = readInt("Enter food ID: ", 1, 999999);
    FoodItem* f = menu.findById(id);
    if (!f) throw invalid_argument("Food item not found.");
    cout << f->getName() << " is currently " << (f->isAvailable() ? "Available" : "Unavailable") << ".\n";
    cout << "Set to: 1. Available   2. Unavailable\n";
    int ch = readInt("Choice: ", 1, 2);
    menu.setAvailability(id, ch == 1);
    cout << "Availability updated and saved.\n";
}

// ---------------- Customer management ----------------

void FoodOrderingSystem::customerMenu() {
    bool back = false;
    while (!back) {
        cout << "\n--- CUSTOMER MANAGEMENT ---\n"
             << "1. Register new customer\n"
             << "2. Display all customers\n"
             << "3. Search customer (by ID or name)\n"
             << "0. Back\n";
        try {
            int ch = readInt("Enter choice: ", 0, 3);
            switch (ch) {
                case 1: registerCustomer(); break;
                case 2: customers.displayAll(); break;
                case 3: searchCustomer(); break;
                case 0: back = true; break;
            }
        } catch (const InputClosedException&) {
            throw;
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }
}

void FoodOrderingSystem::registerCustomer() {
    string name = readNonEmpty("Customer name: ", 30);
    string contact = readPhone("Contact number (10 digits): ");
    string id = customers.registerCustomer(name, contact);
    cout << "Customer registered. Customer ID: " << id << "\n";
}

void FoodOrderingSystem::searchCustomer() {
    cout << "Search by: 1. ID   2. Name\n";
    int ch = readInt("Choice: ", 1, 2);
    if (ch == 1) customers.searchById(toUpper(readNonEmpty("Enter customer ID (e.g. C001): ", 10)));
    else         customers.searchByName(readNonEmpty("Enter name (or part of it): ", 30));
}

// ---------------- Placing an order ----------------

void FoodOrderingSystem::placeOrder() {
    cout << "\n--- PLACE NEW ORDER ---\n";
    if (menu.size() == 0) throw runtime_error("The menu is empty. Add food items first.");
    string cid = toUpper(readNonEmpty("Enter customer ID (e.g. C001): ", 10));
    Customer* c = customers.findById(cid);
    if (!c) throw invalid_argument("Customer not found. Register the customer first.");
    cout << "Customer: " << c->getName() << "\n";

    menu.displayAll();
    Order cart(cid);        // the cart is an Order that is not saved until confirmed

    bool done = false;
    while (!done) {
        cout << "\n--- CART MENU ---\n"
             << "1. Add item\n"
             << "2. Change quantity\n"
             << "3. Remove item\n"
             << "4. View cart / bill\n"
             << "5. Confirm order\n"
             << "6. Cancel order (discard cart)\n"
             << "7. Show food menu again\n";
        try {
            int ch = readInt("Enter choice: ", 1, 7);
            switch (ch) {
                case 1: addToCart(cart); break;
                case 2: changeCartQuantity(cart); break;
                case 3: removeFromCart(cart); break;
                case 4: showCart(cart); break;
                case 5: if (confirmCart(cart)) done = true; break;
                case 6:
                    if (readYesNo("Discard this cart and cancel the order? (y/n): ")) {
                        cout << "Order cancelled. Nothing was saved.\n";
                        done = true;
                    }
                    break;
                case 7: menu.displayAll(); break;
            }
        } catch (const InputClosedException&) {
            throw;
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }
}

void FoodOrderingSystem::addToCart(Order& cart) {
    int id = readInt("Enter food ID: ", 1, 999999);
    FoodItem* f = menu.findById(id);
    if (!f) throw invalid_argument("Food item not found.");
    if (!f->isAvailable()) throw invalid_argument(f->getName() + " is currently unavailable.");
    int qty = readInt("Quantity (1-100): ", 1, MAX_QTY);
    cart.addItem(*f, qty);
    cout << qty << " x " << f->getName() << " added to cart.\n";
}

void FoodOrderingSystem::changeCartQuantity(Order& cart) {
    if (cart.isEmpty()) throw runtime_error("The cart is empty.");
    int id = readInt("Enter food ID in cart: ", 1, 999999);
    if (!cart.hasItem(id)) throw invalid_argument("That item is not in the cart.");
    int qty = readInt("New quantity (1-100; use 'Remove item' to delete): ", 1, MAX_QTY);
    cart.changeQuantity(id, qty);
    cout << "Quantity updated.\n";
}

void FoodOrderingSystem::removeFromCart(Order& cart) {
    if (cart.isEmpty()) throw runtime_error("The cart is empty.");
    int id = readInt("Enter food ID to remove: ", 1, 999999);
    if (!cart.removeItem(id)) throw invalid_argument("That item is not in the cart.");
    cout << "Item removed from cart.\n";
}

void FoodOrderingSystem::showCart(const Order& cart) {
    if (cart.isEmpty()) {
        cout << "The cart is empty.\n";
        return;
    }
    cout << "\nYour cart:\n";
    cart.displayBill();
}

bool FoodOrderingSystem::confirmCart(Order& cart) {
    if (cart.isEmpty()) throw runtime_error("The cart is empty. Add at least one item first.");
    showCart(cart);
    if (!readYesNo("Confirm this order? (y/n): ")) {
        cout << "Order not confirmed. Returning to cart.\n";
        return false;
    }
    string method = choosePaymentMethod();
    bool paidNow = readYesNo("Mark payment as received now? (y = Paid, n = Pending): ");
    string id = orders.confirmOrder(cart, method, paidNow);
    cout << "\nOrder confirmed and saved! Order ID: " << id << "\n";
    orders.displayBillFor(id);
    return true;
}

// ---------------- Order management ----------------

string FoodOrderingSystem::askOrderId() {
    return toUpper(readNonEmpty("Enter order ID (e.g. O1001): ", 10));
}

void FoodOrderingSystem::orderManagementMenu() {
    bool back = false;
    while (!back) {
        cout << "\n--- ORDER MANAGEMENT ---\n"
             << "1. Display all orders\n"
             << "2. Search order by ID / view details\n"
             << "3. Update order status\n"
             << "4. Cancel an order\n"
             << "5. Customer order history\n"
             << "0. Back\n";
        try {
            int ch = readInt("Enter choice: ", 0, 5);
            switch (ch) {
                case 1: orders.displayAllOrders(); break;
                case 2: searchOrder(); break;
                case 3: updateOrderStatus(); break;
                case 4: cancelOrder(); break;
                case 5: customerHistory(); break;
                case 0: back = true; break;
            }
        } catch (const InputClosedException&) {
            throw;
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }
}

void FoodOrderingSystem::searchOrder() {
    orders.displayOrderDetails(askOrderId());
}

void FoodOrderingSystem::updateOrderStatus() {
    string id = askOrderId();
    if (!orders.hasOrder(id)) throw invalid_argument("Order not found: " + id);
    const vector<string>& st = orderStatuses();
    cout << "Select new status:\n";
    for (size_t i = 0; i < st.size(); ++i) cout << "  " << (i + 1) << ". " << st[i] << "\n";
    int ch = readInt("Choice: ", 1, (int)st.size());
    orders.updateStatus(id, st[ch - 1]);
    cout << "Order " << id << " status updated to " << st[ch - 1] << " and saved.\n";
}

void FoodOrderingSystem::cancelOrder() {
    string id = askOrderId();
    if (!orders.hasOrder(id)) throw invalid_argument("Order not found: " + id);
    if (readYesNo("Cancel order " + id + "? (y/n): ")) {
        orders.updateStatus(id, "Cancelled");
        cout << "Order " << id << " cancelled. It will not be counted as revenue.\n";
    } else {
        cout << "Cancellation aborted.\n";
    }
}

void FoodOrderingSystem::customerHistory() {
    string cid = toUpper(readNonEmpty("Enter customer ID (e.g. C001): ", 10));
    Customer* c = customers.findById(cid);
    if (!c) throw invalid_argument("Customer not found.");
    cout << "Order history of " << c->getName() << " (" << cid << "):\n";
    orders.displayCustomerHistory(cid);
}

// ---------------- Billing and payments ----------------

void FoodOrderingSystem::billingMenu() {
    bool back = false;
    while (!back) {
        cout << "\n--- BILLING AND PAYMENTS ---\n"
             << "1. View itemised bill of an order\n"
             << "2. Pay a pending bill (Cash / UPI demo)\n"
             << "3. Display all payment records\n"
             << "0. Back\n";
        try {
            int ch = readInt("Enter choice: ", 0, 3);
            switch (ch) {
                case 1: viewBill(); break;
                case 2: payPendingBill(); break;
                case 3: orders.displayAllPayments(); break;
                case 0: back = true; break;
            }
        } catch (const InputClosedException&) {
            throw;
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }
}

void FoodOrderingSystem::viewBill() {
    orders.displayBillFor(askOrderId());
}

void FoodOrderingSystem::payPendingBill() {
    string id = askOrderId();
    if (!orders.hasOrder(id)) throw invalid_argument("Order not found: " + id);
    string method = choosePaymentMethod();
    orders.payForOrder(id, method);
    cout << "Payment recorded as Paid (" << method << ") for order " << id << ". (Demonstration only.)\n";
}

// ---------------- Reports ----------------

void FoodOrderingSystem::reportsMenu() {
    bool back = false;
    while (!back) {
        cout << "\n--- REPORTS ---\n"
             << "1. Total orders and total revenue\n"
             << "2. Pending orders\n"
             << "3. Most frequently ordered items\n"
             << "4. Daily order summary\n"
             << "0. Back\n";
        try {
            int ch = readInt("Enter choice: ", 0, 4);
            switch (ch) {
                case 1: orders.displaySummaryReport(); break;
                case 2: orders.displayPendingOrders(); break;
                case 3: orders.displayTopItems(); break;
                case 4: orders.displayDailySummary(); break;
                case 0: back = true; break;
            }
        } catch (const InputClosedException&) {
            throw;
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }
}

// ============================================================
// SECTION 9: main
// ============================================================

int main() {
    cout << fixed << setprecision(2);
    FoodOrderingSystem app;
    app.run();
    return 0;
}
