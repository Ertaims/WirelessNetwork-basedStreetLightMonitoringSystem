<template>
  <div class="alarm-management">
    <div class="d-flex justify-content-between align-items-center mb-4">
      <h1>告警管理</h1>
      <div class="btn-group">
        <button 
          class="btn btn-outline-secondary" 
          :class="{ active: filterStatus === 'all' }"
          @click="filterStatus = 'all'"
        >
          全部
        </button>
        <button 
          class="btn btn-outline-secondary" 
          :class="{ active: filterStatus === 'pending' }"
          @click="filterStatus = 'pending'"
        >
          待处理
        </button>
        <button 
          class="btn btn-outline-secondary" 
          :class="{ active: filterStatus === 'confirmed' }"
          @click="filterStatus = 'confirmed'"
        >
          已确认
        </button>
        <button 
          class="btn btn-outline-secondary" 
          :class="{ active: filterStatus === 'resolved' }"
          @click="filterStatus = 'resolved'"
        >
          已解决
        </button>
      </div>
    </div>

    <div class="card">
      <div class="card-body p-0">
        <table class="table table-hover mb-0">
          <thead>
            <tr>
              <th>设备名称</th>
              <th>告警类型</th>
              <th>告警级别</th>
              <th>发生时间</th>
              <th>状态</th>
              <th>操作</th>
            </tr>
          </thead>
          <tbody>
            <tr v-for="alarm in filteredAlarms" :key="alarm.id">
              <td>{{ alarm.deviceName }}</td>
              <td>{{ alarm.type }}</td>
              <td>
                <span class="badge" :class="levelBadgeClass(alarm)">
                  {{ levelText(alarm) }}
                </span>
              </td>
              <td>{{ formatTime(alarm.timestamp) }}</td>
              <td>
                <span class="badge" :class="statusBadgeClass(alarm)">
                  {{ statusText(alarm) }}
                </span>
              </td>
              <td>
                <div class="btn-group btn-group-sm">
                  <button 
                    v-if="alarm.status === 'pending'" 
                    class="btn btn-outline-primary"
                    @click="showAlarmDetail(alarm)"
                  >
                    确认
                  </button>
                  <button 
                    v-if="alarm.status === 'confirmed'" 
                    class="btn btn-outline-success"
                    @click="showAlarmDetail(alarm)"
                  >
                    解决
                  </button>
                  <button 
                    class="btn btn-outline-info"
                    @click="showAlarmDetail(alarm)"
                  >
                    详情
                  </button>
                </div>
              </td>
            </tr>
            <tr v-if="filteredAlarms.length === 0">
              <td colspan="6" class="text-center text-muted py-4">
                暂无告警数据
              </td>
            </tr>
          </tbody>
        </table>
      </div>
    </div>

    <!-- 告警详情模态框 -->
    <div class="modal fade" id="alarmDetailModal" tabindex="-1">
      <div class="modal-dialog">
        <div class="modal-content">
          <div class="modal-header">
            <h5 class="modal-title">告警详情</h5>
            <button type="button" class="btn-close" data-bs-dismiss="modal"></button>
          </div>
          <div class="modal-body" v-if="selectedAlarm">
            <div class="row">
              <div class="col-md-6">
                <h6>告警信息</h6>
                <table class="table table-sm">
                  <tbody>
                    <tr>
                      <td>设备名称:</td>
                      <td>{{ selectedAlarm.deviceName }}</td>
                    </tr>
                    <tr>
                      <td>告警类型:</td>
                      <td>{{ selectedAlarm.type }}</td>
                    </tr>
                    <tr>
                      <td>告警级别:</td>
                      <td>
                        <span class="badge" :class="levelBadgeClass(selectedAlarm)">
                          {{ levelText(selectedAlarm) }}
                        </span>
                      </td>
                    </tr>
                    <tr>
                      <td>发生时间:</td>
                      <td>{{ formatTime(selectedAlarm.timestamp) }}</td>
                    </tr>
                    <tr>
                      <td>当前状态:</td>
                      <td>
                        <span class="badge" :class="statusBadgeClass(selectedAlarm)">
                          {{ statusText(selectedAlarm) }}
                        </span>
                      </td>
                    </tr>
                    <tr>
                      <td>告警描述:</td>
                      <td>{{ selectedAlarm.description || '无' }}</td>
                    </tr>
                  </tbody>
                </table>
              </div>
              <div class="col-md-6">
                <h6>处理信息</h6>
                <div class="mb-3">
                  <label class="form-label">处理备注</label>
                  <textarea 
                    class="form-control" 
                    rows="3" 
                    v-model="remark"
                    placeholder="请输入处理备注信息"
                  ></textarea>
                </div>
                <div v-if="selectedAlarm.remark" class="alert alert-info p-2">
                  <small>历史备注: {{ selectedAlarm.remark }}</small>
                </div>
              </div>
            </div>
          </div>
          <div class="modal-footer">
            <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">关闭</button>
            <button 
              v-if="selectedAlarm && selectedAlarm.status === 'pending'" 
              type="button" 
              class="btn btn-primary"
              @click="handleConfirmAlarm"
            >
              确认告警
            </button>
            <button 
              v-if="selectedAlarm && selectedAlarm.status === 'confirmed'" 
              type="button" 
              class="btn btn-success"
              @click="handleResolveAlarm"
            >
              标记为解决
            </button>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { mapGetters, mapActions } from 'vuex'
import { Modal } from 'bootstrap'

export default {
  name: 'AlarmManagement',
  data() {
    return {
      filterStatus: 'all',
      selectedAlarm: null,
      remark: '',
      modal: null
    }
  },
  computed: {
    ...mapGetters('alarms', ['filteredAlarms']),
  },
  watch: {
    filterStatus(newVal) {
      this.updateFilters({ status: newVal })
    }
  },
  mounted() {
    this.fetchAlarms()
    this.fetchAlarmStats()
    this.modal = new Modal(document.getElementById('alarmDetailModal'))
    console.log('组件加载时过滤后告警数据:', this.filteredAlarms)
    console.log('Vuex模块是否存在:', this.$store.hasModule('alarms'))
  },
  methods: {
    ...mapActions('alarms', ['fetchAlarms', 'fetchAlarmStats', 'confirmAlarm', 'resolveAlarm', 'updateFilters']),
    formatTime(timestamp) {
      if (!timestamp) return '无'
      try {
        return new Date(timestamp).toLocaleString('zh-CN', {
          year: 'numeric',
          month: '2-digit',
          day: '2-digit',
          hour: '2-digit',
          minute: '2-digit',
          second: '2-digit'
        })
      } catch (error) {
        console.error('时间格式化失败:', error)
        return timestamp
      }
    },
    levelBadgeClass(alarm) {
      if (alarm.level === 'high') return 'bg-danger'
      if (alarm.level === 'medium') return 'bg-warning'
      if (alarm.level === 'low') return 'bg-info'
      return 'bg-secondary'
    },
    levelText(alarm) {
      if (alarm.level === 'high') return '高'
      if (alarm.level === 'medium') return '中'
      if (alarm.level === 'low') return '低'
      return '未知'
    },
    statusBadgeClass(alarm) {
      if (alarm.status === 'pending') return 'bg-warning'
      if (alarm.status === 'confirmed') return 'bg-primary'
      if (alarm.status === 'resolved') return 'bg-success'
      return 'bg-secondary'
    },
    statusText(alarm) {
      if (alarm.status === 'pending') return '待处理'
      if (alarm.status === 'confirmed') return '已确认'
      if (alarm.status === 'resolved') return '已解决'
      return '未知'
    },
    showAlarmDetail(alarm) {
      this.selectedAlarm = { ...alarm }
      this.remark = ''
      this.modal.show()
    },
    async handleConfirmAlarm() {
      if (!this.selectedAlarm) return
      try {
        await this.confirmAlarm({ id: this.selectedAlarm.id, remark: this.remark })
        this.modal.hide()
        alert('告警已确认')
        this.fetchAlarms()
      } catch (error) {
        console.error('确认告警失败:', error)
        alert('确认告警失败：' + (error.message || '未知错误'))
      }
    },
    async handleResolveAlarm() {
      if (!this.selectedAlarm) return
      try {
        await this.resolveAlarm({ id: this.selectedAlarm.id, remark: this.remark })
        this.modal.hide()
        alert('告警已标记为解决')
        this.fetchAlarms()
      } catch (error) {
        console.error('解决告警失败:', error)
        alert('解决告警失败：' + (error.message || '未知错误'))
      }
    }
  }
}
</script>