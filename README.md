# Emergency Shelter — Simple C++ OOP Project

This version focuses on code you can understand and explain. It uses two main OOP files, straightforward loops, and only two resource types: water and food.

## What changed from the original version?

- Removed Factory and Strategy patterns, repository interfaces, file storage, priority flags and multiple dashboard screens.
- Kept abstraction, encapsulation, inheritance, overriding, overloading, custom exceptions, composition and a focused expiry interface.
- Added a console menu using the SAME Shelter and Resource classes as the web version.
- Added a real restock action to demonstrate both overloaded addStock methods.
- Simplified the frontend to one page with ordinary forms and tables.
- Data is in memory. Restarting either program resets its example data. Browser refresh does not reset a running server.
- The console and web executable are separate program instances; their data is not shared.

## Start with these two files

1. `backend/resource.hpp`: Resource, Water, Food, IExpirable and custom exception.
2. `backend/shelter.hpp`: Resident, Capacity, Shelter, allocation and restock records.

Read `OOP_GUIDE.md` for a concept-by-concept explanation and `CODE_WALKTHROUGH.md` for the method-by-method explanation.

## Windows: run the frontend with the C++ backend

1. Extract this ZIP into a NEW folder. Keep your old working project separate.
2. In VS Code choose File → Open Folder and select `shelter-simple` itself. It contains `run.bat`, `backend` and `frontend`.
3. Open Terminal → New Terminal.
4. If MSYS2 was installed at `C:\msys64`, select its compiler for the current PowerShell terminal:
   ```powershell
   $env:Path = "C:\msys64\ucrt64\bin;" + $env:Path
   ```
5. Check the compiler:
   ```powershell
   g++ --version
   ```
6. Start the project:
   ```powershell
   .\run.bat
   ```
7. Wait for `Simple Shelter running at http://localhost:8080`.
8. Open `http://localhost:8080` and keep the terminal open.

Stop the old server before running this version if it already uses port 8080. Click the old terminal and press Ctrl+C; answer Y if asked to terminate the batch job.

Opening `frontend/index.html` directly does not run C++. This version's frontend is only an interface for its running backend; it has no separate browser-only simulation.

## Run the console instead — easiest for explaining C++

In the same project folder, with the compiler selected:

```powershell
.\run-console.bat
```

Menu:

```text
1. Show shelter
2. Admit household
3. Add water batch
4. Add food batch
5. Allocate supplies
6. Restock supplies
0. Exit
```

The console requires no browser and no socket library. It is an optional way to demonstrate the same business logic, not an extra service that the frontend needs.

## Linux

Web version:

```bash
bash run.sh
```

Console version:

```bash
mkdir -p build
g++ -std=c++11 -Wall -Wextra -pedantic backend/console.cpp -o build/console
./build/console
```

The code uses C++11 features and avoids the filesystem dependency that caused your earlier compiler error. Continue using your newly installed MSYS2 compiler; the old Windows GCC installation has not been tested.

## Starting example

- Shelter capacity: 50 people.
- Asha family: 3 people; Ravi family: 2 people.
- Occupancy: 5; available spaces: 45.
- Resource 1: Drinking water, 50 litres.
- Resource 2: Meal packs, 30 packs, expiry 30 days ahead.
- Resource 3: Expired food batch, 5 packs, expiry yesterday.

## Simple presentation demo

1. Show the starting data.
2. Admit Demo family, 3 members. Occupancy becomes 8.
3. Allocate 5 litres from resource 1 to household 3. Water becomes 45 litres.
4. Try resource 3, quantity 1. The exception rejects expired food.
5. Try 999 litres of water. The exception rejects insufficient stock.
6. Restock resource 1 with 10 units and no donor: `addStock(amount)`.
7. Restock resource 1 with 5 units and donor College club: `addStock(amount, donor)`.
8. Show Resource, Water, Food, Capacity and Shelter in VS Code.

The code prints or displays which overload was used. Food is usable through its expiry day. Restocking an expired batch does not change its expiry; it remains blocked. Create a new food batch for a new expiry date.

## Files

| File | Purpose |
| --- | --- |
| `backend/resource.hpp` | Main resource OOP classes, expiry and input helpers |
| `backend/shelter.hpp` | Main shelter OOP classes and actions |
| `backend/console.cpp` | Straightforward menu calling Shelter methods |
| `backend/main.cpp` | Creates the shelter and starts the web server |
| `backend/web.hpp` | Networking adapter; converts browser inputs into method calls |
| `backend/json.hpp` | Converts backend state into JSON for the browser |
| `frontend/index.html` | Forms and tables |
| `frontend/style.css` | Basic styling |
| `frontend/app.js` | Sends inputs to C++ and renders results |
| `tests/test.cpp` | Domain checks, separate from presentation code |
| `run.bat` | Compile and run web version on Windows |
| `run-console.bat` | Compile and run console version on Windows |
| `run.sh` | Compile and run web version on Linux |

## Testing

After creating `build` (the run scripts do this automatically):

Windows:

```powershell
g++ -std=c++11 -Wall -Wextra -pedantic tests/test.cpp -o build/tests.exe
.\build\tests.exe
```

Linux:

```bash
mkdir -p build
g++ -std=c++11 -Wall -Wextra -pedantic tests/test.cpp -o build/tests
./build/tests
```

Run tests with assertions enabled. The C++ builds, domain tests, console demo and HTTP integration were tested on Linux. Windows execution and rendered browser layout need checking on your machine.

This is an educational single-user local prototype. It uses no database, accounts, automatic prioritization or persistent storage. Date utilities, JSON and networking are supporting code; your OOP explanation should focus on the two main class headers.
