import request from '../utils/request'

// 认证相关 API
export const authAPI = {
  login: (credentials) => request.post('/auth/login', credentials),
  logout: () => request.post('/auth/logout'),
  refreshToken: () => request.post('/auth/refresh'),
  getProfile: () => request.get('/auth/profile')
}

// 设备相关 API
export const deviceAPI = {
  // 获取所有设备
  getDevices: (params) => request.get('/devices', { params }),
  // 获取单个设备详情
  getDevice: (id) => request.get(`/devices/${id}`),
  // 创建设备
  createDevice: (data) => request.post('/devices', data),
  // 更新设备
  updateDevice: (id, data) => request.put(`/devices/${id}`, data),
  // 删除设备
  deleteDevice: (id) => request.delete(`/devices/${id}`),
  // 控制设备
  controlDevice: (id, command) => request.post(`/devices/${id}/control`, command),
  // 分组控制
  controlGroup: (group, command) => request.post('/devices/group-control', { group, ...command }),
  // 批量控制
  batchControl: (deviceIds, command) => request.post('/devices/batch-control', { deviceIds, ...command })
}

// 告警相关 API
export const alarmAPI = {
  // 获取告警列表
  getAlarms: (params) => request.get('/alarms', { params }),
  // 获取告警统计
  getAlarmStats: () => request.get('/alarms/stats'),
  // 确认告警
  confirmAlarm: (id, data) => request.put(`/alarms/${id}/confirm`, data),
  // 解决告警
  resolveAlarm: (id, data) => request.put(`/alarms/${id}/resolve`, data),
  // 批量处理告警
  batchProcessAlarms: (alarmIds, action) => request.post('/alarms/batch-process', { alarmIds, action })
}

// 统计相关 API
export const statisticsAPI = {
  // 能耗统计
  getEnergyConsumption: (params) => request.get('/statistics/energy', { params }),
  // 亮灯率统计
  getLightingRate: (params) => request.get('/statistics/lighting-rate', { params }),
  // 设备运行统计
  getDeviceStats: (params) => request.get('/statistics/device-stats', { params }),
  // 告警统计
  getAlarmStatistics: (params) => request.get('/statistics/alarms', { params })
}

// 系统相关 API
export const systemAPI = {
  // 获取系统配置
  getConfig: () => request.get('/system/config'),
  // 更新系统配置
  updateConfig: (data) => request.put('/system/config', data)
}