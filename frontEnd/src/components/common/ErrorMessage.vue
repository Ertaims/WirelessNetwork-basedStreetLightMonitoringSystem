<template>
  <div class="error-message text-center py-5">
    <div class="error-icon mb-3">
      <i class="fas fa-exclamation-triangle fa-3x text-warning"></i>
    </div>
    <h4 class="text-danger mb-3">{{ title }}</h4>
    <p class="text-muted mb-4">{{ message }}</p>
    <button @click="handleRetry" class="btn btn-primary" :disabled="loading">
      <span v-if="loading" class="spinner-border spinner-border-sm me-2"></span>
      {{ loading ? '重试中...' : '重试' }}
    </button>
    <div v-if="showDetails" class="mt-3">
      <details class="text-start">
        <summary class="btn btn-sm btn-outline-secondary">查看详情</summary>
        <pre class="mt-2 p-3 bg-light border rounded small">{{ errorDetails }}</pre>
      </details>
    </div>
  </div>
</template>

<script>
export default {
  name: 'ErrorMessage',
  props: {
    title: {
      type: String,
      default: '出错了'
    },
    message: {
      type: String,
      default: '加载数据时出现错误，请稍后重试。'
    },
    error: {
      type: [Error, String],
      default: null
    },
    showRetry: {
      type: Boolean,
      default: true
    },
    showDetails: {
      type: Boolean,
      default: false
    }
  },
  data() {
    return {
      loading: false
    }
  },
  computed: {
    errorDetails() {
      if (!this.error) return '无详细错误信息'
      
      if (typeof this.error === 'string') {
        return this.error
      }
      
      if (this.error.response) {
        return JSON.stringify({
          status: this.error.response.status,
          statusText: this.error.response.statusText,
          data: this.error.response.data,
          url: this.error.response.config?.url
        }, null, 2)
      }
      
      return this.error.message || this.error.toString()
    }
  },
  methods: {
    async handleRetry() {
      this.loading = true
      try {
        await this.$emit('retry')
      } finally {
        this.loading = false
      }
    }
  }
}
</script>

<style scoped>
.error-message {
  min-height: 300px;
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
}

pre {
  max-height: 200px;
  overflow-y: auto;
  white-space: pre-wrap;
  word-break: break-all;
}
</style>