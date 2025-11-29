<template>
  <div class="device-overview">
    <div class="d-flex justify-content-between align-items-center mb-4">
      <h1>设备总览</h1>
      <div class="btn-group">
        <button 
          class="btn btn-outline-secondary" 
          :class="{ active: viewMode === 'card' }"
          @click="viewMode = 'card'"
        >
          卡片视图
        </button>
        <button 
          class="btn btn-outline-secondary" 
          :class="{ active: viewMode === 'map' }"
          @click="viewMode = 'map'"
        >
          地图视图
        </button>
      </div>
    </div>
    
    <!-- 卡片视图 -->
    <div v-if="viewMode === 'card'" class="row">
      <div 
        v-for="device in devices" 
        :key="device.id"
        class="col-md-4 mb-4"
      >
        <div class="card h-100">
          <div class="card-header d-flex justify-content-between align-items-center">
            <h5 class="card-title mb-0">{{ device.name }}</h5>
            <span class="badge" :class="statusBadgeClass(device)">
              {{ statusText(device) }}
            </span>
          </div>
          <div class="card-body">
            <p class="card-text">
              <strong>位置:</strong> {{ device.location }}<br>
              <strong>电压:</strong> {{ device.voltage }}V<br>
              <strong>电流:</strong> {{ device.current }}A<br>
              <strong>亮度:</strong> {{ device.brightness }}%
            </p>
          </div>
          <div class="card-footer">
            <div class="btn-group w-100">
              <button 
                class="btn btn-sm" 
                :class="device.power === 'on' ? 'btn-success' : 'btn-outline-success'"
                @click="toggleDevicePower(device)"
                :disabled="device.status !== 'online'"
              >
                {{ device.power === 'on' ? '关闭' : '开启' }} <!-- 修复：按钮文字与功能一致 -->
              </button>
              <button 
                class="btn btn-sm btn-outline-primary"
                @click="showDeviceDetail(device)"
              >
                详情
              </button>
            </div>
          </div>
        </div>
      </div>
    </div>
    
    <!-- 地图视图 -->
    <div v-if="viewMode === 'map'" class="map-view">
      <div class="card">
        <div class="card-body">
          <div class="map-container position-relative" style="height: 500px; background-color: #e9ecef;">
            <!-- 模拟地图背景 -->
            <div class="position-absolute top-0 start-0 w-100 h-100 d-flex justify-content-center align-items-center text-muted">
              <div class="text-center">
                <i class="fas fa-map-marked-alt fa-5x mb-3"></i>
                <p>地图视图展示区域</p>
              </div>
            </div>
            
            <!-- 设备标记 -->
            <div 
              v-for="device in devices" 
              :key="device.id"
              class="position-absolute device-marker"
              :style="getMarkerPosition(device)"
              @click="showDeviceDetail(device)"
            >
              <i 
                class="fas fa-lightbulb fa-2x" 
                :class="getMarkerClass(device)"
                :title="device.name"
              ></i>
            </div>
          </div>
        </div>
      </div>
    </div>
    
    <!-- 设备详情模态框 -->
    <div class="modal fade" id="deviceDetailModal" tabindex="-1">
      <div class="modal-dialog modal-lg">
        <div class="modal-content">
          <div class="modal-header">
            <h5 class="modal-title">设备详情 - {{ selectedDevice?.name }}</h5>
            <button type="button" class="btn-close" data-bs-dismiss="modal"></button>
          </div>
          <div class="modal-body" v-if="selectedDevice">
            <div class="row">
              <div class="col-md-6">
                <h6>基本信息</h6>
                <table class="table table-sm">
                  <tbody>
                    <tr>
                      <td>设备名称:</td>
                      <td>{{ selectedDevice.name }}</td>
                    </tr>
                    <tr>
                      <td>位置:</td>
                      <td>{{ selectedDevice.location }}</td>
                    </tr>
                    <tr>
                      <td>分组:</td>
                      <td>{{ selectedDevice.group }}</td>
                    </tr>
                    <tr>
                      <td>状态:</td>
                      <td>
                        <span class="badge" :class="statusBadgeClass(selectedDevice)">
                          {{ statusText(selectedDevice) }}
                        </span>
                      </td>
                    </tr>
                  </tbody>
                </table>
              </div>
              <div class="col-md-6">
                <h6>运行参数</h6>
                <table class="table table-sm">
                  <tbody>
                    <tr>
                      <td>电压:</td>
                      <td>{{ selectedDevice.voltage }}V</td>
                    </tr>
                    <tr>
                      <td>电流:</td>
                      <td>{{ selectedDevice.current }}A</td>
                    </tr>
                    <tr>
                      <td>亮度:</td>
                      <td>{{ selectedDevice.brightness }}%</td>
                    </tr>
                    <tr>
                      <td>功率:</td>
                      <td>{{ (selectedDevice.voltage * selectedDevice.current).toFixed(2) }}W</td>
                    </tr>
                  </tbody>
                </table>
              </div>
            </div>
            
            <div class="mt-3">
              <h6>设备控制</h6>
              <div class="d-flex align-items-center flex-wrap">
                <label class="form-label me-3 mb-0">开关:</label>
                <div class="form-check form-switch me-3">
                  <input 
                    class="form-check-input" 
                    type="checkbox" 
                    :checked="selectedDevice.power === 'on'"
                    @change="toggleDevicePower(selectedDevice)"
                    :disabled="selectedDevice.status !== 'online'"
                  >
                  <label class="form-check-label">
                    {{ selectedDevice.power === 'on' ? '开启' : '关闭' }}
                  </label>
                </div>
                
                <label class="form-label me-3 mb-0">亮度:</label>
                <input 
                  type="range" 
                  class="form-range w-50" 
                  min="0" 
                  max="100" 
                  :value="selectedDevice.brightness"
                  @input="updateDeviceBrightness(selectedDevice, $event.target.value)"
                  :disabled="selectedDevice.status !== 'online' || selectedDevice.power === 'off'"
                >
                <span class="ms-2">{{ selectedDevice.brightness }}%</span>
              </div>
            </div>
          </div>
          <div class="modal-footer">
            <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">关闭</button>
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
  name: 'DeviceOverview',
  data() {
    return {
      viewMode: 'card',
      selectedDevice: null,
      modal: null
    }
  },
  computed: {
    // 关键修复1：添加 devices 命名空间，与 Vuex 模块一致
    ...mapGetters('devices', ['devices'])
  },
  mounted() {
    this.fetchDevices() // 加载设备数据
    this.modal = new Modal(document.getElementById('deviceDetailModal'))
    console.log('设备总览加载的数据:', this.devices) // 验证数据是否获取到
  },
  methods: {
    // 关键修复2：添加 devices 命名空间，导入需要的 Action
    ...mapActions('devices', ['fetchDevices', 'controlDevice']),
    
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
    
    getMarkerClass(device) {
      if (device.status === 'online') {
        return device.power === 'on' ? 'text-success' : 'text-secondary'
      } else if (device.status === 'offline') {
        return 'text-dark'
      } else if (device.status === 'fault') {
        return 'text-danger'
      }
      return 'text-secondary'
    },
    
    getMarkerPosition(device) {
      // 模拟设备在地图上的位置（兼容所有10台设备）
      const positions = {
        1: { top: '30%', left: '20%' },
        2: { top: '35%', left: '25%' },
        3: { top: '40%', left: '30%' },
        4: { top: '50%', left: '60%' },
        5: { top: '55%', left: '65%' },
        6: { top: '60%', left: '70%' },
        7: { top: '45%', left: '40%' },
        8: { top: '50%', left: '45%' },
        9: { top: '55%', left: '50%' },
        10: { top: '60%', left: '55%' }
      }
      return positions[device.id] || { top: '50%', left: '50%' }
    },
    
    showDeviceDetail(device) {
      this.selectedDevice = { ...device }
      this.modal.show()
    },
    
    // 关键修复3：调用 Vuex controlDevice Action，参数格式改为 { id, command }
    async toggleDevicePower(device) {
      if (device.status !== 'online') return
      
      try {
        const command = {
          action: device.power === 'on' ? 'turn_off' : 'turn_on',
          brightness: device.power === 'on' ? 0 : 80
        }
        
        await this.controlDevice({ id: device.id, command })
        // 同步更新模态框中的选中设备
        if (this.selectedDevice?.id === device.id) {
          this.selectedDevice.power = command.action === 'turn_on' ? 'on' : 'off'
          this.selectedDevice.brightness = command.brightness
        }
      } catch (error) {
        console.error('控制设备电源失败:', error)
        alert('控制失败：' + (error.message || '未知错误'))
      }
    },
    
    // 关键修复4：亮度调节同样调用 controlDevice，参数格式统一
    async updateDeviceBrightness(device, brightness) {
      if (device.status !== 'online' || device.power === 'off') return
      
      try {
        const command = {
          action: 'set_brightness',
          brightness: parseInt(brightness)
        }
        
        await this.controlDevice({ id: device.id, command })
        // 同步更新模态框中的亮度值
        if (this.selectedDevice?.id === device.id) {
          this.selectedDevice.brightness = parseInt(brightness)
        }
      } catch (error) {
        console.error('调节设备亮度失败:', error)
        alert('调节失败：' + (error.message || '未知错误'))
      }
    }
  }
}
</script>

<style scoped>
.device-marker {
  cursor: pointer;
  transform: translate(-50%, -50%);
  transition: transform 0.2s;
}

.device-marker:hover {
  transform: translate(-50%, -50%) scale(1.2);
}

/* 适配小屏幕，避免控制栏换行混乱 */
@media (max-width: 768px) {
  .d-flex.align-items-center.flex-wrap {
    flex-direction: column;
    align-items: flex-start !important;
    gap: 10px;
  }
}
</style>