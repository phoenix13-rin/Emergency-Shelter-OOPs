# OOP guide — simple version

## Your opening explanation

“My project manages food, water and households in an emergency shelter. I used a Resource base class for shared stock behavior, Water and Food subclasses for differences, and a Shelter class to manage admission and allocation. The frontend is a simple way to demonstrate the C++ methods.”

## 1. Classes and objects

A class defines data and behavior. An object is a specific instance.

```cpp
Water water(1, "Drinking water", 50);
Shelter shelter(50);
```

`Water` and `Shelter` are classes. `water` and `shelter` are objects. The Water object has its own ID, name and quantity.

## 2. Encapsulation

Open `resource.hpp`, find `class Resource`.

```cpp
private:
    int id;
    std::string name;
    int quantity;
```

Outside code cannot write `water.quantity = -5`. It must call methods such as `addStock()` and `consume()`. Those methods validate values before modifying stock. Getters expose the values; `setName()` validates a name change. Quantity deliberately has no unrestricted setter.

Say: “Encapsulation protects the resource's data and ensures stock changes pass through validation.”

## 3. Abstraction

```cpp
virtual std::string getType() const = 0;
virtual std::string getUnit() const = 0;
```

`= 0` makes a method pure virtual. Resource is abstract because it has these methods. You cannot construct Resource directly. Concrete subclasses must implement them.

Say: “The base class states what each resource must provide without deciding the answer for every type.”

## 4. Inheritance

```cpp
class Water : public Resource
class Food : public Resource, public IExpirable
```

Water and Food reuse Resource's private-state access through getters, stock addition and consumption behavior. A derived class cannot directly access Resource's private fields. Food additionally implements an expiry contract.

Say: “Water and Food are types of Resource, so they inherit common stock behavior.”

## 5. Overriding

```cpp
std::string Water::getUnit() const // Actual definition is inside Water.
```

In the file, the inline definition is:

```cpp
std::string getUnit() const override { return "litres"; }
```

Food's version returns `"packs"`. Food also overrides `isUsable()` to require non-expired stock. `override` asks the compiler to check that a matching virtual function exists in a base class.

Say: “Overriding lets each derived resource define its own behavior for a base virtual method.”

## 6. Runtime polymorphism

The actual program stores resource objects as `unique_ptr<Resource>` and calls virtual methods through those pointers. To explain it without smart-pointer syntax, use this small example:

```cpp
Water water(1, "Water", 50);
Food food(2, "Meals", 30, today(30));
Resource* resource = &water;
std::cout << resource->getUnit(); // litres
resource = &food;
std::cout << resource->getUnit(); // packs
```

A base pointer refers to different derived objects. The concrete object selects the virtual implementation at runtime. The console's `showShelter()` loop demonstrates this with real project data. Allocation also uses a base reference; an expired Food object is rejected by its overridden usability rule.

Say: “The same call through Resource produces different behavior depending on the actual object.”

## 7. Compile-time polymorphism: overloading

```cpp
void addStock(int amount);
void addStock(int amount, const std::string& donor);
```

Examples:

```cpp
water.addStock(10);
water.addStock(5, "College club");
```

The compiler chooses the overload from the argument list. The donor version validates the donor and reuses the one-argument version. Shelter records the donor in restock history. The Resource object itself stores stock, not a list of donors.

Live demo: restock once with a blank donor, then with College club. `Shelter::restock()` contains the two calls.

Say: “Overloading uses the same name with different parameters; overriding replaces inherited virtual behavior.”

## 8. Exception handling

```cpp
throw ShelterException("Not enough stock.");
```

`ShelterException` inherits from `std::runtime_error`. Its constructor stores the error message. The console handles it with:

```cpp
catch (const ShelterException& error) {
    std::cout << "Operation rejected: " << error.what() << '\n';
}
```

The web adapter catches the same exception and returns the message to the frontend. Ordinary validation occurs before stock or history changes.

Say: “An invalid operation throws a custom exception. The caller catches it and reports the reason.”

Exception handling is useful supporting behavior, not one of the four OOP pillars.

## 9. Composition

Open `shelter.hpp`:

```cpp
class Shelter {
private:
    Capacity capacity;
    // Other members...
};
```

The Shelter contains and owns a Capacity object. It is a has-a relationship. The component is constructed and destroyed with the owner. Shelter should not inherit from Capacity: a shelter is not a type of capacity.

Say: “Composition lets Shelter reuse capacity checking by owning a Capacity component.”

## 10. Interface segregation

```cpp
class IExpirable {
public:
    virtual bool isExpired() const = 0;
    virtual std::string getExpiry() const = 0;
    virtual ~IExpirable() = default;
};
```

C++ has no separate interface keyword here. This abstract class represents a focused contract. Food implements it; Water does not need expiry methods in this simplified model. Food's use of Resource and IExpirable is multiple inheritance from a base class and an interface.

Say: “Only objects that need expiry behavior implement the expiry interface.”

## 11. Constructors, destructors and const

- `Resource(...)` and `Food(...)` initialize objects and validate initial values.
- The `: Resource(...)` initializer invokes the base constructor.
- `virtual ~Resource() = default` allows safe destruction of a derived object through a base pointer.
- `getQuantity() const` promises not to modify the object during that call.
- `explicit Shelter(int maximum)` prevents unintended implicit integer-to-Shelter conversions.

## 12. Ownership and RAII

```cpp
std::vector<std::unique_ptr<Resource> > resources;
```

The vector stores owning smart pointers, allowing Water and Food objects in one collection. `unique_ptr` releases its object automatically. `std::move` transfers ownership into the vector; the pointer cannot be copied.

Say: “Smart pointers let the shelter own different resource types without manually calling delete.”

The web adapter's Connection destructor similarly closes sockets, but that is supporting infrastructure.

## 13. Association

```cpp
struct Allocation {
    int residentId;
    int resourceId;
    int quantity;
};
```

An allocation links a household and a resource by ID. It stores the relationship, not ownership of either entity. Simple Allocation and Restock records use public fields; invariant-heavy Resource and Resident objects use private fields.

## What this simpler version does not claim

Factory, Strategy, dependency injection and repository aggregation were removed to reduce complexity. You do not need to claim coverage of every design pattern or all five SOLID principles. It retains a clear interface segregation example, separated responsibilities and ordinary substitutable resource types, but the full SOLID checklist is not its goal.

Templates and lambdas in the separate test file are test helpers, not part of the main business model. Networking helpers use some utility lambdas; do not confuse those with an OOP pillar.

## Six-minute presentation order

1. State the problem and show the starting data.
2. Open Resource: private fields, constructor, getters, abstract methods.
3. Open Water and Food: inheritance and overriding.
4. Explain polymorphism with the console display loop.
5. Demonstrate restocking twice for overloading.
6. Open Shelter: Capacity composition, admission and allocation.
7. Attempt an expired allocation to demonstrate throw/catch.
8. Finish with limitations: in-memory data and a small educational prototype.
