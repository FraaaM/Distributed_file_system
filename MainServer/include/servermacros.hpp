#pragma once

#define CHUNK_SIZE 1024

#define DATABASE_TYPE "QSQLITE"
#define DATABASE_NAME "db.db"

#define UPLOAD_DIRECTORY "/Uploaded_files"

#define CLIENT "CLIENT"
#define MAIN_SERVER "MAIN_SERVER"
#define REPLICA "REPLICA"

#define RESPONSE_LOGIN_USER_SUCCESS "LOGIN_USER_SUCCESS"
#define RESPONSE_LOGIN_ADMIN_SUCCESS "LOGIN_ADMIN_SUCCESS"
#define RESPONSE_LOGIN_FAILED "LOGIN_FAILED"
#define RESPONSE_REGISTER_SUCCESS "REGISTER_SUCCESS"
#define RESPONSE_REGISTER_USER_EXISTS "REGISTER_USER_EXISTS"
#define RESPONSE_REGISTER_FAILED "REGISTER_FAILED"
#define RESPONSE_DOWNLOAD_READY "DOWNLOAD_READY"
#define RESPONSE_DOWNLOAD_FAILED "DOWNLOAD_FAILED"
#define RESPONSE_DELETE_SUCCESS "DELETE_SUCCESS"
#define RESPONSE_DELETE_FAILED "DELETE_FAILED"
#define RESPONSE_UPLOAD_SUCCESS "UPLOAD_SUCCESS"
#define RESPONSE_UPDATE_USER_FAILED "UPDATE_USER_FAILED"
#define RESPONSE_UPDATE_USER_SUCCESS "UPDATE_USER_SUCCESS"
#define RESPONSE_UPLOAD_FAILED "UPLOAD_FAILED"
#define RESPONSE_READY_FOR_DATA "READY_FOR_DATA"
#define RESPONSE_CHUNK_RECEIVED "CHUNK_RECEIVED"
#define RESPONSE_FILES_LIST "FILES_LIST"
#define RESPONSE_USERS_LIST "USERS_LIST"
#define RESPONSE_FILE_INFO "FILE_INFO"
#define RESPONSE_USER_INFO "USER_INFO"
#define RESPONSE_USER_DOES_NOT_EXIST "USER_DOES_NOT_EXIST"

#define COMMAND_LOGIN "LOGIN"
#define COMMAND_REGISTER "REGISTER"
#define COMMAND_DOWNLOAD "DOWNLOAD"
#define COMMAND_UPLOAD "UPLOAD"
#define COMMAND_UPLOAD_CHUNK "UPLOAD_CHUNK"
#define COMMAND_DELETE "DELETE"
#define COMMAND_DELETE_USER "DELETE_USER"
#define COMMAND_UPDATE_USER "UPDATE_USER"
#define COMMAND_GET_FILES "GET_FILES"
#define COMMAND_GET_USERS "GET_USERS"
#define COMMAND_REPLICA_UPLOAD "REPLICA_UPLOAD"
#define COMMAND_REPLICA_DELETE "REPLICA_DELETE"
#define COMMAND_REPLICA_DOWNLOAD "REPLICA_DOWNLOAD"
#define COMMAND_GET_FILE_INFO "GET_FILE_INFO"
#define COMMAND_GET_USER_INFO "GET_USER_INFO"

#define TABLE_USERS "users"
#define FIELD_USER_ID "id"
#define FIELD_USER_USERNAME "username"
#define FIELD_USER_PASSWORD "password"
#define FIELD_USER_IS_ADMIN "is_admin"
#define FIELD_USER_GROUP_ID "group_id"
#define FIELD_USER_RIGHTS "rights"

#define TABLE_FILES "files"
#define FIELD_FILE_ID "id"
#define FIELD_FILE_FILENAME "file_name"
#define FIELD_FILE_OWNER "owner"
#define FIELD_FILE_SIZE "size"
#define FIELD_FILE_UPLOAD_DATE "upload_date"
#define FIELD_FILE_GROUP_ID "group_id"

#define TABLE_FILE_REPLICAS "file_replicas"
#define FIELD_REPLICA_ID "id"
#define FIELD_REPLICA_ADDRESS "replica_address"
#define FIELD_REPLICA_PORT "replica_port"
