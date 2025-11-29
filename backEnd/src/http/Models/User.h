#pragma once

#include <string>
#include <vector>

class User {
public:
    User();
    User(const std::string& username, const std::string& passwordHash, const std::string& role);
    
    // Getters and Setters
    int getId() const; 
    void setId(int id);
    
    std::string getUsername() const; 
    void setUsername(const std::string& username);
    
    std::string getPasswordHash() const; 
    void setPasswordHash(const std::string& passwordHash);
    
    std::string getRole() const; 
    void setRole(const std::string& role);
    
    std::string getName() const; 
    void setName(const std::string& name);
    
    std::string getEmail() const; 
    void setEmail(const std::string& email);
    
    std::string getPhone() const; 
    void setPhone(const std::string& phone);
    
    std::string getCreatedAt() const; 
    void setCreatedAt(const std::string& createdAt);

    // 用户认证相关
    bool checkPassword(const std::string& password) const;

private:
    int id;
    std::string username;
    std::string passwordHash;
    std::string role; // admin, user
    std::string name;
    std::string email;
    std::string phone;
    std::string created_at;
};