<template>
  <div id="app">
    <nav class="navbar navbar-expand-lg navbar-dark bg-primary" v-if="isAuthenticated">
      <div class="container-fluid">
        <router-link to="/" class="navbar-brand d-flex align-items-center">
          <i class="fas fa-lightbulb me-2"></i>
          智能路灯管理系统
        </router-link>
        
        <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbarNav">
          <span class="navbar-toggler-icon"></span>
        </button>
        
        <div class="collapse navbar-collapse" id="navbarNav">
          <ul class="navbar-nav me-auto">
            <li class="nav-item">
              <router-link to="/" class="nav-link" :class="{ active: $route.name === 'Dashboard' }">
                <i class="fas fa-tachometer-alt me-1"></i>
                仪表盘
              </router-link>
            </li>
            <li class="nav-item">
              <router-link to="/devices" class="nav-link" :class="{ active: $route.name === 'Devices' }">
                <i class="fas fa-list me-1"></i>
                设备总览
              </router-link>
            </li>
            <li class="nav-item">
              <router-link to="/control" class="nav-link" :class="{ active: $route.name === 'Control' }">
                <i class="fas fa-cogs me-1"></i>
                设备控制
              </router-link>
            </li>
            <li class="nav-item">
              <router-link to="/statistics" class="nav-link" :class="{ active: $route.name === 'Statistics' }">
                <i class="fas fa-chart-bar me-1"></i>
                数据统计
              </router-link>
            </li>
            <li class="nav-item">
              <router-link to="/alarms" class="nav-link position-relative" :class="{ active: $route.name === 'Alarms' }">
                <i class="fas fa-bell me-1"></i>
                告警管理
                <span v-if="pendingAlarmsCount > 0" class="position-absolute top-0 start-100 translate-middle badge rounded-pill bg-danger">
                  {{ pendingAlarmsCount }}
                </span>
              </router-link>
            </li>
          </ul>
          
          <ul class="navbar-nav">
            <li class="nav-item dropdown">
              <a class="nav-link dropdown-toggle" href="#" id="navbarDropdown" role="button" data-bs-toggle="dropdown">
                <i class="fas fa-user me-1"></i>
                {{ user?.name || '用户' }}
              </a>
              <ul class="dropdown-menu dropdown-menu-end">
                <li>
                  <span class="dropdown-item-text">
                    <small class="text-muted">角色: {{ user?.role || '未知' }}</small>
                  </span>
                </li>
                <li><hr class="dropdown-divider"></li>
                <li>
                  <router-link to="/profile" class="dropdown-item">
                    <i class="fas fa-user-cog me-2"></i>个人设置
                  </router-link>
                </li>
                <li>
                  <a class="dropdown-item" href="#" @click="handleLogout">
                    <i class="fas fa-sign-out-alt me-2"></i>退出登录
                  </a>
                </li>
              </ul>
            </li>
          </ul>
        </div>
      </div>
    </nav>
    
    <main :class="{ 'container-fluid': isAuthenticated, 'container': !isAuthenticated }">
      <router-view />
    </main>
    
    <!-- 全局通知 -->
    <div class="position-fixed bottom-0 end-0 p-3" style="z-index: 11">
      <div v-for="(notification, index) in notifications" :key="index" 
           class="toast show mb-2" role="alert">
        <div class="toast-header" :class="`bg-${notification.type}`">
          <strong class="me-auto text-white">{{ notification.title }}</strong>
          <button type="button" class="btn-close btn-close-white" @click="removeNotification(index)"></button>
        </div>
        <div class="toast-body">
          {{ notification.message }}
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { mapGetters, mapActions } from 'vuex'
import { setupTokenRefresh } from './utils/auth'

export default {
  name: 'App',
  data() {
    return {
      notifications: [],
      autoRefreshInterval: null
    }
  },
  computed: {
    ...mapGetters('auth', ['isAuthenticated', 'user']),
    ...mapGetters('alarms', ['pendingAlarmsCount'])
  },
  async mounted() {
    if (this.isAuthenticated) {
      // 设置 Token 自动刷新
      setupTokenRefresh(this.$store)
      
      // 获取用户信息（优先获取，避免后续接口权限问题）
      try {
        await this.$store.dispatch('auth/getProfile')
        this.showNotification('成功', '用户信息加载完成', 'success')
      } catch (error) {
        console.error('获取用户信息失败:', error)
        this.showNotification('错误', '获取用户信息失败', 'danger')
      }
      
      // 加载初始数据（串行执行，避免并行请求冲突）
      await this.loadInitialData()
      
      // 数据加载完成后，再设置自动刷新
      this.setupAutoRefresh()
    }
  },
  beforeUnmount() {
    if (this.autoRefreshInterval) {
      clearInterval(this.autoRefreshInterval)
    }
  },
  watch: {
    isAuthenticated(newVal) {
      if (newVal) {
        // 登录后重新执行初始化流程
        this.initAfterLogin()
      } else {
        if (this.autoRefreshInterval) {
          clearInterval(this.autoRefreshInterval)
        }
      }
    }
  },
  methods: {
    ...mapActions('auth', ['logout']),
    
    async initAfterLogin() {
      setupTokenRefresh(this.$store)
      try {
        await this.$store.dispatch('auth/getProfile')
      } catch (error) {
        console.error('获取用户信息失败:', error)
      }
      await this.loadInitialData()
      this.setupAutoRefresh()
    },
    
    async handleLogout() {
      try {
        await this.logout()
        this.$router.push('/login')
        this.showNotification('成功', '您已成功退出登录', 'success')
      } catch (error) {
        console.error('退出登录失败:', error)
        this.showNotification('错误', '退出登录失败', 'danger')
      }
    },
    
    async loadInitialData() {
  // 记录每个接口的加载状态，便于后续排查
  const loadStatus = {
    devices: false,
    alarms: false,
    alarmStats: false,
    statistics: false
  };

  try {
    console.log('开始加载初始数据...');

    // 1. 加载设备列表（允许失败，不阻断后续）
    try {
      console.log('开始加载设备列表...');
      await this.$store.dispatch('devices/fetchDevices');
      loadStatus.devices = true;
      console.log('设备列表加载完成');
    } catch (error) {
      console.error('设备列表加载失败:', error);
      this.showNotification('警告', '设备列表加载失败', 'warning');
    }

    // 2. 加载告警列表（依赖前序接口失败仍继续）
    try {
      console.log('开始加载告警列表...');
      await this.$store.dispatch('alarms/fetchAlarms');
      loadStatus.alarms = true;
      console.log('告警列表加载完成');
    } catch (error) {
      console.error('告警列表加载失败:', error);
      this.showNotification('警告', '告警列表加载失败', 'warning');
    }

    // 3. 加载告警统计
    try {
      console.log('开始加载告警统计...');
      await this.$store.dispatch('alarms/fetchAlarmStats');
      loadStatus.alarmStats = true;
      console.log('告警统计加载完成');
    } catch (error) {
      console.error('告警统计加载失败:', error);
      this.showNotification('警告', '告警统计加载失败', 'warning');
    }

    // 4. 加载数据统计
    try {
      console.log('开始加载数据统计...');
      await this.$store.dispatch('statistics/fetchAllStatistics');
      loadStatus.statistics = true;
      console.log('数据统计加载完成');
    } catch (error) {
      console.error('数据统计加载失败:', error);
      this.showNotification('警告', '数据统计加载失败', 'warning');
    }

    // 汇总加载结果
    const allSuccess = Object.values(loadStatus).every(status => status);
    if (allSuccess) {
      this.showNotification('成功', '所有初始数据加载完成', 'success');
    } else {
      const failedModules = Object.entries(loadStatus)
        .filter(([_, status]) => !status)
        .map(([module]) => module);
      this.showNotification('提示', `部分数据加载完成，失败模块：${failedModules.join(', ')}`, 'info');
    }

  } catch (error) {
    // 捕获非接口调用本身的异常（如代码语法错误）
    console.error('初始数据加载流程异常:', error);
    this.showNotification('错误', '数据加载流程发生未知错误', 'danger');
  }
},
    
    setupAutoRefresh() {
      // 关键修改：延长刷新间隔（从30秒改为60秒），减少接口请求频率
      const refreshInterval = 60000 // 60秒
      if (this.autoRefreshInterval) {
        clearInterval(this.autoRefreshInterval) // 避免重复创建定时器
      }
      
      this.autoRefreshInterval = setInterval(async () => {
        if (this.isAuthenticated) {
          await this.refreshData()
        }
      }, refreshInterval)
    },
    
    async refreshData() {
      try {
        // 自动刷新时仍用串行，且忽略统计接口（非核心数据）
        await this.$store.dispatch('devices/fetchDevices')
        await this.$store.dispatch('alarms/fetchAlarms')
        await this.$store.dispatch('alarms/fetchAlarmStats')
        console.log('数据自动刷新完成')
      } catch (error) {
        console.error('自动刷新数据失败:', error)
        // 自动刷新失败不弹窗，避免打扰用户，仅打印日志
      }
    },
    
    showNotification(title, message, type = 'info') {
      this.notifications.push({
        title,
        message,
        type,
        timestamp: Date.now()
      })
      
      // 5秒后自动移除
      setTimeout(() => {
        this.removeNotification(this.notifications.length - 1)
      }, 5000)
    },
    
    removeNotification(index) {
      this.notifications.splice(index, 1)
    }
  }
}
</script>

<style>
#app {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
}

main {
  padding: 20px;
  min-height: calc(100vh - 76px);
}

.navbar-brand {
  font-weight: 700;
  font-size: 1.25rem;
}

.nav-link {
  font-weight: 500;
}

.nav-link.active {
  font-weight: 600;
}

.dropdown-menu {
  box-shadow: 0 0.5rem 1rem rgba(0, 0, 0, 0.15);
}

.toast {
  min-width: 300px;
}
</style>