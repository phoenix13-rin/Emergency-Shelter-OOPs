# Emergency Shelter Resource Management System

A C++ mini-project demonstrating Object-Oriented Programming through a simple emergency shelter system. It manages food and water supplies, household admission, allocation, and restocking.

The project includes a console menu and a basic web interface. Both use the same C++ classes and business rules.

## Features

- Admit households after checking available shelter space.
- Add water and food batches.
- Track resource quantities and food expiry dates.
- Allocate supplies to registered households.
- Reject expired food, insufficient stock, and invalid input.
- Restock supplies with or without a donor name.
- View allocation and restock history.

## OOP concepts demonstrated

| Concept | Example in the code |
| --- | --- |
| Classes and objects | `Resource`, `Water`, `Food`, `Resident`, `Capacity`, and `Shelter` |
| Encapsulation | Private resource fields with getters and validated stock methods |
| Abstraction | Pure virtual `getType()` and `getUnit()` methods in `Resource` |
| Inheritance | `Water` and `Food` inherit from `Resource` |
| Method overriding | Derived implementations of `getUnit()` and Food's `isUsable()` |
| Runtime polymorphism | Calls through `Resource` pointers and references |
| Method overloading | `addStock(int)` and `addStock(int, const std::string&)` |
| Composition | `Shelter` owns a `Capacity` component |
| Association | Allocation records connect household and resource IDs |
| Interface segregation | Food implements the focused `IExpirable` interface |
| Exception handling | Custom `ShelterException` with `throw` and `catch` |
| Object lifetime | Virtual destructors and `std::unique_ptr` ownership |

Start with **`backend/resource.hpp`** and **`backend/shelter.hpp`** to understand the main OOP implementation.

## Project structure

| File | Purpose |
| --- | --- |
| `backend/resource.hpp` | Resource classes, expiry interface, and validation helpers |
| `backend/shelter.hpp` | Household, capacity, allocation, and restocking logic |
| `backend/console.cpp` | Console menu and user input |
| `backend/main.cpp` | Web application entry point |
| `backend/web.hpp` | Local HTTP server and request routing |
| `backend/json.hpp` | JSON response formatting |
| `frontend/index.html` | Web forms and tables |
| `frontend/style.css` | Frontend styling |
| `frontend/app.js` | API requests and display updates |
| `tests/test.cpp` | Tests for the C++ business rules |
| `run-console.bat` | Windows console build and launch |
| `run.bat` | Windows web server build and launch |
| `run.sh` | Linux web server build and launch |
| `OOP_GUIDE.md` | Concept explanations and presentation notes |
| `CODE_WALKTHROUGH.md` | Method-by-method code explanation |

## Requirements

- A C++11-compatible or newer compiler.
- On Windows, a recent MinGW-w64 `g++` compiler, such as MSYS2 UCRT64.
- A browser for the optional web interface.
- VS Code or another editor for viewing and building the code.

No Node.js, Python, or database server is required to run this project.

## Run on Windows

Download or clone this repository. Open the folder containing `run.bat`, `run-console.bat`, and `backend` in VS Code, then open a PowerShell terminal.

If MSYS2 is installed at `C:\msys64`, select its compiler for the current terminal:

```powershell
$env:Path = "C:\msys64\ucrt64\bin;" + $env:Path
g++ --version
```

### Console application

```powershell
.\run-console.bat
```

Choose an option from the menu:

1. Show shelter
2. Admit household
3. Add water batch
4. Add food batch
5. Allocate supplies
6. Restock supplies
0. Exit

### Web application

```powershell
.\run.bat
```

Wait for the server startup message, then open:

**http://localhost:8080**

Keep the server terminal open while using the frontend. Stop the server with **Ctrl+C**. If port 8080 is already occupied, stop the other server first.

Open the frontend through the server's URL. Opening `frontend/index.html` directly does not start the C++ backend.

## Run on Linux

Web application:

```bash
bash run.sh
```

Console application:

```bash
mkdir -p build
g++ -std=c++11 -Wall -Wextra -pedantic backend/console.cpp -o build/console
./build/console
```

Run these commands from the repository root.

## Example demonstration

Start a fresh program instance before following this sequence.

| Action | Expected result |
| --- | --- |
| Show initial data | 5 people in a 50-person shelter; 50 litres of water |
| Admit Demo family with 3 members | 8 people; new household ID 3 |
| Allocate 5 units of resource 1 to household 3 | Water decreases to 45 litres |
| Allocate 1 unit of resource 3 to household 3 | Expired-food exception; stock and history stay unchanged |
| Restock resource 1 with 10 units and no donor | 55 litres; one-argument `addStock()` call |
| Restock resource 1 with 5 units and donor College club | 60 litres; two-argument `addStock()` call |

Resource 3 is an intentionally expired example batch. Food remains usable through its expiry date. Adding stock to an expired batch does not change that batch's expiry date.

## Run the tests

On Windows:

```powershell
New-Item -ItemType Directory -Force build | Out-Null
g++ -std=c++11 -Wall -Wextra -pedantic tests/test.cpp -o build/tests.exe
.\build\tests.exe
```

On Linux:

```bash
mkdir -p build
g++ -std=c++11 -Wall -Wextra -pedantic tests/test.cpp -o build/tests
./build/tests
```

Successful tests print `All simple shelter tests passed.` Run tests with assertions enabled; do not compile them with `-DNDEBUG`.

## Data and project scope

- Data is stored in memory and resets when the program restarts.
- Refreshing the browser preserves data while its server remains running.
- Console and web applications have separate in-memory state.
- This version supports one shelter and two resource types.
- It does not include accounts, persistent storage, discharge, or automatic prioritization.
- GitHub stores the source code. The C++ backend runs locally; GitHub Pages cannot execute it.
