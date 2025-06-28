# GitHub Issue: Refactor hardware interface modules

**Title:** Refactor: Move hardware interface modules to components/ directory

**Labels:** refactoring, architecture, enhancement

**Description:**

## 🎯 Objective
Refactor the project structure to follow ESP-IDF best practices by moving low-level hardware interface modules from `main/` to a dedicated `components/` directory.

## 📋 Current Structure
```
main/
├── led_controller.h/c      # Hardware interface - should be moved
├── main.c                  # Application logic - stays
└── test/                   # Application tests - stays
```

## 🎯 Target Structure
```
components/
├── led_controller/
│   ├── include/led_controller.h
│   ├── led_controller.c
│   └── CMakeLists.txt
└── distance_sensor/         # Future component
    ├── include/distance_sensor.h
    ├── distance_sensor.c
    └── CMakeLists.txt
main/
├── main.c                   # Application logic only
├── display_logic.h/c        # Business logic (future)
├── web_server.h/c          # Application layer (future)
└── test/                   # Application tests
```

## ✅ Tasks
- [ ] Create `components/led_controller/` directory
- [ ] Move `led_controller.c` to `components/led_controller/`
- [ ] Move `led_controller.h` to `components/led_controller/include/`
- [ ] Create `components/led_controller/CMakeLists.txt`
- [ ] Update `main/CMakeLists.txt` to remove led_controller references
- [ ] Update `#include` statements in all files to use new path
- [ ] Update test files to use new include path
- [ ] Verify build and functionality
- [ ] Update README.md project structure section

## 🧪 Testing
- [ ] Ensure all existing tests still pass
- [ ] Verify LED controller functionality unchanged
- [ ] Confirm main application builds and runs correctly

## 📚 Benefits
- ✅ Better separation of concerns
- ✅ Reusable hardware components
- ✅ Follows ESP-IDF best practices
- ✅ Easier unit testing of components
- ✅ Cleaner project architecture

## 🔗 Related
This prepares the architecture for upcoming distance sensor integration while maintaining clean separation between hardware abstraction and application logic.

## 🤖 Assignment
@github-copilot[bot] - This refactoring task is well-suited for automated completion while we continue with distance sensor development.
