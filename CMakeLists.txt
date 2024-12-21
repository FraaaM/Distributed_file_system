cmake_minimum_required(VERSION 3.5)

project(DistributedFileSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(DFS)
add_subdirectory(FollowerServer)
add_subdirectory(FollowerServer_2)
add_subdirectory(MainServer)
add_subdirectory(ReplicaServer)
