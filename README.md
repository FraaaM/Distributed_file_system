# Distributed File System (DFS)

## Description:
This project implements a simple Distributed File System (DFS) with a server-client architecture. The server handles file storage, retrieval, deletion, and user authentication, while the client provides an interface for interacting with the server. The system is designed for basic distributed file management.

## Features

### Server functionality:
- User authentication (login and registration).
- Upload, download, and delete files.
- Retrieve a list of available files.

### Client functionality:
- Easy-to-use commands for file operations.
- Interactive communication with the server.

## Requirements

### General Requirements:
- **Programming Language:** C++ with Qt framework.
- **Database:** SQLite.

### Specific Libraries:
- Qt Core and Network modules (e.g., `QTcpServer`, `QTcpSocket`).
- Qt SQL module for database integration.

## Build and use:
- Go to the desired subproject and run `build.bat`.
- Run the exe file from the build_ninja folder.
