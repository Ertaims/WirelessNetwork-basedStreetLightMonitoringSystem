<template>
  <div class="device-control">
    <div class="d-flex justify-content-between align-items-center mb-4">
      <h1>设备监控与控制</h1>
      <button class="btn btn-primary" @click="refreshData">
        <i class="fas fa-sync-alt" :class="{ 'fa-spin': refreshing }"></i>
        刷新
      </button>
    </div>
    
    <LoadingSpinner v-if="loading && devices.length === 0" />
    <ErrorMessage v-else-if="error" :message="error" @retry="fetchDevices" />
    
    <div v-else class="row">
      <!-- 设备列表 -->
      <div class="col-md-4">
        <div class="card">
          <div class="card-header d-flex justify-content-between align-items-center">
            <h5 class="card-title mb-0">设备列表</h5>
            <span class="badge bg-primary">{{ devices.length }} 台设备</span>
          </div>
          <div class="card-body p-0">
            <div class="list-group list-group-flush">
              <div 
                v-for="device in devices" 
                :key="device.id"
                class="list-group-item list-group-item-action"
                :class="{ 
                  active: selectedDevice?.id === device.id,
                  'list-group-item-warning': device.status === 'fault',
                  'list-group-item-secondary': device.status === 'offline'
                }"
                @click="selectDevice(device)"
              >
                <div class="d-flex w-100 justify-content-between align-items-start">
                  <div class="flex-grow-1">
                    <h6 class="mb-1">{{ device.name }}</h6>
                    <p class="mb-1 text-muted small">{{ device.location }}</p>
                    <div class="d-flex align-items-center">
                      <span class="badge me-1" :class="statusBadgeClass(device)">
                        {{ statusText(device) }}
                      </span>
                      <!-- 修复：分组字段已映射为 device.group -->
                      <small class="text-muted">分组: {{ device.group }}</small>
                    </div>
                  </div>
                  <div class="text-end">
                    <div class="fw-bold">{{ device.brightness }}%</div>
                    <small class="text-muted">{{ (device.voltage * device.current).toFixed(1) }}W</small>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
        
        <!-- 分组控制 -->
        <div class="card mt-4">
          <div class="card-header">
            <h5 class="card-title mb-0">分组控制</h5>
          </div>
          <div class="card-body">
            <div class="mb-3">
              <label class="form-label">选择分组</label>
              <select class="form-select" v-model="selectedGroup">
                <option value="">全部设备</option>
                <!-- 修复：分组下拉框已能正确获取 group 列表 -->
                <option v-for="group in Object.keys(deviceGroups)" :key="group">
                  {{ group }}
                </option>
              </select>
            </div>
            <div class="btn-group w-100 mb-2">
              <button 
                class="btn btn-success" 
                @click="groupControl('on')"
                :disabled="groupControlLoading"
              >
                <span v-if="groupControlLoading" class="spinner-border spinner-border-sm me-2"></span>
                开启选中分组
              </button>
              <button 
                class="btn btn-secondary" 
                @click="groupControl('off')"
                :disabled="groupControlLoading"
              >
                <span v-if="groupControlLoading" class="spinner-border spinner-border-sm me-2"></span>
                关闭选中分组
              </button>
            </div>
            <div class="input-group">
              <input 
                type="range" 
                class="form-range" 
                min="0" 
                max="100" 
                v-model="groupBrightness"
                :disabled="groupControlLoading"
              >
              <button 
                class="btn btn-outline-primary" 
                @click="setGroupBrightness"
                :disabled="groupControlLoading"
              >
                设置亮度 {{ groupBrightness }}%
              </button>
            </div>
          </div>
        </div>
      </div>
      
      <!-- 设备控制面板 -->
      <div class="col-md-8">
        <div class="card" v-if="selectedDevice">
          <div class="card-header d-flex justify-content-between align-items-center">
            <h5 class="card-title mb-0">
              设备控制 - {{ selectedDevice.name }}
              <small class="text-muted ms-2">{{ selectedDevice.location }}</small>
            </h5>
            <div>
              <span class="badge me-2" :class="statusBadgeClass(selectedDevice)">
                {{ statusText(selectedDevice) }}
              </span>
              <span class="badge bg-dark">{{ selectedDevice.group }}</span>
            </div>
          </div>
          
          <div class="card-body">
            <!-- 实时数据 -->
            <div class="row mb-4">
              <div class="col-md-3 text-center">
                <div class="card bg-light">
                  <div class="card-body py-3">
                    <h6 class="card-title text-muted">电压</h6>
                    <h4 class="text-primary">{{ selectedDevice.voltage }}V</h4>
                  </div>
                </div>
              </div>
              <div class="col-md-3 text-center">
                <div class="card bg-light">
                  <div class="card-body py-3">
                    <h6 class="card-title text-muted">电流</h6>
                    <h4 class="text-info">{{ selectedDevice.current }}A</h4>
                  </div>
                </div>
              </div>
              <div class="col-md-3 text-center">
                <div class="card bg-light">
                  <div class="card-body py-3">
                    <h6 class="card-title text-muted">功率</h6>
                    <h4 class="text-success">{{ (selectedDevice.voltage * selectedDevice.current).toFixed(1) }}W</h4>
                  </div>
                </div>
              </div>
              <div class="col-md-3 text-center">
                <div class="card bg-light">
                  <div class="card-body py-3">
                    <h6 class="card-title text-muted">运行时间</h6>
                    <!-- 修复：运行时间字段已映射为 device.uptime -->
                    <h4 class="text-warning">{{ selectedDevice.uptime || 0 }}h</h4>
                  </div>
                </div>
              </div>
            </div>
            
            <!-- 控制面板（其余逻辑不变，因字段已在 Vuex 中格式化，无需修改） -->
            <div class="control-panel">
              <h6 class="border-bottom pb-2">设备控制</h6>
              
              <!-- 开关控制 -->
              <div class="row mb-4">
                <div class="col-md-6">
                  <label class="form-label fw-bold">电源开关</label>
                  <div class="form-check form-switch">
                    <input 
                      class="form-check-input" 
                      type="checkbox" 
                      :checked="selectedDevice.power === 'on'"
                      @change="toggleDevicePower(selectedDevice)"
                      :disabled="selectedDevice.status !== 'online' || controlLoading"
                    >
                    <label class="form-check-label">
                      {{ selectedDevice.power === 'on' ? '设备已开启' : '设备已关闭' }}
                    </label>
                  </div>
                </div>
                <div class="col-md-6">
                  <label class="form-label fw-bold">自动模式</label>
                  <div class="form-check form-switch">
                    <input 
                      class="form-check-input" 
                      type="checkbox" 
                      :checked="selectedDevice.autoMode"
                      @change="toggleAutoMode(selectedDevice)"
                      :disabled="selectedDevice.status !== 'online' || controlLoading"
                    >
                    <label class="form-check-label">
                      {{ selectedDevice.autoMode ? '自动模式开启' : '自动模式关闭' }}
                    </label>
                  </div>
                </div>
              </div>
              
              <!-- 亮度调节、定时控制、高级设置（均无需修改） -->
              <div class="mb-4">
                <label class="form-label fw-bold d-flex justify-content-between">
                  <span>亮度调节</span>
                  <span class="text-primary">{{ selectedDevice.brightness }}%</span>
                </label>
                <input 
                  type="range" 
                  class="form-range" 
                  min="0" 
                  max="100" 
                  :value="selectedDevice.brightness"
                  @input="updateDeviceBrightness(selectedDevice, $event.target.value)"
                  :disabled="selectedDevice.status !== 'online' || selectedDevice.power === 'off' || controlLoading"
                >
                <div class="d-flex justify-content-between text-muted small">
                  <span>0%</span>
                  <span>25%</span>
                  <span>50%</span>
                  <span>75%</span>
                  <span>100%</span>
                </div>
              </div>
              
              <div class="mb-4">
                <label class="form-label fw-bold">定时控制</label>
                <div class="row g-2">
                  <div class="col-md-6">
                    <label class="form-label small">开启时间</label>
                    <input 
                      type="time" 
                      class="form-control" 
                      v-model="schedule.onTime"
                      :disabled="controlLoading"
                    >
                  </div>
                  <div class="col-md-6">
                    <label class="form-label small">关闭时间</label>
                    <input 
                      type="time" 
                      class="form-control" 
                      v-model="schedule.offTime"
                      :disabled="controlLoading"
                    >
                  </div>
                </div>
                <div class="mt-2">
                  <button 
                    class="btn btn-outline-primary btn-sm"
                    @click="setSchedule(selectedDevice)"
                    :disabled="controlLoading"
                  >
                    <span v-if="controlLoading" class="spinner-border spinner-border-sm me-2"></span>
                    设置定时
                  </button>
                </div>
              </div>
              
              <div class="advanced-settings">
                <h6 class="border-bottom pb-2">高级设置</h6>
                <div class="row g-3">
                  <div class="col-md-6">
                    <label class="form-label small">电压阈值</label>
                    <input 
                      type="number" 
                      class="form-control" 
                      v-model="advancedSettings.voltageThreshold"
                      placeholder="电压报警阈值"
                    >
                  </div>
                  <div class="col-md-6">
                    <label class="form-label small">电流阈值</label>
                    <input 
                      type="number" 
                      class="form-control" 
                      step="0.1"
                      v-model="advancedSettings.currentThreshold"
                      placeholder="电流报警阈值"
                    >
                  </div>
                </div>
                <div class="mt-3">
                  <button 
                    class="btn btn-outline-secondary btn-sm"
                    @click="saveAdvancedSettings(selectedDevice)"
                    :disabled="controlLoading"
                  >
                    保存设置
                  </button>
                </div>
              </div>
            </div>
          </div>
          
          <div class="card-footer text-muted small">
            最后更新: {{ formatTime(selectedDevice.lastUpdate) || '未知' }}
            <button class="btn btn-sm btn-outline-info float-end" @click="fetchDeviceDetail(selectedDevice.id)">
              刷新数据
            </button>
          </div>
        </div>
        
        <div class="card" v-else>
          <div class="card-body text-center text-muted py-5">
            <i class="fas fa-lightbulb fa-3x mb-3"></i>
            <p>请从左侧选择一个设备进行控制</p>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { mapState, mapActions, mapGetters } from 'vuex'
import LoadingSpinner from './common/LoadingSpinner.vue'
import ErrorMessage from './common/ErrorMessage.vue'

export default {
  name: 'DeviceControl',
  components: {
    LoadingSpinner,
    ErrorMessage
  },
  data() {
    return {
      selectedDevice: null,
      selectedGroup: '',
      groupBrightness: 80,
      controlLoading: false,
      groupControlLoading: false,
      refreshing: false,
      schedule: {
        onTime: '18:00',
        offTime: '06:00'
      },
      advancedSettings: {
        voltageThreshold: 240,
        currentThreshold: 1.5
      }
    }
  },
  computed: {
    ...mapState('devices', ['devices', 'loading', 'error']),
    ...mapGetters('devices', ['deviceGroups'])
  },
  async mounted() {
    await this.fetchDevices()
    console.log('设备列表加载后数据:', this.devices)
    console.log('设备分组:', this.deviceGroups)
  },
  methods: {
    ...mapActions('devices', [
      'fetchDevices', 
      'fetchDevice', 
      'controlDevice', 
      'controlGroup',
      'updateDevice'
    ]),
    
    // 新增：时间格式化（统一显示格式）
    formatTime(timeStr) {
      if (!timeStr) return '未知'
      try {
        return new Date(timeStr).toLocaleString('zh-CN', {
          year: 'numeric',
          month: '2-digit',
          day: '2-digit',
          hour: '2-digit',
          minute: '2-digit',
          second: '2-digit'
        })
      } catch (error) {
        return timeStr
      }
    },
    
    statusBadgeClass(device) {
      if (device.status === 'online') {
        return device.power === 'on' ? 'bg-success' : 'bg-secondary'
      } else if (device.status === 'offline') {
        return 'bg-dark'
      } else if (device.status === 'fault') {
        return 'bg-danger'
      }
      return 'bg-secondary'
    },
    
    statusText(device) {
      if (device.status === 'online') {
        return device.power === 'on' ? '在线-开启' : '在线-关闭'
      } else if (device.status === 'offline') {
        return '离线'
      } else if (device.status === 'fault') {
        return '故障'
      }
      return '未知'
    },
    
    async selectDevice(device) {
      try {
        const deviceDetail = await this.fetchDevice(device.id)
        this.selectedDevice = deviceDetail
      } catch (error) {
        console.error('获取设备详情失败:', error)
        this.selectedDevice = device
      }
    },
    
    async fetchDeviceDetail(deviceId) {
      try {
        const deviceDetail = await this.fetchDevice(deviceId)
        this.selectedDevice = deviceDetail
      } catch (error) {
        console.error('刷新设备数据失败:', error)
      }
    },
    
    async toggleDevicePower(device) {
      this.controlLoading = true
      try {
        const command = {
          action: device.power === 'on' ? 'turn_off' : 'turn_on',
          brightness: device.power === 'on' ? 0 : 80
        }
        
        await this.controlDevice({
          id: device.id,
          command
        })
      } catch (error) {
        console.error('控制设备失败:', error)
      } finally {
        this.controlLoading = false
      }
    },
    
    async toggleAutoMode(device) {
      this.controlLoading = true
      try {
        await this.updateDevice({
          id: device.id,
          data: { autoMode: !device.autoMode }
        })
      } catch (error) {
        console.error('切换自动模式失败:', error)
      } finally {
        this.controlLoading = false
      }
    },
    
    async updateDeviceBrightness(device, brightness) {
      this.controlLoading = true
      try {
        const command = {
          action: 'set_brightness',
          brightness: parseInt(brightness)
        }
        
        await this.controlDevice({
          id: device.id,
          command
        })
      } catch (error) {
        console.error('调节亮度失败:', error)
      } finally {
        this.controlLoading = false
      }
    },
    
    async groupControl(action) {
      if (!this.selectedGroup) {
        alert('请先选择分组')
        return
      }
      
      this.groupControlLoading = true
      try {
        const command = {
          action: action === 'on' ? 'turn_on' : 'turn_off',
          brightness: action === 'on' ? this.groupBrightness : 0
        }
        
        await this.controlGroup({
          group: this.selectedGroup,
          command
        })
        
        await this.fetchDevices()
      } catch (error) {
        console.error('分组控制失败:', error)
        alert('分组控制失败: ' + (error.response?.data?.message || error.message))
      } finally {
        this.groupControlLoading = false
      }
    },
    
    async setGroupBrightness() {
      if (!this.selectedGroup) {
        alert('请先选择分组')
        return
      }
      
      this.groupControlLoading = true
      try {
        const command = {
          action: 'set_brightness',
          brightness: this.groupBrightness
        }
        
        await this.controlGroup({
          group: this.selectedGroup,
          command
        })
        
        await this.fetchDevices()
      } catch (error) {
        console.error('设置分组亮度失败:', error)
        alert('设置分组亮度失败: ' + (error.response?.data?.message || error.message))
      } finally {
        this.groupControlLoading = false
      }
    },
    
    async setSchedule(device) {
      this.controlLoading = true
      try {
        await this.updateDevice({
          id: device.id,
          data: {
            schedule: {
              onTime: this.schedule.onTime,
              offTime: this.schedule.offTime
            }
          }
        })
        alert('定时设置已保存')
      } catch (error) {
        console.error('设置定时失败:', error)
        alert('设置定时失败: ' + (error.response?.data?.message || error.message))
      } finally {
        this.controlLoading = false
      }
    },
    
    async saveAdvancedSettings(device) {
      this.controlLoading = true
      try {
        await this.updateDevice({
          id: device.id,
          data: {
            thresholds: {
              voltage: this.advancedSettings.voltageThreshold,
              current: this.advancedSettings.currentThreshold
            }
          }
        })
        alert('高级设置已保存')
      } catch (error) {
        console.error('保存设置失败:', error)
        alert('保存设置失败: ' + (error.response?.data?.message || error.message))
      } finally {
        this.controlLoading = false
      }
    },
    
    async refreshData() {
      this.refreshing = true
      try {
        await this.fetchDevices()
        if (this.selectedDevice) {
          await this.fetchDeviceDetail(this.selectedDevice.id)
        }
      } catch (error) {
        console.error('刷新数据失败:', error)
      } finally {
        this.refreshing = false
      }
    }
  }
}
</script>

<style scoped>
.list-group-item {
  cursor: pointer;
  transition: all 0.2s;
}

.list-group-item:hover {
  transform: translateX(5px);
}

.control-panel {
  background: #f8f9fa;
  padding: 1.5rem;
  border-radius: 0.5rem;
}

.advanced-settings {
  background: #e9ecef;
  padding: 1rem;
  border-radius: 0.5rem;
  margin-top: 1rem;
}

.form-range::-webkit-slider-thumb {
  background: #0d6efd;
}

.form-range::-moz-range-thumb {
  background: #0d6efd;
}
</style>