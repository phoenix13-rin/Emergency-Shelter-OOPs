#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

// Custom exception: used for invalid input and rejected operations.
class ShelterException : public std::runtime_error {
public:
    explicit ShelterException(const std::string& message)
        : std::runtime_error(message) {}
};

inline void checkName(const std::string& name) {
    if (name.empty() || name.size() > 80 ||
        name.find_first_not_of(" \t\r\n") == std::string::npos) {
        throw ShelterException("Name must contain 1 to 80 characters.");
    }
    for (unsigned char ch : name) {
        if (ch < 32) throw ShelterException("Name contains an invalid character.");
    }
}

// Helper functions, not OOP concepts. Dates use YYYY-MM-DD.
inline std::string today(int daysFromNow = 0) {
    std::time_t time = std::time(nullptr) + daysFromNow * 86400;
    std::tm date = {};
#ifdef _WIN32
    localtime_s(&date, &time);
#else
    localtime_r(&time, &date);
#endif
    std::ostringstream output;
    output << std::put_time(&date, "%Y-%m-%d");
    return output.str();
}

inline bool validDate(const std::string& date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
    for (int i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && (date[i] < '0' || date[i] > '9')) return false;
    }
    int year = std::stoi(date.substr(0, 4));
    int month = std::stoi(date.substr(5, 2));
    int day = std::stoi(date.substr(8, 2));
    if (year < 1900 || month < 1 || month > 12) return false;
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) days[1] = 29;
    return day >= 1 && day <= days[month - 1];
}

// ABSTRACTION / ISP: only resources with expiry implement this interface.
class IExpirable {
public:
    virtual bool isExpired() const = 0;
    virtual std::string getExpiry() const = 0;
    virtual ~IExpirable() = default;
};

// ABSTRACTION: pure virtual methods make Resource an abstract class.
class Resource {
private:
    // ENCAPSULATION: outside code cannot directly change these fields.
    int id;
    std::string name;
    int quantity;

public:
    Resource(int resourceId, const std::string& resourceName, int stock)
        : id(resourceId), name(resourceName), quantity(stock) {
        checkName(name);
        if (stock < 0 || stock > 100000) {
            throw ShelterException("Stock must be between 0 and 100000.");
        }
    }

    virtual ~Resource() = default;
    int getId() const { return id; }
    std::string getName() const { return name; }
    int getQuantity() const { return quantity; }

    void setName(const std::string& newName) {
        checkName(newName);
        name = newName;
    }

    // OVERLOADING: same method name, different parameters.
    void addStock(int amount) {
        if (amount <= 0 || amount > 100000 - quantity) {
            throw ShelterException("Invalid amount or total stock exceeds 100000.");
        }
        quantity += amount;
    }

    void addStock(int amount, const std::string& donor) {
        checkName(donor);
        addStock(amount); // Reuse the first overload's validation.
    }

    void consume(int amount) {
        if (amount <= 0) throw ShelterException("Quantity must be positive.");
        if (!isUsable()) throw ShelterException("This resource is expired or empty.");
        if (amount > quantity) throw ShelterException("Not enough stock.");
        quantity -= amount;
    }

    virtual std::string getType() const = 0;
    virtual std::string getUnit() const = 0;
    virtual bool isUsable() const { return quantity > 0; }
};

// INHERITANCE: Water is a Resource.
class Water : public Resource {
public:
    Water(int id, const std::string& name, int quantity)
        : Resource(id, name, quantity) {}

    // OVERRIDING: provide the derived class's own implementations.
    std::string getType() const override { return "Water"; }
    std::string getUnit() const override { return "litres"; }
};

// Food is a Resource and also implements the expiry interface.
class Food : public Resource, public IExpirable {
private:
    std::string expiry;

public:
    Food(int id, const std::string& name, int quantity, const std::string& date)
        : Resource(id, name, quantity), expiry(date) {
        if (!validDate(date)) throw ShelterException("Enter a valid expiry date.");
    }

    std::string getType() const override { return "Food"; }
    std::string getUnit() const override { return "packs"; }
    std::string getExpiry() const override { return expiry; }
    bool isExpired() const override { return expiry < today(); }

    bool isUsable() const override {
        return getQuantity() > 0 && !isExpired();
    }
};

#endif
