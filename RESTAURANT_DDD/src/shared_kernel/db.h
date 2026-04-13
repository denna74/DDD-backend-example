#pragma once

#include <sqlite3.h>
#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>

class Database {
public:
    explicit Database(const std::string& path) {
        if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
            throw std::runtime_error("Failed to open database: " + path);
        }
        exec("PRAGMA journal_mode=WAL;");
        exec("PRAGMA foreign_keys=ON;");
    }

    ~Database() { if (db_) sqlite3_close(db_); }
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void exec(const std::string& sql) {
        char* err = nullptr;
        if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
            std::string msg = err ? err : "unknown error";
            sqlite3_free(err);
            throw std::runtime_error("SQL error: " + msg);
        }
    }

    void execFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) throw std::runtime_error("Cannot open SQL file: " + path);
        std::stringstream ss;
        ss << file.rdbuf();
        exec(ss.str());
    }

    sqlite3* handle() { return db_; }

private:
    sqlite3* db_ = nullptr;
};
