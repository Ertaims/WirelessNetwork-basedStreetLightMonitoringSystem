<template>
  <div class="login-container d-flex justify-content-center align-items-center vh-100">
    <div class="card shadow-lg" style="width: 400px;">
      <div class="card-body p-5">
        <h2 class="card-title text-center mb-4">智能路灯管理系统</h2>
        <form @submit.prevent="handleLogin">
          <!-- 用户名输入框（不变） -->
          <div class="mb-3">
            <label for="username" class="form-label">用户名</label>
            <input 
              type="text" 
              class="form-control" 
              id="username" 
              v-model="form.username" 
              required
              :class="{ 'is-invalid': errors.username }"
              placeholder="请输入用户名"
            >
            <div class="invalid-feedback" v-if="errors.username">
              {{ errors.username }}
            </div>
          </div>
          <!-- 密码输入框（不变） -->
          <div class="mb-3">
            <label for="password" class="form-label">密码</label>
            <input 
              type="password" 
              class="form-control" 
              id="password" 
              v-model="form.password" 
              required
              :class="{ 'is-invalid': errors.password }"
              placeholder="请输入密码"
            >
            <div class="invalid-feedback" v-if="errors.password">
              {{ errors.password }}
            </div>
          </div>
          <!-- 记住我（不变） -->
          <div class="mb-3 form-check">
            <input 
              type="checkbox" 
              class="form-check-input" 
              id="remember"
              v-model="form.remember"
            >
            <label class="form-check-label" for="remember">
              记住我
            </label>
          </div>
          <!-- 🔴 新增：提交错误提示（如用户名密码错误、后端500等） -->
          <div v-if="errors.submit" class="mb-3 text-danger text-center">
            {{ errors.submit }}
          </div>
          <!-- 登录按钮（不变） -->
          <div class="d-grid">
            <button 
              type="submit" 
              class="btn btn-primary btn-lg" 
              :disabled="loading"
            >
              <span v-if="loading" class="spinner-border spinner-border-sm me-2"></span>
              {{ loading ? '登录中...' : '登录' }}
            </button>
          </div>
        </form>
        <div class="mt-4 text-center text-muted">
          <small>演示账号: admin / 123456</small>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { mapActions } from 'vuex'

export default {
  name: 'Login',
  data() {
    return {
      form: {
        username: '',
        password: '',
        remember: false
      },
      errors: {}, // 存储表单错误和提交错误
      loading: false
    }
  },
  methods: {
    ...mapActions('auth', ['login']), // 正确映射Vuex的auth/login action
    
    validateForm() {
      this.errors = {} // 清空之前的错误
      
      // 用户名验证
      if (!this.form.username.trim()) {
        this.errors.username = '请输入用户名'
      }
      
      // 密码验证
      if (!this.form.password) {
        this.errors.password = '请输入密码'
      }
      
      return Object.keys(this.errors).length === 0 // 无错误返回true
    },
    
    async handleLogin() {
      if (!this.validateForm()) return // 表单验证失败直接返回
      
      this.loading = true
      this.errors.submit = '' // 清空之前的提交错误
      
      try {
        // 调用Vuex action，传递表单对象（正确：作为请求体）
        const result = await this.login(this.form)
        
        if (result.success) {
          // 登录成功：获取用户信息→加载初始数据→跳首页
          await this.$store.dispatch('auth/getProfile')
          await this.loadInitialData()
          this.$router.push('/')
        } else {
          // 🔴 优化：添加默认错误提示，避免undefined
          this.errors.submit = result.error || '登录失败，请重试'
        }
      } catch (error) {
        // 🔴 优化：覆盖更多错误场景（如网络断开、后端无响应）
        this.errors.submit = error.response?.data?.message 
          || error.message 
          || '登录失败，请检查网络或联系管理员'
      } finally {
        this.loading = false // 无论成功失败，关闭加载状态
      }
    },
    
    async loadInitialData() {
      try {
        // 并行加载初始数据（依赖devices/alarms/statistics模块的action）
        await Promise.all([
          this.$store.dispatch('devices/fetchDevices'),
          this.$store.dispatch('alarms/fetchAlarms'),
          this.$store.dispatch('statistics/fetchDeviceStats')
        ])
      } catch (error) {
        console.error('加载初始数据失败:', error)
        // 可选：给用户提示（不阻断登录，仅日志报错）
        // ElMessage.warning('部分初始数据加载失败，不影响系统使用')
      }
    }
  }
}
</script>

<style scoped>
.login-container {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
}

.card {
  border: none;
  border-radius: 15px;
}

.btn-primary {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  border: none;
}

.btn-primary:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 15px rgba(102, 126, 234, 0.4);
}
</style>