<template>
  <div class="statistics">
    <div class="d-flex justify-content-between align-items-center mb-4">
      <h1>数据统计与分析</h1>
      <div class="d-flex align-items-center">
        <div class="input-group me-3" style="width: 300px;">
          <span class="input-group-text">日期范围</span>
          <input type="date" class="form-control" v-model="dateRange.start">
          <span class="input-group-text">至</span>
          <input type="date" class="form-control" v-model="dateRange.end">
          <button class="btn btn-outline-primary" @click="applyDateRange">
            <i class="fas fa-filter"></i>
          </button>
        </div>
        <button class="btn btn-primary" @click="refreshData">
          <i class="fas fa-sync-alt" :class="{ 'fa-spin': refreshing }"></i>
          刷新
        </button>
      </div>
    </div>
    
    <LoadingSpinner v-if="loading && !hasData" />
    <ErrorMessage v-else-if="error" :message="error" @retry="fetchAllData" />
    
    <div v-else>
      <!-- 统计卡片 -->
      <div class="row mb-4">
        <div class="col-md-3">
          <div class="card text-white bg-primary">
            <div class="card-body">
              <div class="d-flex justify-content-between">
                <div>
                  <h5 class="card-title">总能耗</h5>
                  <p class="card-text display-6">{{ formatNumber(energyData.total || 0) }} kWh</p>
                </div>
                <i class="fas fa-bolt fa-2x opacity-50"></i>
              </div>
              <small class="opacity-75">较上月 {{ energyData.changeRate >= 0 ? '+' : '' }}{{ (energyData.changeRate * 100).toFixed(1) }}%</small>
            </div>
          </div>
        </div>
        <div class="col-md-3">
          <div class="card text-white bg-success">
            <div class="card-body">
              <div class="d-flex justify-content-between">
                <div>
                  <h5 class="card-title">平均亮灯率</h5>
                  <p class="card-text display-6">{{ (lightingRateData.average * 100).toFixed(1) }}%</p>
                </div>
                <i class="fas fa-lightbulb fa-2x opacity-50"></i>
              </div>
              <small class="opacity-75">{{ lightingRateData.days || 0 }} 天数据</small>
            </div>
          </div>
        </div>
        <div class="col-md-3">
          <div class="card text-white bg-warning">
            <div class="card-body">
              <div class="d-flex justify-content-between">
                <div>
                  <h5 class="card-title">设备在线率</h5>
                  <p class="card-text display-6">{{ (deviceStats.onlineRate * 100).toFixed(1) }}%</p>
                </div>
                <i class="fas fa-wifi fa-2x opacity-50"></i>
              </div>
              <small class="opacity-75">{{ deviceStats.onlineCount || 0 }}/{{ deviceStats.totalCount || 0 }} 台设备</small>
            </div>
          </div>
        </div>
        <div class="col-md-3">
          <div class="card text-white bg-danger">
            <div class="card-body">
              <div class="d-flex justify-content-between">
                <div>
                  <h5 class="card-title">告警数量</h5>
                  <p class="card-text display-6">{{ alarmStatistics.total || 0 }}</p>
                </div>
                <i class="fas fa-exclamation-triangle fa-2x opacity-50"></i>
              </div>
              <small class="opacity-75">{{ alarmStatistics.pending || 0 }} 个待处理</small>
            </div>
          </div>
        </div>
      </div>
      
      <!-- 图表区域 -->
      <div class="row mb-4">
        <div class="col-md-8">
          <div class="card">
            <div class="card-header d-flex justify-content-between align-items-center">
              <h5 class="card-title mb-0">能耗趋势分析</h5>
              <div class="btn-group btn-group-sm">
                <button 
                  v-for="period in timePeriods" 
                  :key="period.value"
                  class="btn btn-outline-secondary"
                  :class="{ active: selectedPeriod === period.value }"
                  @click="changeTimePeriod(period.value)"
                >
                  {{ period.label }}
                </button>
              </div>
            </div>
            <div class="card-body">
              <div ref="energyChart" style="height: 400px;"></div>
            </div>
          </div>
        </div>
        <div class="col-md-4">
          <div class="card">
            <div class="card-header">
              <h5 class="card-title mb-0">能耗分布</h5>
            </div>
            <div class="card-body">
              <div ref="energyDistributionChart" style="height: 400px;"></div>
            </div>
          </div>
        </div>
      </div>
      
      <div class="row">
        <div class="col-md-6">
          <div class="card">
            <div class="card-header">
              <h5 class="card-title mb-0">亮灯率统计</h5>
            </div>
            <div class="card-body">
              <div ref="lightingRateChart" style="height: 300px;"></div>
            </div>
          </div>
        </div>
        <div class="col-md-6">
          <div class="card">
            <div class="card-header">
              <h5 class="card-title mb-0">设备运行状态分布</h5>
            </div>
            <div class="card-body">
              <div ref="statusChart" style="height: 300px;"></div>
            </div>
          </div>
        </div>
      </div>
      
      <div class="row mt-4">
        <div class="col-md-12">
          <div class="card">
            <div class="card-header">
              <h5 class="card-title mb-0">告警统计</h5>
            </div>
            <div class="card-body">
              <div ref="alarmChart" style="height: 300px;"></div>
            </div>
          </div>
        </div>
      </div>
      
      <!-- 详细数据表格 -->
      <div class="row mt-4">
        <div class="col-md-12">
          <div class="card">
            <div class="card-header d-flex justify-content-between align-items-center">
              <h5 class="card-title mb-0">详细数据报表</h5>
              <button class="btn btn-sm btn-outline-primary" @click="exportData">
                <i class="fas fa-download me-1"></i>导出数据
              </button>
            </div>
            <div class="card-body">
              <table class="table table-striped table-hover">
                <thead>
                  <tr>
                    <th>日期</th>
                    <th>总能耗 (kWh)</th>
                    <th>平均亮灯率</th>
                    <th>设备在线率</th>
                    <th>告警数量</th>
                    <th>平均亮度</th>
                    <th>运行时长</th>
                  </tr>
                </thead>
                <tbody>
                  <tr v-for="day in dailyData" :key="day.date">
                    <td>{{ day.date }}</td>
                    <td>{{ day.energyConsumption.toFixed(2) }}</td>
                    <td>
                      <div class="d-flex align-items-center">
                        <div class="progress flex-grow-1 me-2" style="height: 6px;">
                          <div class="progress-bar" :style="{ width: (day.lightingRate * 100) + '%' }"></div>
                        </div>
                        <span>{{ (day.lightingRate * 100).toFixed(1) }}%</span>
                      </div>
                    </td>
                    <td>{{ (day.onlineRate * 100).toFixed(1) }}%</td>
                    <td>
                      <span class="badge" :class="day.alarms > 0 ? 'bg-danger' : 'bg-success'">
                        {{ day.alarms }}
                      </span>
                    </td>
                    <td>{{ day.averageBrightness }}%</td>
                    <td>{{ day.uptime }}h</td>
                  </tr>
                </tbody>
              </table>
              
              <!-- 分页 -->
              <nav v-if="pagination.total > pagination.pageSize">
                <ul class="pagination justify-content-center">
                  <li class="page-item" :class="{ disabled: pagination.currentPage === 1 }">
                    <button class="page-link" @click="changePage(pagination.currentPage - 1)">上一页</button>
                  </li>
                  <li v-for="page in pagination.pages" :key="page" 
                      class="page-item" :class="{ active: pagination.currentPage === page }">
                    <button class="page-link" @click="changePage(page)">{{ page }}</button>
                  </li>
                  <li class="page-item" :class="{ disabled: pagination.currentPage === pagination.totalPages }">
                    <button class="page-link" @click="changePage(pagination.currentPage + 1)">下一页</button>
                  </li>
                </ul>
              </nav>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { mapState, mapActions, mapGetters } from 'vuex'
import * as echarts from 'echarts'
import LoadingSpinner from './common/LoadingSpinner.vue'
import ErrorMessage from './common/ErrorMessage.vue'

export default {
  name: 'Statistics',
  components: {
    LoadingSpinner,
    ErrorMessage
  },
  data() {
    return {
      selectedPeriod: '7d',
      refreshing: false,
      timePeriods: [
        { label: '7天', value: '7d' },
        { label: '30天', value: '30d' },
        { label: '90天', value: '90d' },
        { label: '1年', value: '1y' }
      ],
      charts: {},
      pagination: {
        currentPage: 1,
        pageSize: 10,
        total: 0,
        totalPages: 0,
        pages: []
      }
    }
  },
  computed: {
    ...mapState('statistics', [
      'energyData',
      'lightingRateData', 
      'deviceStats',
      'alarmStatistics',
      'loading',
      'error',
      'dateRange'
    ]),
    ...mapState('devices', ['devices']),
    
    hasData() {
      return this.energyData.daily && this.energyData.daily.length > 0
    },
    
    dailyData() {
      // 模拟每日数据 - 实际应该从 API 获取
      const data = []
      for (let i = 0; i < 10; i++) {
        const date = new Date()
        date.setDate(date.getDate() - i)
        data.push({
          date: date.toISOString().split('T')[0],
          energyConsumption: Math.random() * 100 + 50,
          lightingRate: Math.random() * 0.2 + 0.8,
          onlineRate: Math.random() * 0.1 + 0.9,
          alarms: Math.floor(Math.random() * 5),
          averageBrightness: Math.floor(Math.random() * 40 + 60),
          uptime: Math.floor(Math.random() * 10 + 18)
        })
      }
      return data
    }
  },
  async mounted() {
    await this.fetchAllData()
    this.$nextTick(() => {
      this.initCharts()
    })
  },
  beforeUnmount() {
    // 销毁所有图表实例
    Object.values(this.charts).forEach(chart => {
      if (chart) {
        chart.dispose()
      }
    })
  },
  methods: {
    ...mapActions('statistics', [
      'fetchEnergyConsumption',
      'fetchLightingRate',
      'fetchDeviceStats',
      'fetchAlarmStatistics',
      'fetchAllStatistics',
      'updateDateRange'
    ]),
    
    async fetchAllData() {
      try {
        await this.fetchAllStatistics()
      } catch (error) {
        console.error('获取统计数据失败:', error)
      }
    },
    
    async refreshData() {
      this.refreshing = true
      try {
        await this.fetchAllData()
        this.updateCharts()
      } catch (error) {
        console.error('刷新数据失败:', error)
      } finally {
        this.refreshing = false
      }
    },
    
    async applyDateRange() {
      await this.updateDateRange(this.dateRange)
      this.updateCharts()
    },
    
    changeTimePeriod(period) {
      this.selectedPeriod = period
      this.updateCharts()
    },
    
    initCharts() {
      this.initEnergyChart()
      this.initEnergyDistributionChart()
      this.initLightingRateChart()
      this.initStatusChart()
      this.initAlarmChart()
      
      // 响应式调整
      window.addEventListener('resize', this.handleResize)
    },
    
    handleResize() {
      Object.values(this.charts).forEach(chart => {
        if (chart) {
          chart.resize()
        }
      })
    },
    
    updateCharts() {
      Object.values(this.charts).forEach(chart => {
        if (chart) {
          chart.dispose()
        }
      })
      this.$nextTick(() => {
        this.initCharts()
      })
    },
    
    initEnergyChart() {
      const chart = echarts.init(this.$refs.energyChart)
      this.charts.energy = chart
      
      // 模拟能耗数据
      const dates = []
      const energyData = []
      const today = new Date()
      
      for (let i = 6; i >= 0; i--) {
        const date = new Date(today)
        date.setDate(today.getDate() - i)
        dates.push(`${date.getMonth() + 1}/${date.getDate()}`)
        energyData.push(Math.floor(Math.random() * 50) + 80)
      }
      
      const option = {
        tooltip: {
          trigger: 'axis',
          formatter: '{b}<br/>{a}: {c} kWh'
        },
        legend: {
          data: ['总能耗']
        },
        grid: {
          left: '3%',
          right: '4%',
          bottom: '3%',
          containLabel: true
        },
        xAxis: {
          type: 'category',
          boundaryGap: false,
          data: dates
        },
        yAxis: {
          type: 'value',
          name: '千瓦时 (kWh)'
        },
        series: [
          {
            name: '总能耗',
            type: 'line',
            stack: '总量',
            data: energyData,
            smooth: true,
            lineStyle: {
              width: 3
            },
            areaStyle: {
              color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
                { offset: 0, color: 'rgba(58,77,233,0.8)' },
                { offset: 1, color: 'rgba(58,77,233,0.1)' }
              ])
            },
            markPoint: {
              data: [
                { type: 'max', name: '最大值' },
                { type: 'min', name: '最小值' }
              ]
            }
          }
        ]
      }
      
      chart.setOption(option)
    },
    
    initEnergyDistributionChart() {
      const chart = echarts.init(this.$refs.energyDistributionChart)
      this.charts.energyDistribution = chart
      
      const option = {
        tooltip: {
          trigger: 'item',
          formatter: '{a} <br/>{b}: {c} kWh ({d}%)'
        },
        legend: {
          orient: 'vertical',
          left: 'left'
        },
        series: [
          {
            name: '能耗分布',
            type: 'pie',
            radius: ['40%', '70%'],
            avoidLabelOverlap: false,
            itemStyle: {
              borderRadius: 10,
              borderColor: '#fff',
              borderWidth: 2
            },
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
              { value: 35, name: 'A区' },
              { value: 25, name: 'B区' },
              { value: 20, name: 'C区' },
              { value: 15, name: 'D区' },
              { value: 5, name: '其他' }
            ]
          }
        ]
      }
      
      chart.setOption(option)
    },
    
    initLightingRateChart() {
      const chart = echarts.init(this.$refs.lightingRateChart)
      this.charts.lightingRate = chart
      
      const dates = []
      const rates = []
      const today = new Date()
      
      for (let i = 6; i >= 0; i--) {
        const date = new Date(today)
        date.setDate(today.getDate() - i)
        dates.push(`${date.getMonth() + 1}/${date.getDate()}`)
        rates.push((Math.random() * 0.15 + 0.85) * 100) // 85%-100%
      }
      
      const option = {
        tooltip: {
          trigger: 'axis',
          formatter: '{b}: {c}%'
        },
        xAxis: {
          type: 'category',
          data: dates
        },
        yAxis: {
          type: 'value',
          min: 0,
          max: 100,
          axisLabel: {
            formatter: '{value}%'
          }
        },
        series: [
          {
            data: rates,
            type: 'bar',
            itemStyle: {
              color: function(params) {
                const value = params.data
                if (value >= 95) return '#28a745'
                if (value >= 90) return '#ffc107'
                return '#dc3545'
              }
            },
            label: {
              show: true,
              position: 'top',
              formatter: '{c}%'
            }
          }
        ]
      }
      
      chart.setOption(option)
    },
    
    initStatusChart() {
      const chart = echarts.init(this.$refs.statusChart)
      this.charts.status = chart
      
      const statusCount = {
        '在线-开启': this.devices.filter(d => d.status === 'online' && d.power === 'on').length,
        '在线-关闭': this.devices.filter(d => d.status === 'online' && d.power === 'off').length,
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
          data: ['在线-开启', '在线-关闭', '离线', '故障']
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
              { value: statusCount['在线-开启'], name: '在线-开启', itemStyle: { color: '#28a745' } },
              { value: statusCount['在线-关闭'], name: '在线-关闭', itemStyle: { color: '#6c757d' } },
              { value: statusCount['离线'], name: '离线', itemStyle: { color: '#343a40' } },
              { value: statusCount['故障'], name: '故障', itemStyle: { color: '#dc3545' } }
            ]
          }
        ]
      }
      
      chart.setOption(option)
    },
    
    initAlarmChart() {
      const chart = echarts.init(this.$refs.alarmChart)
      this.charts.alarm = chart
      
      const alarmTypes = ['电压异常', '电流异常', '设备离线', '设备故障', '亮度异常']
      const alarmCounts = alarmTypes.map(() => Math.floor(Math.random() * 10) + 1)
      
      const option = {
        tooltip: {
          trigger: 'axis',
          axisPointer: {
            type: 'shadow'
          }
        },
        grid: {
          left: '3%',
          right: '4%',
          bottom: '3%',
          containLabel: true
        },
        xAxis: {
          type: 'value'
        },
        yAxis: {
          type: 'category',
          data: alarmTypes
        },
        series: [
          {
            name: '告警数量',
            type: 'bar',
            data: alarmCounts,
            itemStyle: {
              color: function(params) {
                const colorList = ['#c23531','#2f4554','#61a0a8','#d48265','#91c7ae']
                return colorList[params.dataIndex % colorList.length]
              }
            },
            label: {
              show: true,
              position: 'right'
            }
          }
        ]
      }
      
      chart.setOption(option)
    },
    
    formatNumber(num) {
      return new Intl.NumberFormat().format(num)
    },
    
    changePage(page) {
      if (page >= 1 && page <= this.pagination.totalPages) {
        this.pagination.currentPage = page
        // 这里应该调用 API 获取对应页面的数据
      }
    },
    
    exportData() {
      // 模拟数据导出
      const dataStr = JSON.stringify(this.dailyData, null, 2)
      const dataBlob = new Blob([dataStr], { type: 'application/json' })
      const url = URL.createObjectURL(dataBlob)
      const link = document.createElement('a')
      link.href = url
      link.download = `statistics-${new Date().toISOString().split('T')[0]}.json`
      link.click()
      URL.revokeObjectURL(url)
    }
  }
}
</script>

<style scoped>
.card {
  box-shadow: 0 0.125rem 0.25rem rgba(0, 0, 0, 0.075);
  border: 1px solid rgba(0, 0, 0, 0.125);
  margin-bottom: 1.5rem;
}

.card:hover {
  box-shadow: 0 0.5rem 1rem rgba(0, 0, 0, 0.15);
}

.progress {
  background-color: #e9ecef;
}

.table th {
  border-top: none;
  font-weight: 600;
  background-color: #f8f9fa;
}

.pagination {
  margin-bottom: 0;
}
</style>