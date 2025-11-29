import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'

export default defineConfig({
  plugins: [vue()],
  // 强制定义环境变量
  define: {
    'import.meta.env.VITE_API_BASE_URL': JSON.stringify('http://localhost:8080/api'),
    'import.meta.env.VITE_APP_TITLE': JSON.stringify('智能路灯管理系统'),
    'import.meta.env.VITE_APP_VERSION': JSON.stringify('1.0.0')
  }
})