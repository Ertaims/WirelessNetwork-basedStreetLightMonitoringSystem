#pragma once
#include <httplib.h>

// 应用CORS中间件
void applyCorsMiddleware(httplib::Server& svr);