# Read the code in this order

Do not start with the web server. Start with `resource.hpp`, then `shelter.hpp`, and finally `console.cpp`. Those files explain the OOP model without HTML or HTTP.

## resource.hpp

**ShelterException:** a custom error class. It forwards a message to the standard runtime_error constructor.

**checkName():** a helper that rejects empty, overlong or control-character names. It is a free function shared by several classes.

**today() and validDate():** helpers for dates. They are ordinary functions, not OOP examples. Dates use YYYY-MM-DD, so comparing validated date strings gives chronological ordering. A food batch is usable through the date printed on it.

**IExpirable:** a small interface describing two expiry operations. Only Food implements it.

**Resource private fields:** every resource has an ID, a name and stock quantity. Private access prevents uncontrolled direct changes.

**Resource constructor:** initializes those fields and rejects invalid initial stock. Getters read the data. setName validates changes.

**addStock(amount):** rejects non-positive additions and totals above 100000, then increases quantity.

**addStock(amount, donor):** validates the donor, then calls the first overload. Keeping these stock rules in one method avoids duplicated validation.

**consume(amount):** rejects a non-positive quantity, unusable resource or insufficient stock, then subtracts the quantity. Notice that it calls the virtual isUsable() method; Food's version will be used for a Food object.

**getType()/getUnit():** pure virtual methods; Resource does not provide a category or unit by itself. This makes the class abstract.

**Water:** inherits common Resource behavior. Its type is Water, its unit is litres, and it uses the base stock-only isUsable() implementation.

**Food:** inherits Resource and implements IExpirable. It owns an expiry date and overrides isUsable() to check both stock and expiry. Its unit is packs.

## shelter.hpp

**Resident:** a household contact with an ID, name and member count. A household has 1 to 20 members. Private fields cannot be altered directly.

**Capacity:** owns the maximum number of people. check(current, incoming) throws when admission would exceed that maximum.

**Allocation:** links household ID, resource ID and distributed quantity.

**Restock:** links a resource ID, added quantity and optional donor name.

**Shelter private members:** a Capacity component, resources, residents and history. nextResourceId and nextResidentId generate separate sequential IDs.

**Shelter constructor:** creates a Capacity component and starts both ID counters at 1. reset() is called by the application after construction to populate the example data.

**findResource(id):** loops over resources and returns a reference to the matching base Resource object. Throws if no matching ID exists.

**getOccupancy():** loops over all admitted households and sums their member counts.

**addWater()/addFood():** construct the chosen derived object, transfer its owning smart pointer into the resources vector, and increment the resource ID. These direct methods replace the old factory.

**admit(name, members):** constructs and validates a Resident, checks capacity, adds it, then increments the resident ID. If validation fails, no resident is inserted.

**allocate(residentId, resourceId, quantity):** verifies the household, finds the resource, validates stock/usability, records the allocation and consumes stock. The checks happen before state changes. consume() also checks its own contract; this supports safe calls outside allocation. There is no database transaction or separate allocation strategy.

**restock(resourceId, quantity, donor):** finds the resource, validates the addition, records restock details, and calls one of the two addStock overloads. Blank donor means ordinary restocking; nonblank donor means a named donation.

**reset():** clears collections, resets IDs and inserts three example batches and two example households. Capacity stays the same. In the supplied applications it is 50.

## console.cpp

**readNumber():** reads a line, converts it to a whole number and reports invalid input through an exception.

**showShelter():** loops over base resource pointers. getType(), getUnit() and isUsable() demonstrate runtime polymorphism. It also displays residents and history counts.

**main():** creates one Shelter, populates example data and repeatedly displays a menu. Each choice collects input and calls a Shelter method. A try/catch reports rejected actions. Option 0 exits.

## Browser version

main.cpp creates the SAME Shelter class and starts a server. web.hpp maps browser requests to methods such as admit(), allocate() and restock(). json.hpp formats the current state for the browser. frontend/app.js forwards form values and updates tables.

You can explain that architecture briefly, but the C++ objects and their interactions are the presentation's focus. The server runs one request at a time on localhost. The console and web program have independent in-memory state.

## Follow one operation completely

Example: give household 3 five litres from resource 1.

1. Console menu or browser form supplies IDs 3 and 1, quantity 5.
2. Shelter::allocate() confirms that household 3 exists.
3. findResource(1) returns a Resource reference referring to Water.
4. isUsable() and getQuantity() validate the distribution.
5. An Allocation record links household 3 and resource 1.
6. consume(5) subtracts five litres from private stock.
7. Display code reads stock through getQuantity().

If resource 3 is chosen instead, isUsable() executes Food's overridden expiry check. The custom exception is thrown before allocation history or stock changes. The console or web adapter catches it and displays the message.
