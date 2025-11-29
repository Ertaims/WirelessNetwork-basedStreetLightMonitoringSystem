#include "Jwt.h"
#include <openssl/hmac.h>
#include <openssl/buffer.h>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

Jwt::Jwt(const std::string& secret) : secretKey(secret) {}

/**
 * 生成JWT令牌
 * @param claims 声明
 * @param expirationSeconds 过期时间（秒）
 * @return JWT令牌
 */
std::string Jwt::generateToken(const std::map<std::string, std::string>& claims, int expirationSeconds) {
    using namespace std::chrono;
    
    // 创建JWT头部
    nlohmann::json header = {
        {"alg", "HS256"},
        {"typ", "JWT"}
    };
    
    // 创建JWT载荷
    nlohmann::json payload;
    for (const auto& claim : claims) {
        payload[claim.first] = claim.second;
    }
    
    // 添加过期时间
    if (expirationSeconds > 0) {
        auto now = system_clock::now();
        auto exp = now + seconds(expirationSeconds);
        auto expSeconds = duration_cast<seconds>(exp.time_since_epoch()).count();
        payload["exp"] = std::to_string(expSeconds);
    }
    
    // 对头部和载荷进行Base64编码
    std::string encodedHeader = base64Encode(header.dump());
    std::string encodedPayload = base64Encode(payload.dump());
    
    // 构建签名数据
    std::string signatureData = encodedHeader + "." + encodedPayload;
    
    // 生成HMAC签名
    std::string signature = generateHmac(signatureData);
    
    // 组合完整的JWT令牌
    return encodedHeader + "." + encodedPayload + "." + signature;
}

/**
 * 验证JWT令牌
 * @param token JWT令牌
 * @return 验证结果
 */
bool Jwt::validateToken(const std::string& token) {
    try {
        // 检查令牌格式
        size_t firstDot = token.find('.');
        size_t lastDot = token.rfind('.');
        
        if (firstDot == std::string::npos || lastDot == std::string::npos || firstDot == lastDot) {
            return false;
        }
        
        // 提取签名数据和签名
        std::string signatureData = token.substr(0, lastDot);
        std::string signature = token.substr(lastDot + 1);
        
        // 重新计算签名并验证
        std::string newSignature = generateHmac(signatureData);
        
        // 比较签名
        if (newSignature != signature) {
            return false;
        }
        
        // 检查令牌是否过期
        if (isTokenExpired(token)) {
            return false;
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

/**
 * 提取JWT令牌中的声明
 * @param token JWT令牌
 * @return 声明
 */
std::map<std::string, std::string> Jwt::extractClaims(const std::string& token) {
    try {
        // 提取载荷部分
        size_t firstDot = token.find('.');
        size_t lastDot = token.rfind('.');
        
        if (firstDot == std::string::npos || lastDot == std::string::npos || firstDot >= lastDot) {
            throw std::runtime_error("无效的JWT格式");
        }
        
        std::string encodedPayload = token.substr(firstDot + 1, lastDot - firstDot - 1);
        std::string payload = base64Decode(encodedPayload);
        
        // 解析JSON
        nlohmann::json json = nlohmann::json::parse(payload);
        
        // 转换为map
        std::map<std::string, std::string> claims;
        for (auto& [key, value] : json.items()) {
            claims[key] = value.dump();
        }
        
        return claims;
    } catch (const std::exception& e) {
        throw std::runtime_error("提取JWT声明失败: " + std::string(e.what()));
    }
}

/**
 * 检查JWT令牌是否已过期
 * @param token JWT令牌
 * @return 是否已过期
 */
bool Jwt::isTokenExpired(const std::string& token) {
    try {
        // 提取过期时间
        auto claims = extractClaims(token);
        
        if (claims.find("exp") == claims.end()) {
            return false; // 没有过期时间
        }
        
        // 获取当前时间
        using namespace std::chrono;
        auto now = system_clock::now();
        auto nowSeconds = duration_cast<seconds>(now.time_since_epoch()).count();
        
        // 转换过期时间
        long long expSeconds = std::stoll(claims["exp"]);
        
        // 检查是否过期
        return nowSeconds > expSeconds;
    } catch (const std::exception&) {
        return true; // 出错时认为已过期
    }
}

/**
 * Base64编码
 * @param input 要编码的字符串
 * @return 编码后的字符串
 */
std::string Jwt::base64Encode(const std::string& input) {
    // 实际实现中应使用标准的Base64编码
    // 这里简化处理，使用OpenSSL的函数
    BIO* bio, *b64;
    BUF_MEM* bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // 不添加换行
    BIO_write(bio, input.data(), input.size());
    BIO_flush(bio);
    
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    std::string result(bufferPtr->data, bufferPtr->length);

    // 清理
    BIO_free_all(bio);
    
    // 替换Base64 URL安全字符
    std::replace(result.begin(), result.end(), '+', '-');
    std::replace(result.begin(), result.end(), '/', '_');
    result.erase(std::remove(result.begin(), result.end(), '='), result.end());
    
    return result;
}

/**
 * Base64解码
 * @param input 要解码的Base64字符串
 * @return 解码后的字符串
 */
std::string Jwt::base64Decode(const std::string& input) {
    // 实际实现中应使用标准的Base64解码
    std::string paddedInput = input;
    
    // 添加缺失的填充字符
    while (paddedInput.size() % 4 != 0) {
        paddedInput += '=';
    }
    
    // 替换Base64 URL安全字符
    std::replace(paddedInput.begin(), paddedInput.end(), '-', '+');
    std::replace(paddedInput.begin(), paddedInput.end(), '_', '/');
    
    // 使用OpenSSL进行解码
    BIO* bio, *b64;
    char buffer[512];
    int length;
    std::string result;
    
    bio = BIO_new_mem_buf(paddedInput.data(), paddedInput.size());
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    
    while ((length = BIO_read(bio, buffer, sizeof(buffer))) > 0) {
        result.append(buffer, length);
    }
    
    BIO_free_all(bio);
    
    return result;
}

/**
 * 生成HMAC签名
 * @param data 待签名的数据
 * @return 签名
 */
std::string Jwt::generateHmac(const std::string& data) {
    unsigned char* digest;
    size_t digestLength = 0;    
    
     // 使用 EVP_MAC API (OpenSSL 3.0 推荐方式)
    EVP_MAC* mac = EVP_MAC_fetch(nullptr, "HMAC", nullptr);
    if (!mac) {
        // 处理错误
        return "";
    }
    
    EVP_MAC_CTX* ctx = EVP_MAC_CTX_new(mac);
    if (!ctx) {
        EVP_MAC_free(mac);
        return "";
    }
    
    // 设置参数
    const char* digestType = "SHA256";
    OSSL_PARAM params[2];
    params[0] = OSSL_PARAM_construct_utf8_string("digest", const_cast<char*>(digestType), 0);
    params[1] = OSSL_PARAM_construct_end();
    
    // 初始化
    if (EVP_MAC_init(ctx, reinterpret_cast<const unsigned char*>(secretKey.c_str()), secretKey.size(), params) != 1) 
    {
        EVP_MAC_CTX_free(ctx);
        EVP_MAC_free(mac);
        return "";
    }
    
    // 更新数据
    if (EVP_MAC_update(ctx, reinterpret_cast<const unsigned char*>(data.c_str()), data.size()) != 1) 
    {
        EVP_MAC_CTX_free(ctx);
        EVP_MAC_free(mac);
        return "";
    }
    
    // 分配输出缓冲区并完成计算
    size_t outsize;
    if (EVP_MAC_final(ctx, nullptr, &outsize, 0) != 1) {
        EVP_MAC_CTX_free(ctx);
        EVP_MAC_free(mac);
        return "";
    }
    
    digest = new unsigned char[outsize];
    if (EVP_MAC_final(ctx, digest, &digestLength, outsize) != 1) {
        delete[] digest;
        EVP_MAC_CTX_free(ctx);
        EVP_MAC_free(mac);
        return "";
    }
    
    // 清理资源
    EVP_MAC_CTX_free(ctx);
    EVP_MAC_free(mac);
    
    // 编码为 Base64
    std::string signature(reinterpret_cast<char*>(digest), digestLength);
    delete[] digest;
    
    return base64Encode(signature);
}