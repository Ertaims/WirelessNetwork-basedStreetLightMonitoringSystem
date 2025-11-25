<!-- src/components/NotFound.vue -->
<template>
  <div class="d-flex flex-column justify-center align-items-center vh-100">
    <h1 class="display-1 text-muted">404</h1>
    <h2 class="mb-4">页面未找到</h2>
    <p class="text-muted mb-4">您访问的页面不存在或已被移动</p>
    <router-link to="/" class="btn btn-primary">返回首页</router-link>
  </div>
</template>