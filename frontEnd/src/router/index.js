import { createRouter, createWebHistory } from 'vue-router';
import store from '../store';

const routes = [
  {
    path: '/login',
    name: 'Login',
    // 🔴 核心修正：匹配实际文件位置（components目录）
    component: () => import('../components/Login.vue'),
    meta: { 
      title: '登录 - 智能路灯管理系统',
      requiresGuest: true 
    }
  },
  {
    path: '/',
    name: 'Dashboard',
    component: () => import('../components/Dashboard.vue'),
    meta: { 
      title: '仪表盘 - 智能路灯管理系统',
      requiresAuth: true 
    }
  },
  {
    path: '/devices',
    name: 'Devices',
    component: () => import('../components/DeviceOverview.vue'),
    meta: { 
      title: '设备总览 - 智能路灯管理系统',
      requiresAuth: true 
    }
  },
  {
    path: '/control',
    name: 'Control',
    component: () => import('../components/DeviceControl.vue'),
    meta: { 
      title: '设备控制 - 智能路灯管理系统',
      requiresAuth: true 
    }
  },
  {
    path: '/statistics',
    name: 'Statistics',
    component: () => import('../components/Statistics.vue'),
    meta: { 
      title: '数据统计 - 智能路灯管理系统',
      requiresAuth: true 
    }
  },
  {
    path: '/alarms',
    name: 'Alarms',
    component: () => import('../components/AlarmManagement.vue'),
    meta: { 
      title: '告警管理 - 智能路灯管理系统',
      requiresAuth: true 
    }
  },
  {
    path: '/:pathMatch(.*)*',
    name: 'NotFound',
    component: () => import('../components/NotFound.vue'), // 若未创建，可先改为 redirect: '/'
    meta: { title: '页面未找到 - 智能路灯管理系统' }
  }
];

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes,
  scrollBehavior() {
    return { top: 0 }; // 跳转时滚动到顶部
  }
});

// 路由守卫逻辑不变（权限控制正常）
router.beforeEach((to, from, next) => {
  if (to.meta.title) {
    document.title = to.meta.title;
  }

  const isAuthenticated = store.getters['auth/isAuthenticated'];

  if (to.meta.requiresAuth && !isAuthenticated) {
    next('/login');
  } else if (to.meta.requiresGuest && isAuthenticated) {
    next('/');
  } else {
    next();
  }
});

export default router;