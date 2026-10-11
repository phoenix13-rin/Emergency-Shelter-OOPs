#include "shelter.hpp"
#include <iostream>

int readNumber(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    if (!std::getline(std::cin, input)) throw ShelterException("Input closed.");
    try {
        std::size_t used = 0;
        int value = std::stoi(input, &used);
        if (used != input.size()) throw ShelterException("Enter a whole number.");
        return value;
    }
    catch (const std::invalid_argument&) { throw ShelterException("Enter a whole number."); }
    catch (const std::out_of_range&) { throw ShelterException("Number is too large."); }
}

std::string readText(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

void showShelter(const Shelter& shelter) {
    std::cout << "\nPeople: " << shelter.getOccupancy()
              << " / " << shelter.getCapacity() << "\n\nRESOURCES\n";

    // POLYMORPHISM: the same Resource pointer calls Water or Food methods.
    for (const auto& resource : shelter.getResources()) {
        std::cout << resource->getId() << ". " << resource->getName()
                  << " | " << resource->getType() << " | "
                  << resource->getQuantity() << ' ' << resource->getUnit()
                  << " | " << (resource->isUsable() ? "Usable" : "Blocked") << '\n';
    }

    std::cout << "\nHOUSEHOLDS\n";
    for (const Resident& person : shelter.getResidents()) {
        std::cout << person.getId() << ". " << person.getName()
                  << " | " << person.getMembers() << " people\n";
    }
    std::cout << "Allocations recorded: " << shelter.getAllocations().size()
              << "\nRestocks recorded: " << shelter.getRestocks().size() << '\n';
}

int main() {
    Shelter shelter(50);
    shelter.reset();
    std::cout << "Emergency Shelter - C++ OOP Console\n";

    while (std::cin) {
        std::cout << "\n1. Show shelter\n2. Admit household\n3. Add water batch\n"
                  << "4. Add food batch\n5. Allocate supplies\n6. Restock supplies\n0. Exit\n";
        try {
            int choice = readNumber("Choice: ");
            if (choice == 0) break;
            if (choice == 1) {
                showShelter(shelter);
            }
            else if (choice == 2) {
                std::string name = readText("Household name: ");
                int members = readNumber("Number of people: ");
                shelter.admit(name, members);
                std::cout << "Household admitted.\n";
            }
            else if (choice == 3) {
                std::string name = readText("Water batch name: ");
                int quantity = readNumber("Litres: ");
                shelter.addWater(name, quantity);
                std::cout << "Water added.\n";
            }
            else if (choice == 4) {
                std::string name = readText("Food batch name: ");
                int quantity = readNumber("Packs: ");
                std::string expiry = readText("Expiry (YYYY-MM-DD): ");
                shelter.addFood(name, quantity, expiry);
                std::cout << "Food added.\n";
            }
            else if (choice == 5) {
                int resident = readNumber("Household ID: ");
                int resource = readNumber("Resource ID: ");
                int quantity = readNumber("Quantity: ");
                shelter.allocate(resident, resource, quantity);
                std::cout << "Supplies allocated.\n";
            }
            else if (choice == 6) {
                int resource = readNumber("Resource ID: ");
                int quantity = readNumber("Quantity to add: ");
                std::string donor = readText("Donor name (leave blank for ordinary stock): ");
                shelter.restock(resource, quantity, donor);
                std::cout << "Stock added using " << (donor.empty() ? "addStock(amount)" : "addStock(amount, donor)") << ".\n";
            }
            else {
                std::cout << "Choose a listed option.\n";
            }
        }
        catch (const ShelterException& error) {
            std::cout << "Operation rejected: " << error.what() << '\n';
        }
    }
}
