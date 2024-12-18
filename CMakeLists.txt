cmake_minimum_required(VERSION 3.5)

# Основной проект
project(DistributedFileSystem LANGUAGES CXX)

# Установка глобальных настроек
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем каждый из проектов
add_subdirectory(DFS)
add_subdirectory(FollowerServer)
add_subdirectory(MainServer)
add_subdirectory(ReplicaServer)
