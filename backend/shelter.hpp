#ifndef SHELTER_HPP
#define SHELTER_HPP

#include "resource.hpp"
#include <memory>
#include <vector>

// ENCAPSULATION: a household's member count cannot be changed directly.
class Resident {
private:
    int id;
    std::string name;
    int members;

public:
    Resident(int residentId, const std::string& residentName, int count)
        : id(residentId), name(residentName), members(count) {
        checkName(name);
        if (count < 1 || count > 20) {
            throw ShelterException("A household must have 1 to 20 people.");
        }
    }
    int getId() const { return id; }
    std::string getName() const { return name; }
    int getMembers() const { return members; }
};

class Capacity {
private:
    int limit;

public:
    explicit Capacity(int maximum) : limit(maximum) {
        if (maximum < 1) throw ShelterException("Capacity must be positive.");
    }
    int getLimit() const { return limit; }
    void check(int current, int incoming) const {
        if (current + incoming > limit) {
            throw ShelterException("Not enough space in the shelter.");
        }
    }
};

// ASSOCIATION: records link existing residents and resources by their IDs.
struct Allocation {
    int residentId;
    int resourceId;
    int quantity;
};

struct Restock {
    int resourceId;
    int quantity;
    std::string donor;
};

class Shelter {
private:
    Capacity capacity; // COMPOSITION: a Shelter owns its Capacity component.
    std::vector<std::unique_ptr<Resource> > resources;
    std::vector<Resident> residents;
    std::vector<Allocation> allocations;
    std::vector<Restock> restocks;
    int nextResourceId;
    int nextResidentId;

    Resource& findResource(int id) {
        for (const auto& resource : resources) {
            if (resource->getId() == id) return *resource;
        }
        throw ShelterException("Resource not found.");
    }

public:
    explicit Shelter(int maximum = 50)
        : capacity(maximum), nextResourceId(1), nextResidentId(1) {}

    int getCapacity() const { return capacity.getLimit(); }

    int getOccupancy() const {
        int total = 0;
        for (const Resident& person : residents) {
            total += person.getMembers();
        }
        return total;
    }

    const std::vector<std::unique_ptr<Resource> >& getResources() const { return resources; }
    const std::vector<Resident>& getResidents() const { return residents; }
    const std::vector<Allocation>& getAllocations() const { return allocations; }
    const std::vector<Restock>& getRestocks() const { return restocks; }

    void addWater(const std::string& name, int quantity) {
        std::unique_ptr<Resource> water(new Water(nextResourceId, name, quantity));
        resources.push_back(std::move(water));
        nextResourceId++;
    }

    void addFood(const std::string& name, int quantity, const std::string& expiry) {
        std::unique_ptr<Resource> food(new Food(nextResourceId, name, quantity, expiry));
        resources.push_back(std::move(food));
        nextResourceId++;
    }

    void admit(const std::string& name, int members) {
        Resident person(nextResidentId, name, members);
        capacity.check(getOccupancy(), members);
        residents.push_back(person);
        nextResidentId++;
    }

    void allocate(int residentId, int resourceId, int quantity) {
        bool found = false;
        for (const Resident& person : residents) {
            if (person.getId() == residentId) found = true;
        }
        if (!found) throw ShelterException("Resident not found.");

        Resource& resource = findResource(resourceId);
        // Validate before changing stock or history.
        if (quantity <= 0) throw ShelterException("Quantity must be positive.");
        if (!resource.isUsable()) throw ShelterException("This resource is expired or empty.");
        if (quantity > resource.getQuantity()) throw ShelterException("Not enough stock.");

        allocations.push_back({residentId, resourceId, quantity});
        resource.consume(quantity);
    }

    void restock(int resourceId, int quantity, const std::string& donor) {
        Resource& resource = findResource(resourceId);
        if (quantity <= 0 || quantity > 100000 - resource.getQuantity()) {
            throw ShelterException("Invalid amount or total stock exceeds 100000.");
        }
        if (!donor.empty()) checkName(donor);
        restocks.push_back({resourceId, quantity, donor});

        if (donor.empty()) {
            resource.addStock(quantity); // One-argument overload.
        } else {
            resource.addStock(quantity, donor); // Two-argument overload.
        }
    }

    void reset() {
        resources.clear();
        residents.clear();
        allocations.clear();
        restocks.clear();
        nextResourceId = 1;
        nextResidentId = 1;
        addWater("Drinking water", 50);
        addFood("Meal packs", 30, today(30));
        addFood("Expired food batch", 5, today(-1));
        admit("Asha family", 3);
        admit("Ravi family", 2);
    }
};

#endif
