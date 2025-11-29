import { authAPI } from '../../api';
import { TokenManager } from '../../utils/auth';

const state = {
  user: null,
  token: TokenManager.getToken() || null,
  isAuthenticated: TokenManager.isTokenValid()
};

const mutations = {
  SET_USER(state, user) {
    state.user = user;
  },
  SET_TOKEN(state, token) {
    state.token = token;
    state.isAuthenticated = TokenManager.isTokenValid();
    if (token) {
      TokenManager.setToken(token);
    } else {
      TokenManager.removeToken();
    }
  },
  CLEAR_AUTH(state) {
    state.user = null;
    state.token = null;
    state.isAuthenticated = false;
    TokenManager.removeToken();
  }
};

const actions = {
  async login({ commit }, credentials) {
    try {
      // 关键修复1：response 已是后端 data，直接提取 token 和 user（无需 .data）
      const response = await authAPI.login(credentials); 
      // 假设后端登录返回格式：{ token: 'xxx', user: {...} }（若格式不同可调整）
      const token = response.token;
      const user = response.user;
      
      // 兜底：若后端登录返回的是完整用户信息（含 token 字段），则适配
      const finalUser = user || response;
      const finalToken = token || response.token;
      
      if (!finalToken) throw new Error('登录未返回有效 Token');
      
      commit('SET_USER', finalUser);
      commit('SET_TOKEN', finalToken);
  
      return { success: true, user: finalUser };
    } catch (error) {
      return { 
        success: false, 
        error: error.response?.data?.message || '登录失败，请检查用户名或密码' 
      };
    }
  },
  
  async logout({ commit }) {
    try {
      await authAPI.logout();
    } catch (error) {
      console.error('退出登录接口请求失败:', error);
    } finally {
      commit('CLEAR_AUTH');
    }
  },
  
  async getProfile({ commit }) {
    try {
      // 关键修复2：去掉多余的 .data，直接使用 response（已是后端用户信息）
      const userInfo = await authAPI.getProfile(); 
      console.log('获取到的用户信息:', userInfo); // 验证数据是否正确
      
      // 关键修复3：统一角色大小写（适配前端判断逻辑）
      const formattedUser = {
        ...userInfo,
        role: userInfo.role?.toLowerCase() || 'user' // 转为小写，避免判断异常
      };
      
      commit('SET_USER', formattedUser);
      return formattedUser;
    } catch (error) {
      commit('CLEAR_AUTH');
      throw error;
    }
  },
  
  async refreshToken({ commit }) {
    try {
      if (!TokenManager.isTokenValid()) {
        throw new Error('Token已失效，无法刷新');
      }
      // 关键修复4：去掉多余的 .data，直接提取 token
      const response = await authAPI.refreshToken(); 
      const token = response.token;
      
      if (!token) throw new Error('刷新Token失败');
      
      commit('SET_TOKEN', token);
      return token;
    } catch (error) {
      commit('CLEAR_AUTH');
      throw error;
    }
  }
};

const getters = {
  user: state => state.user,
  isAuthenticated: state => state.isAuthenticated,
  // 关键修复5：用户信息兜底，避免显示“未知”
  userName: state => state.user?.name || '未知用户',
  userRole: state => state.user?.role || 'user',
  userEmail: state => state.user?.email || '未知邮箱'
};

export default {
  namespaced: true,
  state,
  mutations,
  actions,
  getters
};