#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <ctime>

using namespace std;

// Forward declarations
class GroceryItem;
class Customer;

// GroceryItem class representing an item in the store
class GroceryItem {
private:
    string name;
    double price;
    string category;
    int stock;
    string expirationDate; // New: Expiration date for perishable items

public:
    GroceryItem(const string& name, double price, const string& category, int stock, const string& expirationDate = "")
            : name(name), price(price), category(category), stock(stock), expirationDate(expirationDate) {}

    // Getters
    const string& getName() const { return name; }
    double getPrice() const { return price; }
    const string& getCategory() const { return category; }
    int getStock() const { return stock; }
    const string& getExpirationDate() const { return expirationDate; }

    // Purchase functionality
    void purchaseItem(int quantity) {
        if (quantity > stock) {
            throw runtime_error("Insufficient stock for this item.");
        }
        stock -= quantity;
    }

    // Restock functionality
    void restockItem(int quantity) {
        stock += quantity;
    }

    // Check if item is expired
    bool isExpired() const {
        if (expirationDate.empty()) return false; // Non-perishable items have no expiration date
        time_t now = time(nullptr);
        tm currentDate = *localtime(&now);

        tm expiryDate = {};
        istringstream ss(expirationDate);
        ss >> get_time(&expiryDate, "%Y-%m-%d");

        return mktime(&expiryDate) < mktime(&currentDate);
    }

    // Display item details
    void display() const {
        cout << "Name: " << name << ", Price: $" << fixed << setprecision(2) << price
             << ", Category: " << category << ", Stock: " << stock
             << (expirationDate.empty() ? "" : ", Expiration Date: " + expirationDate)
             << (isExpired() ? " (EXPIRED)" : "") << "\n";
    }
};

// Customer class representing a customer
class Customer {
private:
    string name;
    int id;
    string membershipType; // Regular or Premium
    vector<pair<shared_ptr<GroceryItem>, int>> purchasedItems; // Item and quantity
    double loyaltyPoints; // New: Loyalty points for rewards

public:
    constexpr static const double REGULAR_DISCOUNT = 0.0;   // No discount for Regular members
    constexpr static const double PREMIUM_DISCOUNT = 0.1;   // 10% discount for Premium members
    constexpr static const double LOYALTY_RATE = 0.01;      // 1% of total spending as loyalty points

    Customer(const string& name, int id, const string& membershipType)
            : name(name), id(id), membershipType(membershipType), loyaltyPoints(0.0) {}

    // Getters
    const string& getName() const { return name; }
    int getID() const { return id; }
    const string& getMembershipType() const { return membershipType; }
    double getLoyaltyPoints() const { return loyaltyPoints; }

    // Purchase an item
    void purchaseItem(shared_ptr<GroceryItem> item, int quantity) {
        item->purchaseItem(quantity);
        purchasedItems.emplace_back(item, quantity);

        // Update loyalty points
        double subtotal = item->getPrice() * quantity;
        loyaltyPoints += subtotal * LOYALTY_RATE;

        cout << "Item purchased successfully.\n";
    }

    // Display purchased items and total bill
    void displayPurchases() const {
        cout << "Purchases by " << name << ":\n";
        if (purchasedItems.empty()) {
            cout << "No items purchased.\n";
        } else {
            double totalBill = 0.0;
            for (const auto& [item, quantity] : purchasedItems) {
                double price = item->getPrice();
                double subtotal = price * quantity;
                cout << "Item: " << item->getName() << ", Quantity: " << quantity
                     << ", Subtotal: $" << fixed << setprecision(2) << subtotal << "\n";
                totalBill += subtotal;
            }

            // Apply discount based on membership type
            double discountRate = (membershipType == "Premium") ? PREMIUM_DISCOUNT : REGULAR_DISCOUNT;
            double discount = totalBill * discountRate;
            double finalBill = totalBill - discount;

            cout << "Total Bill: $" << fixed << setprecision(2) << totalBill
                 << ", Discount: $" << fixed << setprecision(2) << discount
                 << ", Final Bill: $" << fixed << setprecision(2) << finalBill
                 << ", Loyalty Points: " << loyaltyPoints << "\n";
        }
    }
};

// GroceryStore class managing items and customers
class GroceryStore {
private:
    vector<shared_ptr<GroceryItem>> items;
    vector<shared_ptr<Customer>> customers;
    vector<tuple<string, string, int, double>> salesHistory; // New: Sales history (itemName, customerName, quantity, total)

public:
    // Add an item to the store
    void addItem(const string& name, double price, const string& category, int stock, const string& expirationDate = "") {
        items.push_back(make_shared<GroceryItem>(name, price, category, stock, expirationDate));
    }

    // Add a customer to the store
    void addCustomer(const string& name, int id, const string& membershipType) {
        customers.push_back(make_shared<Customer>(name, id, membershipType));
    }

    // Find an item by name
    shared_ptr<GroceryItem> findItem(const string& name) {
        auto it = find_if(items.begin(), items.end(),
                          [&name](const shared_ptr<GroceryItem>& item) { return item->getName() == name; });
        if (it == items.end()) {
            throw runtime_error("Item not found.");
        }
        return *it;
    }

    // Find a customer by ID
    shared_ptr<Customer> findCustomer(int id) {
        auto it = find_if(customers.begin(), customers.end(),
                          [id](const shared_ptr<Customer>& c) { return c->getID() == id; });
        if (it == customers.end()) {
            throw runtime_error("Customer not found.");
        }
        return *it;
    }

    // Display all items
    void displayItems() const {
        cout << "Grocery Items:\n";
        for (const auto& item : items) {
            item->display();
        }
    }

    // Display all customers
    void displayCustomers() const {
        cout << "Customers:\n";
        for (const auto& customer : customers) {
            cout << "Name: " << customer->getName() << ", ID: " << customer->getID()
                 << ", Membership Type: " << customer->getMembershipType()
                 << ", Loyalty Points: " << customer->getLoyaltyPoints() << "\n";
        }
    }

    // Save store data to a file
    void saveToFile(const string& filename) const {
        ofstream file(filename);
        if (!file) {
            throw runtime_error("Failed to open file for saving.");
        }

        // Save items
        file << "Items:\n";
        for (const auto& item : items) {
            file << item->getName() << "|" << item->getPrice() << "|" << item->getCategory() << "|"
                 << item->getStock() << "|" << item->getExpirationDate() << "\n";
        }

        // Save customers
        file << "Customers:\n";
        for (const auto& customer : customers) {
            file << customer->getName() << "|" << customer->getID() << "|" << customer->getMembershipType() << "|"
                 << customer->getLoyaltyPoints() << "\n";
        }

        file.close();
        cout << "Store data saved to " << filename << "\n";
    }

    // Load store data from a file
    void loadFromFile(const string& filename) {
        ifstream file(filename);
        if (!file) {
            throw runtime_error("Failed to open file for loading.");
        }

        string line;
        while (getline(file, line)) {
            istringstream iss(line);
            string type;
            getline(iss, type, ':');

            if (type == "Items") {
                while (getline(file, line) && !line.empty()) {
                    istringstream itemStream(line);
                    string name, category, expirationDate;
                    double price;
                    int stock;
                    getline(itemStream, name, '|');
                    itemStream >> price;
                    itemStream.ignore(); // Ignore delimiter
                    getline(itemStream, category, '|');
                    itemStream >> stock;
                    itemStream.ignore(); // Ignore delimiter
                    getline(itemStream, expirationDate);

                    addItem(name, price, category, stock, expirationDate);
                }
            } else if (type == "Customers") {
                while (getline(file, line) && !line.empty()) {
                    istringstream customerStream(line);
                    string name, membershipType;
                    int id;
                    double loyaltyPoints;
                    getline(customerStream, name, '|');
                    customerStream >> id;
                    customerStream.ignore(); // Ignore delimiter
                    getline(customerStream, membershipType, '|');
                    customerStream >> loyaltyPoints;

                    auto customer = make_shared<Customer>(name, id, membershipType);
                    customer->purchaseItem(nullptr, loyaltyPoints / Customer::LOYALTY_RATE); // Simulate loyalty points
                    customers.push_back(customer);
                }
            }
        }

        file.close();
        cout << "Store data loaded from " << filename << "\n";
    }

    // New: Alert for low stock items
    void checkLowStock(int threshold) const {
        cout << "Low Stock Alert:\n";
        bool found = false;
        for (const auto& item : items) {
            if (item->getStock() < threshold) {
                cout << "Item: " << item->getName() << ", Stock: " << item->getStock() << "\n";
                found = true;
            }
        }
        if (!found) {
            cout << "No items below the stock threshold.\n";
        }
    }

    // New: Search for items by name or category
    void searchItems(const string& query) const {
        cout << "Search Results:\n";
        bool found = false;
        for (const auto& item : items) {
            if (item->getName().find(query) != string::npos ||
                item->getCategory().find(query) != string::npos) {
                item->display();
                found = true;
            }
        }
        if (!found) {
            cout << "No matching items found.\n";
        }
    }

    // New: Record sales history
    void recordSale(const string& itemName, const string& customerName, int quantity, double total) {
        salesHistory.emplace_back(itemName, customerName, quantity, total);
    }

    // New: Display sales history
    void displaySalesHistory() const {
        cout << "Sales History:\n";
        if (salesHistory.empty()) {
            cout << "No sales recorded.\n";
        } else {
            for (const auto& [itemName, customerName, quantity, total] : salesHistory) {
                cout << "Item: " << itemName << ", Customer: " << customerName
                     << ", Quantity: " << quantity << ", Total: $" << fixed << setprecision(2) << total << "\n";
            }
        }
    }
};

// Main function with dynamic menu system
int main() {
    try {
        GroceryStore store;
        int choice;

        do {
            cout << "\n===== Grocery Management System =====\n";
            cout << "1. Add Item\n";
            cout << "2. Add Customer\n";
            cout << "3. Purchase Item\n";
            cout << "4. Restock Item\n";
            cout << "5. Display All Items\n";
            cout << "6. Display All Customers\n";
            cout << "7. View Customer Purchases\n";
            cout << "8. Save Data to File\n";
            cout << "9. Load Data from File\n";
            cout << "10. Check Low Stock\n";       // New: Low stock alert
            cout << "11. Search Items\n";         // New: Search functionality
            cout << "12. View Sales History\n";   // New: Sales history
            cout << "13. Exit\n";
            cout << "Enter your choice: ";
            cin >> choice;
            cin.ignore(); // Clear input buffer

            switch (choice) {
                case 1: {
                    string name, category, expirationDate;
                    double price;
                    int stock;
                    cout << "Enter Name: ";
                    getline(cin, name);
                    cout << "Enter Price: ";
                    cin >> price;
                    cin.ignore();
                    cout << "Enter Category: ";
                    getline(cin, category);
                    cout << "Enter Stock: ";
                    cin >> stock;
                    cout << "Enter Expiration Date (YYYY-MM-DD, leave blank if none): ";
                    cin.ignore();
                    getline(cin, expirationDate);
                    store.addItem(name, price, category, stock, expirationDate);
                    cout << "Item added successfully.\n";
                    break;
                }
                case 2: {
                    string name, membershipType;
                    int id;
                    cout << "Enter Name: ";
                    getline(cin, name);
                    cout << "Enter ID: ";
                    cin >> id;
                    cin.ignore();
                    cout << "Enter Membership Type (Regular/Premium): ";
                    getline(cin, membershipType);
                    store.addCustomer(name, id, membershipType);
                    cout << "Customer added successfully.\n";
                    break;
                }
                case 3: {
                    string itemName, customerName;
                    int customerId, quantity;
                    cout << "Enter Customer ID: ";
                    cin >> customerId;
                    cin.ignore();
                    cout << "Enter Item Name: ";
                    getline(cin, itemName);
                    cout << "Enter Quantity: ";
                    cin >> quantity;

                    auto customer = store.findCustomer(customerId);
                    auto item = store.findItem(itemName);

                    // Bulk purchase discount
                    double price = item->getPrice();
                    double total = price * quantity;
                    if (quantity >= 10) {
                        total *= 0.95; // 5% discount for bulk purchases
                        cout << "Applied 5% bulk purchase discount.\n";
                    }

                    customer->purchaseItem(item, quantity);
                    store.recordSale(itemName, customer->getName(), quantity, total);

                    // Check for expired items
                    if (item->isExpired()) {
                        cout << "Warning: This item is expired!\n";
                    }
                    break;
                }
                case 4: {
                    string itemName;
                    int quantity;
                    cout << "Enter Item Name: ";
                    getline(cin, itemName);
                    cout << "Enter Quantity to Restock: ";
                    cin >> quantity;
                    auto item = store.findItem(itemName);
                    item->restockItem(quantity);
                    cout << "Item restocked successfully.\n";
                    break;
                }
                case 5:
                    store.displayItems();
                    break;
                case 6:
                    store.displayCustomers();
                    break;
                case 7: {
                    int customerId;
                    cout << "Enter Customer ID: ";
                    cin >> customerId;
                    auto customer = store.findCustomer(customerId);
                    customer->displayPurchases();
                    break;
                }
                case 8: {
                    string filename;
                    cout << "Enter filename to save data: ";
                    getline(cin, filename);
                    store.saveToFile(filename);
                    break;
                }
                case 9: {
                    string filename;
                    cout << "Enter filename to load data: ";
                    getline(cin, filename);
                    store.loadFromFile(filename);
                    break;
                }
                case 10: {
                    int threshold;
                    cout << "Enter stock threshold: ";
                    cin >> threshold;
                    store.checkLowStock(threshold);
                    break;
                }
                case 11: {
                    string query;
                    cout << "Enter search query (name/category): ";
                    getline(cin, query);
                    store.searchItems(query);
                    break;
                }
                case 12:
                    store.displaySalesHistory();
                    break;
                case 13:
                    cout << "Exiting...\n";
                    break;
                default:
                    cout << "Invalid choice. Please try again.\n";
            }
        } while (choice != 13);

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}