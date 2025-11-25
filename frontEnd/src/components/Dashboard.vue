<template>
  <div class="dashboard">
    <h1 class="mb-4">系统仪表盘</h1>
    
    <div class="row mb-4">
      <div class="col-md-3">
        <div class="card text-white bg-primary">
          <div class="card-body">
            <h5 class="card-title">总设备数</h5>
            <p class="card-text display-4">{{ totalDevices }}</p>
          </div>
        </div>
      </div>
      <div class="col-md-3">
        <div class="card text-white bg-success">
          <div class="card-body">
            <h5 class="card-title">在线设备</h5>
            <p class="card-text display-4">{{ onlineDevices }}</p>
          </div>
        </div>
      </div>
      <div class="col-md-3">
        <div class="card text-white bg-warning">
          <div class="card-body">
            <h5 class="card-title">开启设备</h5>
            <p class="card-text display-4">{{ poweredOnDevices }}</p>
          </div>
        </div>
      </div>
      <div class="col-md-3">
        <div class="card text-white bg-danger">
          <div class="card-body">
            <h5 class="card-title">告警设备</h5>
            <p class="card-text display-4">{{ faultDevices }}</p>
          </div>
        </div>
      </div>
    </div>
    
    <div class="row">
      <div class="col-md-8">
        <div class="card">
          <div class="card-header">
            <h5 class="card-title mb-0">设备状态分布</h5>
          </div>
          <div class="card-body">
            <div ref="statusChart" style="height: 300px;"></div>
          </div>
        </div>
      </div>
      <div class="col-md-4">
        <div class="card">
          <div class="card-header">
            <h5 class="card-title mb-0">最新告警</h5>
          </div>
          <div class="card-body p-0">
            <ul class="list-group list-group-flush">
              <li 
                v-for="alarm in recentAlarms" 
                :key="alarm.id"
                class="list-group-item d-flex justify-content-between align-items-start"
              >
                <div>
                  <div class="fw-bold">{{ alarm.deviceName }}</div>
                  <small class="text-muted">{{ alarm.type }}</small>
                </div>
                <span 
                  class="badge" 
                  :class="{
                    'bg-danger': alarm.level === 'high',
                    'bg-warning': alarm.level === 'medium',
                    'bg-info': alarm.level === 'low'
                  }"
                >
                  {{ alarmLevelText[alarm.level] }}
                </span>
              </li>
              <li v-if="recentAlarms.length === 0" class="list-group-item text-center text-muted">
                暂无告警
              </li>
            </ul>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { mapGetters, mapActions } from 'vuex'
import * as echarts from 'echarts'

export default {
  name: 'Dashboard',
  data() {
    return {
      alarmLevelText: {
        high: '高',
        medium: '中',
        low: '低'
      }
    }
  },
  computed: {
    // 关键修改1：给 devices 模块 getter 加命名空间
    ...mapGetters('devices', ['devices']),
    // 关键修改2：给 alarms 模块 getter 加命名空间
    ...mapGetters('alarms', ['alarms']),
    totalDevices() {
      return this.devices.length
    },
    onlineDevices() {
      return this.devices.filter(d => d.status === 'online').length
    },
    poweredOnDevices() {
      return this.devices.filter(d => d.power === 'on').length
    },
    faultDevices() {
      return this.devices.filter(d => d.status === 'fault').length
    },
    recentAlarms() {
      // 容错：避免 alarms 为 undefined 或 null
      return (this.alarms || []).slice(0, 5)
    }
  },
  mounted() {
    this.fetchDevices()
    this.fetchAlarms()
    this.$nextTick(() => {
      this.initChart()
    })
  },
  methods: {
    // 关键修改3：给 devices 模块 action 加命名空间
    ...mapActions('devices', ['fetchDevices']),
    // 关键修改4：给 alarms 模块 action 加命名空间
    ...mapActions('alarms', ['fetchAlarms']),
    initChart() {
      const chart = echarts.init(this.$refs.statusChart)
      
      const statusCount = {
        '在线': this.devices.filter(d => d.status === 'online').length,
        '离线': this.devices.filter(d => d.status === 'offline').length,
        '故障': this.devices.filter(d => d.status === 'fault').length
      }
      
      const option = {
        tooltip: {
          trigger: 'item',
          formatter: '{a} <br/>{b}: {c} ({d}%)'
        },
        legend: {
          orient: 'vertical',
          left: 10,
          data: ['在线', '离线', '故障']
        },
        series: [
          {
            name: '设备状态',
            type: 'pie',
            radius: ['50%', '70%'],
            avoidLabelOverlap: false,
            label: {
              show: false,
              position: 'center'
            },
            emphasis: {
              label: {
                show: true,
                fontSize: '18',
                fontWeight: 'bold'
              }
            },
            labelLine: {
              show: false
            },
            data: [
              { value: statusCount['在线'], name: '在线', itemStyle: { color: '#28a745' } },
              { value: statusCount['离线'], name: '离线', itemStyle: { color: '#6c757d' } },
              { value: statusCount['故障'], name: '故障', itemStyle: { color: '#dc3545' } }
            ]
          }
        ]
      }
      
      chart.setOption(option)
      
      // 响应式调整
      window.addEventListener('resize', () => {
        chart.resize()
      })
    }
  }
}
</script>