#include "../backend/shelter.hpp"
#include <cassert>
#include <iostream>

// Tests are separate from the code you explain in the presentation.
template<typename Action>
void mustReject(Action action) {
    bool rejected = false;
    try { action(); }
    catch (const ShelterException&) { rejected = true; }
    assert(rejected);
}

int main() {
    Shelter shelter;
    shelter.reset();
    assert(shelter.getOccupancy() == 5);
    assert(shelter.getCapacity() == 50);
    shelter.admit("Demo family", 3);
    assert(shelter.getOccupancy() == 8);

    shelter.allocate(3, 1, 5);
    assert(shelter.getResources()[0]->getQuantity() == 45);
    assert(shelter.getAllocations().size() == 1);
    mustReject([&]() { shelter.allocate(3, 3, 1); }); // Expired food.
    mustReject([&]() { shelter.allocate(3, 1, 999); });
    mustReject([&]() { shelter.allocate(999, 1, 1); });
    assert(shelter.getAllocations().size() == 1);
    assert(shelter.getResources()[0]->getQuantity() == 45);

    shelter.restock(1, 10, "");
    shelter.restock(1, 5, "College club");
    assert(shelter.getResources()[0]->getQuantity() == 60);
    assert(shelter.getRestocks().size() == 2);
    mustReject([&]() { shelter.restock(1, -1, ""); });
    assert(shelter.getRestocks().size() == 2);

    shelter.admit("Large family A", 20);
    shelter.admit("Large family B", 20);
    assert(shelter.getOccupancy() == 48);
    mustReject([&]() { shelter.admit("Too many people", 3); });
    assert(shelter.getOccupancy() == 48);
    mustReject([&]() { shelter.admit(" ", 1); });
    mustReject([&]() { shelter.addFood("Invalid date", 1, "2026-02-30"); });

    Water water(10, "Water", 5);
    Food food(11, "Meal", 5, today());
    Resource* first = &water;
    Resource* second = &food;
    assert(first->getUnit() == "litres");
    assert(second->getUnit() == "packs");
    assert(food.isUsable()); // Valid through its expiry day.
    water.addStock(2);
    water.addStock(3, "Donor");
    assert(water.getQuantity() == 10);
    water.setName("Clean water");
    assert(water.getName() == "Clean water");
    assert(validDate("2024-02-29") && !validDate("2025-02-29"));
    shelter.reset();
    assert(shelter.getOccupancy() == 5 && shelter.getAllocations().empty());
    std::cout << "All simple shelter tests passed.\n";
}
