#include "PasswordHasher.h"
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <cstring>
#include <sstream>
#include <iomanip>

/**
 * @brief 生成随机盐值的十六进制字符串
 * @param length 盐值的字节长度
 * @return 盐值的十六进制字符串
 */
std::string PasswordHasher::generateSalt(size_t length) {
    unsigned char salt[length];
    RAND_bytes(salt, length);
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(salt[i]);
    }
    
    return ss.str();
}

/**
 * @brief 密码哈希函数
 * @param password 密码
 * @return 密码哈希值（包含盐值和哈希值的十六进制字符串）
 */
std::string PasswordHasher::hashPassword(const std::string &password) {
    // 生成盐值（16字节，转换为32字符的十六进制字符串）
    std::string saltHex = generateSalt(16);
    
    // 准备要哈希的数据：盐值的十六进制字符串 + 密码
    std::string data = saltHex + password;
    
    // 使用SHA-256进行哈希
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;
    
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(mdctx, data.c_str(), data.length());
    EVP_DigestFinal_ex(mdctx, hash, &hashLen);
    EVP_MD_CTX_free(mdctx);
    
    // 将哈希值转换为十六进制字符串
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < hashLen; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
    }
    
    // 返回盐值和哈希值的组合（盐值在前，哈希值在后）
    return saltHex + ss.str();
}

/**
 * @brief 验证密码
 * @param password 密码
 * @param hash 密码哈希值
 * @return 密码是否验证成功
 */
bool PasswordHasher::verifyPassword(const std::string &password, const std::string &hash) {
    try {
        // 检查哈希是否符合预期格式（至少32个字符，用于存储盐值）
        if (hash.length() < 32) {
            // 哈希格式不符合预期，可能是明文密码
            return password == hash;
        }
        
        // 从哈希中提取盐值（前32个字符）
        std::string saltHex = hash.substr(0, 32);
        
        // 使用相同的盐值对输入密码进行哈希
        std::string newHash = hashPasswordWithSalt(password, saltHex);
        
        // 比较哈希值
        return newHash == hash;
    } catch (const std::exception &e) {
        // 解析哈希时发生错误，可能是明文密码
        return password == hash;
    }
}

/**
 * @brief 使用给定的盐值对密码进行哈希
 * @param password 密码
 * @param saltHex 十六进制格式的盐值
 * @return 密码哈希值
 */
std::string PasswordHasher::hashPasswordWithSalt(const std::string& password, const std::string& saltHex) {
    // 准备要哈希的数据：盐值的十六进制字符串 + 密码
    std::string data = saltHex + password;
    
    // 使用SHA-256进行哈希
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;
    
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(mdctx, data.c_str(), data.length());
    EVP_DigestFinal_ex(mdctx, hash, &hashLen);
    EVP_MD_CTX_free(mdctx);
    
    // 将哈希值转换为十六进制字符串
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < hashLen; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
    }
    
    // 返回盐值和哈希值的组合（盐值在前，哈希值在后）
    return saltHex + ss.str();
}