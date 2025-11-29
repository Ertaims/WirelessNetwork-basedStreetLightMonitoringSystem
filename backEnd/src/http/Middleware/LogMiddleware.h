#pragma once
#include <httplib.h>

// 应用日志中间件
void applyLogMiddleware(httplib::Server& svr);