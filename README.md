# Distributed File System (DFS)

## Description:
This project implements a simple Distributed File System (DFS) with a server-client-replica architecture. The system is designed for basic distributed file management, ensuring availability by replicating files across multiple servers.

## Features

### Main Server functionality:
- **User authentication**: Login and registration for clients.
- **File operations**:
  - Upload files to the system.
  - Download files, with automatic fallback to replicas in case of unavailability on the main server.
  - Delete files, ensuring removal from all replicas.
- **File listing**: Retrieve a list of available files.
- **Replica management**:
  - Distribute files to replicas for redundancy.
  - Handle requests to fetch files from replicas when needed.

### FollowerServer Functionality:
- Connects to the `MainServer` and sends heartbeat notifications to confirm availability.
- Regularly synchronizes its data with the `MainServer`.
- If the `MainServer` becomes unresponsive:
  - Attempts to connect to known replicas.
  - Assumes the role of a server, providing availability and updating other replicas as needed.

### Replica functionality:
- Synchronize files with the main server.
- Serve as backup storage for file retrieval when the other ones is unavailable.
- Handle file deletion requests from the main server.

### Client functionality:
- Easy-to-use commands for interacting with the DFS:
  - File upload, download, and deletion.
  - Fetch a list of files.

### Administration and Access Control:
- For each user, an administrator can assign read (r), write (w), and delete (d) permissions. These permissions determine which actions can be performed on visible files.
- Administrators can create or modify user groups and assign users to these groups.
- Administrators can promote users to administrators, granting them access to management functions.
- In case of need, administrators can remove a user's account from the system.

## Requirements

### General Requirements:
- **Programming Language:** C++ with Qt framework.
- **Database:** SQLite.

### Specific Libraries:
- Qt Core and Network modules (e.g., `QTcpServer`, `QTcpSocket`).
- Qt SQL module for database integration.

## Build and Use:

### Building:
1. **Individual Builds**:  
   Navigate to the desired subproject directory (e.g., `MainServer`, `FollowerServer`, `ReplicaServer`, or `DFS`) and run `build.bat` to compile the project for that specific part.

2. **Build All at Once**:  
   Alternatively, you can build the entire system at once by using the `build.bat` script located in the root of the repository. Running this script will:
   - Compile each part of the system (MainServer, FollowerServer, ReplicaServer, and DFS) individually.
   - All binaries will be placed in the `build_ninja` directory, with separate folders for each subproject, containing the corresponding executable files (`MainServer.exe`, `FollowerServer.exe`, `ReplicaServer.exe`, and `DFS.exe`).

### Running:
1. After building, locate the `build_ninja` folder in the root directory.
2. Inside the `build_ninja` folder, you will find individual folders for each subproject.
3. Run the respective executable files (`MainServer.exe`, `FollowerServer.exe`, `ReplicaServer.exe`, or `DFS.exe`) from their corresponding subfolder.

### Usage:
- Start the main server and replicas first to ensure proper system initialization.
- Launch the client application to interact with the DFS.
- After starting the servers, log in to the `DFS` client application and create a new user.
- Then log in with the credentials `admin` (username) and `!For@each` (password).
- Assign the newly created user as an administrator to give them access to management functions.
- For security purposes, delete the old administrator account after the new admin account is set up.
- Once the new administrator is in place, you can grant permissions and provide access to other participants.
