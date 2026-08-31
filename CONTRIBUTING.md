# Contributing to WindNinja

Thank you for your interest in contributing to WindNinja! This document provides guidelines and instructions for contributing to this project.

## Getting Started

### Prerequisites
- C++17 compatible compiler (GCC, Clang, or MSVC)
- CMake (version 3.10 or higher)
- GDAL library
- Qt5 (for GUI components)
- OpenMP (for parallel processing)

### Building the Project
1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd WindNinja
   ```
2. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```
3. Configure with CMake:
   ```bash
   cmake ..
   ```
4. Build:
   ```bash
   cmake --build .
   ```

## Development Guidelines

### Code Style
- Follow the existing code style and formatting conventions
- Use meaningful variable and function names
- Add comments for complex logic
- Ensure all new code compiles without warnings

### Commit Messages
- Write clear, descriptive commit messages
- Reference any related issues (e.g., "Fixes #123")
- Keep commits focused on a single change

### Pull Requests
1. Fork the repository and create a feature branch
2. Make your changes and test thoroughly
3. Ensure all existing tests pass
4. Submit a pull request with a clear description of changes
5. Be responsive to code review feedback

## Reporting Issues
- Use the issue tracker to report bugs or request features
- Include steps to reproduce for bug reports
- Provide system information (OS, compiler version, etc.)

## Questions?
Feel free to open an issue for any questions about contributing.
