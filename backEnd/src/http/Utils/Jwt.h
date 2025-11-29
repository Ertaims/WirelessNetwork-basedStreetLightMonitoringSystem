#pragma once
#include <string>
#include <map>

class Jwt {
public:
    Jwt(const std::string& secret);
    
    // 生成JWT令牌
    std::string generateToken(const std::map<std::string, std::string>& claims, int expirationSeconds);
    
    // 验证令牌
    bool validateToken(const std::string& token);
    
    // 从令牌中提取声明
    std::map<std::string, std::string> extractClaims(const std::string& token);
    
    // 检查令牌是否过期
    bool isTokenExpired(const std::string& token);

private:
    std::string secretKey;
    
    // Base64编码/解码辅助函数
    std::string base64Encode(const std::string& input);
    std::string base64Decode(const std::string& input);
    
    // HMAC签名生成
    std::string generateHmac(const std::string& data);
};