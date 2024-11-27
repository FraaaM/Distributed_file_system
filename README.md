# Distributed File System (DFS)

## Description:
This project implements a simple Distributed File System (DFS) with a server-client-replica architecture. The system is designed for basic distributed file management, ensuring availability by replicating files across multiple servers.

## Features

### Server functionality:
- **User authentication**: Login and registration for clients.
- **File operations**:
  - Upload files to the system.
  - Download files, with automatic fallback to replicas in case of unavailability on the main server.
  - Delete files, ensuring removal from all replicas.
- **File listing**: Retrieve a list of available files.
- **Replica management**:
  - Distribute files to replicas for redundancy.
  - Handle requests to fetch files from replicas when needed.

### Replica functionality:
- Synchronize files with the main server.
- Serve as backup storage for file retrieval when the other ones is unavailable.
- Handle file deletion requests from the main server.

### Client functionality:
- Easy-to-use commands for interacting with the DFS:
  - File upload, download, and deletion.
  - Fetch a list of files.
- Communication with the main server and replicas.

## Requirements

### General Requirements:
- **Programming Language:** C++ with Qt framework.
- **Database:** SQLite.

### Specific Libraries:
- Qt Core and Network modules (e.g., `QTcpServer`, `QTcpSocket`).
- Qt SQL module for database integration.

## Build and Use:

### Building:
1. Navigate to the desired subproject directory (e.g., `MainServer`, `ReplicaServer`, or `Client`).
2. Run `build.bat` to compile the project.

### Running:
1. Locate the `build_ninja` folder in each subproject.
2. Run the respective executable files (`MainServer.exe`, `ReplicaServer.exe`, `Client.exe`).

### Usage:
- Start the main server and replicas first to ensure proper system initialization.
- Launch the client application to interact with the DFS.
