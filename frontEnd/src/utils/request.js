import axios from 'axios';
import { ElMessage } from 'element-plus';
import { TokenManager } from './auth'; // 导入Token工具类（统一管理Token）
import router from '../router'; // 导入路由实例（用于跳转）

// 创建 axios 实例
const request = axios.create({
  baseURL: import.meta.env.VITE_API_BASE_URL || '', // 加默认值，避免环境变量未配置时报错
  timeout: 10000,
  headers: {
    'Content-Type': 'application/json'
  }
});

// 请求拦截器：从TokenManager获取Token（统一来源，避免直接操作localStorage）
request.interceptors.request.use(
  (config) => {
    const token = TokenManager.getToken(); // 关键：用工具类获取，而非直接读localStorage
    if (token) {
      config.headers.Authorization = `Bearer ${token}`;
    }
    return config;
  },
  (error) => {
    return Promise.reject(error);
  }
);

// 响应拦截器：统一处理错误，使用路由跳转而非页面刷新
request.interceptors.response.use(
  (response) => {
    return response.data;
  },
  (error) => {
    const { response } = error;
    const status = response?.status;
    const data = response?.data;

    switch (status) {
      case 401:
        ElMessage.error('登录已过期，请重新登录');
        TokenManager.removeToken(); // 关键：用工具类清除Token，与存储逻辑一致
        router.push('/login'); // 用路由跳转，避免页面刷新（单页应用最佳实践）
        break;
      case 403:
        ElMessage.error('没有权限执行此操作');
        break;
      case 500:
        ElMessage.error('服务器内部错误');
        break;
      default:
        ElMessage.error(data?.message || '网络错误，请稍后重试');
    }

    return Promise.reject(error);
  }
);

export default request;