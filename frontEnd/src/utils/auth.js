import { jwtDecode } from 'jwt-decode'

// Token 管理
export const TokenManager = {
  getToken() {
    return localStorage.getItem('auth_token')
  },
  
  setToken(token) {
    localStorage.setItem('auth_token', token)
  },
  
  removeToken() {
    localStorage.removeItem('auth_token')
  },
  
  isTokenValid() {
    const token = this.getToken()
    if (!token) return false
    
    try {
      const decoded = jwtDecode(token)
      const currentTime = Date.now() / 1000
      return decoded.exp > currentTime
    } catch (error) {
      console.error('Token 解析失败:', error)
      return false
    }
  },
  
  getTokenExpiration() {
    const token = this.getToken()
    if (!token) return null
    
    try {
      const decoded = jwtDecode(token)
      return decoded.exp * 1000 // 转换为毫秒
    } catch (error) {
      return null
    }
  },
  
  getTokenData() {
    const token = this.getToken()
    if (!token) return null
    
    try {
      return jwtDecode(token)
    } catch (error) {
      return null
    }
  }
}

// 权限检查
export const Permission = {
  hasPermission(user, permission) {
    if (!user || !user.role) return false
    
    const rolePermissions = {
      admin: ['read', 'write', 'delete', 'manage_users'],
      operator: ['read', 'write'],
      viewer: ['read']
    }
    
    return rolePermissions[user.role]?.includes(permission) || false
  },
  
  canAccessDevice(user, device) {
    if (!user) return false
    
    // 管理员可以访问所有设备
    if (user.role === 'admin') return true
    
    // 操作员和查看员只能访问自己区域的设备
    if (user.allowedAreas && device.area) {
      return user.allowedAreas.includes(device.area)
    }
    
    return false
  },
  
  canControlDevice(user, device) {
    if (!user) return false
    
    // 只有管理员和操作员可以控制设备
    if (!['admin', 'operator'].includes(user.role)) return false
    
    return this.canAccessDevice(user, device)
  }
}

// 用户信息管理
export const UserManager = {
  getUserInfo() {
    const userStr = localStorage.getItem('user_info')
    if (!userStr) return null
    
    try {
      return JSON.parse(userStr)
    } catch (error) {
      console.error('用户信息解析失败:', error)
      return null
    }
  },
  
  setUserInfo(user) {
    localStorage.setItem('user_info', JSON.stringify(user))
  },
  
  removeUserInfo() {
    localStorage.removeItem('user_info')
  },
  
  updateUserInfo(updates) {
    const user = this.getUserInfo()
    if (user) {
      const updatedUser = { ...user, ...updates }
      this.setUserInfo(updatedUser)
      return updatedUser
    }
    return null
  }
}

// 自动刷新 Token
export const setupTokenRefresh = (store) => {
  const refreshToken = async () => {
    if (!TokenManager.isTokenValid()) {
      // Token 已过期，需要重新登录
      store.dispatch('auth/logout')
      return
    }
    
    const tokenExpiration = TokenManager.getTokenExpiration()
    const currentTime = Date.now()
    const refreshThreshold = 5 * 60 * 1000 // 5分钟前刷新
    
    if (tokenExpiration && (tokenExpiration - currentTime) < refreshThreshold) {
      try {
        await store.dispatch('auth/refreshToken')
        console.log('Token 已自动刷新')
      } catch (error) {
        console.error('Token 刷新失败:', error)
        store.dispatch('auth/logout')
      }
    }
  }
  
  // 每分钟检查一次 Token
  setInterval(refreshToken, 60 * 1000)
}