#include "PasswordHasher.h"
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <cstring>
#include <sstream>
#include <iomanip>

std::string PasswordHasher::hashPassword(const std::string &password)
{
    // 生成盐值
    std::string salt = generateSalt();

    // 使用SHA-256进行哈希
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();                               // 创建一个消息摘要上下文
    EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr);                    // 初始化消息摘要上下文
    EVP_DigestUpdate(mdctx, salt.c_str(), salt.length());               // 更新消息摘要
    EVP_DigestUpdate(mdctx, password.c_str(), password.length());       // 添加密码
    EVP_DigestFinal_ex(mdctx, hash, &hashLen);                          // 完成消息摘要计算
    EVP_MD_CTX_free(mdctx);                                             // 释放消息摘要上下文

    // 将盐值和哈希值转换为十六进制字符串
    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    // 添加盐值
    for (char c : salt) {
        ss << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(c));
    }

    // 添加哈希值
    for (unsigned int i = 0; i < hashLen; i++) {
        ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
    }

    return ss.str();
}

bool PasswordHasher::verifyPassword(const std::string &password, const std::string &hash)
{
    try {
        // 检查哈希是否符合预期格式（至少32个字符，用于存储盐值）
        if (hash.length() < 32) {
            // 哈希格式不符合预期，可能是明文密码
            // 直接比较密码和哈希（仅用于向后兼容）
            return password == hash;
        }

        // 从哈希中提取盐值（前32个字符，16字节）
        std::string salt;
        for (size_t i = 0; i < 32; i += 2) {
            std::string byteHex = hash.substr(i, 2);
            char byte = static_cast<char>(std::stoi(byteHex, nullptr, 16));
            salt.push_back(byte);
        }
        
        // 使用相同的盐值对输入密码进行哈希
        std::string newHash = hashPasswordWithSalt(password, salt);
        
        // 比较哈希值
        return newHash == hash;
    } catch (const std::exception &e) {
        // 解析哈希时发生错误，可能是明文密码
        // 直接比较密码和哈希（仅用于向后兼容）
        return password == hash;
    }
}

std::string PasswordHasher::generateSalt(size_t length) {
    std::string salt(length, '\0');
    RAND_bytes(reinterpret_cast<unsigned char*>(&salt[0]), length);
    return salt;
}

std::string PasswordHasher::hashPasswordWithSalt(const std::string& password, const std::string& salt) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;
    
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(mdctx, salt.c_str(), salt.length());
    EVP_DigestUpdate(mdctx, password.c_str(), password.length());
    EVP_DigestFinal_ex(mdctx, hash, &hashLen);
    EVP_MD_CTX_free(mdctx);
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    // 添加盐值
    for (char c : salt) {
        ss << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(c));
    }
    
    // 添加哈希值
    for (unsigned int i = 0; i < hashLen; i++) {
        ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
    }
    
    return ss.str();
}
