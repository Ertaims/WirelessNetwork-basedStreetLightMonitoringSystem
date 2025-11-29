import { deviceAPI } from '../../api'

const state = {
  devices: [],
  currentDevice: null,
  loading: false,
  error: null,
  pagination: {
    page: 1,
    limit: 20,
    total: 0
  }
}

const mutations = {
  SET_LOADING(state, loading) {
    state.loading = loading
  },
  SET_ERROR(state, error) {
    state.error = error
  },
  SET_DEVICES(state, devices) {
    state.devices = devices
  },
  SET_CURRENT_DEVICE(state, device) {
    state.currentDevice = device
  },
  UPDATE_DEVICE(state, updatedDevice) {
    const index = state.devices.findIndex(d => d.id === updatedDevice.id)
    if (index !== -1) {
      state.devices.splice(index, 1, updatedDevice)
    }
    if (state.currentDevice && state.currentDevice.id === updatedDevice.id) {
      state.currentDevice = updatedDevice
    }
  },
  ADD_DEVICE(state, device) {
    state.devices.unshift(device)
  },
  REMOVE_DEVICE(state, deviceId) {
    state.devices = state.devices.filter(d => d.id !== deviceId)
    if (state.currentDevice && state.currentDevice.id === deviceId) {
      state.currentDevice = null
    }
  },
  SET_PAGINATION(state, pagination) {
    state.pagination = { ...state.pagination, ...pagination }
  }
}

const actions = {
  async fetchDevices({ commit }, params = {}) {
    commit('SET_LOADING', true)
    commit('SET_ERROR', null)
    
    try {
      const response = await deviceAPI.getDevices(params)
      console.log('设备列表请求参数:', params)
      console.log('设备列表后端返回原始数据:', response)
      
      if (!response) {
        throw new Error('接口未返回任何响应')
      }
      
      let devices = []
      let pagination = {}
      
      // 关键修复1：直接使用 response（无需 .data），适配后端数组格式
      if (Array.isArray(response)) {
        devices = response.map(item => ({
          ...item,
          // 关键修复2：字段映射（groupName→group、uptimeHours→uptime）
          group: item.groupName || '未分组',
          uptime: item.uptimeHours || 0,
          // 关键修复3：大小写统一（转为小写，适配前端判断）
          status: item.status?.toLowerCase() || 'unknown',
          power: item.power?.toLowerCase() || 'off'
        }))
        pagination = { page: 1, limit: 20, total: devices.length }
      } else if (typeof response === 'object') {
        // 兼容后端外层对象格式（如果后续调整）
        const deviceList = response.devices || response.deviceList || response.list || response.items || []
        devices = deviceList.map(item => ({
          ...item,
          group: item.groupName || '未分组',
          uptime: item.uptimeHours || 0,
          status: item.status?.toLowerCase() || 'unknown',
          power: item.power?.toLowerCase() || 'off'
        }))
        pagination = response.pagination || response.pageInfo || response.page || {}
      }
      
      // 分页字段兜底
      pagination = {
        page: pagination.page || 1,
        limit: pagination.limit || 20,
        total: pagination.total || devices.length
      }
      
      commit('SET_DEVICES', devices)
      commit('SET_PAGINATION', pagination)
      console.log('设备列表加载成功，数据量:', devices.length)
      return devices
    } catch (error) {
      const errorMsg = error.response?.data?.message || error.message || '获取设备列表失败'
      commit('SET_ERROR', errorMsg)
      console.error('加载设备列表失败:', error)
      return []
    } finally {
      commit('SET_LOADING', false)
    }
  },
  
  async fetchDevice({ commit }, deviceId) {
    commit('SET_LOADING', true)
    commit('SET_ERROR', null)
    
    try {
      // 关键修复1：无需 .data
      const deviceDetail = await deviceAPI.getDevice(deviceId)
      console.log(`设备 ${deviceId} 详情:', deviceDetail`)
      
      if (!deviceDetail) {
        throw new Error('设备详情数据异常')
      }
      
      // 关键修复2：字段映射+大小写统一
      const formattedDevice = {
        ...deviceDetail,
        group: deviceDetail.groupName || '未分组',
        uptime: deviceDetail.uptimeHours || 0,
        status: deviceDetail.status?.toLowerCase() || 'unknown',
        power: deviceDetail.power?.toLowerCase() || 'off'
      }
      
      commit('SET_CURRENT_DEVICE', formattedDevice)
      return formattedDevice
    } catch (error) {
      const errorMsg = error.response?.data?.message || error.message || '获取设备详情失败'
      commit('SET_ERROR', errorMsg)
      console.error(`获取设备 ${deviceId} 详情失败:`, error)
      throw error
    } finally {
      commit('SET_LOADING', false)
    }
  },
  
  async updateDevice({ commit }, { id, data }) {
    commit('SET_LOADING', true)
    commit('SET_ERROR', null)
    
    try {
      // 关键修复1：无需 .data
      const updatedDevice = await deviceAPI.updateDevice(id, data)
      
      if (!updatedDevice) {
        throw new Error('设备更新结果异常')
      }
      
      // 关键修复2：格式化返回数据
      const formattedDevice = {
        ...updatedDevice,
        group: updatedDevice.groupName || '未分组',
        uptime: updatedDevice.uptimeHours || 0,
        status: updatedDevice.status?.toLowerCase() || 'unknown',
        power: updatedDevice.power?.toLowerCase() || 'off'
      }
      
      commit('UPDATE_DEVICE', formattedDevice)
      return formattedDevice
    } catch (error) {
      const errorMsg = error.response?.data?.message || error.message || '更新设备失败'
      commit('SET_ERROR', errorMsg)
      console.error(`更新设备 ${id} 失败:`, error)
      throw error
    } finally {
      commit('SET_LOADING', false)
    }
  },
  
  async controlDevice({ commit }, { id, command }) {
    commit('SET_LOADING', true)
    commit('SET_ERROR', null)
    
    try {
      // 关键修复1：无需 .data，适配后端返回格式
      const result = await deviceAPI.controlDevice(id, command)
      console.log(`控制设备 ${id} 结果:', result`)
      
      // 假设后端返回 { device: 更新后的设备数据 }（如果格式不同可调整）
      const updatedDevice = result.device || result
      if (!updatedDevice) {
        throw new Error('设备控制结果异常，未返回设备信息')
      }
      
      // 关键修复2：格式化数据
      const formattedDevice = {
        ...updatedDevice,
        group: updatedDevice.groupName || '未分组',
        uptime: updatedDevice.uptimeHours || 0,
        status: updatedDevice.status?.toLowerCase() || 'unknown',
        power: updatedDevice.power?.toLowerCase() || 'off'
      }
      
      commit('UPDATE_DEVICE', formattedDevice)
      return { device: formattedDevice }
    } catch (error) {
      const errorMsg = error.response?.data?.message || error.message || '控制设备失败'
      commit('SET_ERROR', errorMsg)
      console.error(`控制设备 ${id} 失败:`, error)
      throw error
    } finally {
      commit('SET_LOADING', false)
    }
  },
  
  async controlGroup({ dispatch }, { group, command }) {
    try {
      // 关键修复1：无需 .data
      await deviceAPI.controlGroup(group, command)
      await dispatch('fetchDevices') // 重新加载设备列表
      return true
    } catch (error) {
      console.error(`批量控制分组 ${group} 设备失败:`, error)
      throw error
    }
  },
  
  async createDevice({ commit }, deviceData) {
    commit('SET_LOADING', true)
    commit('SET_ERROR', null)
    
    try {
      // 关键修复1：无需 .data
      const newDevice = await deviceAPI.createDevice(deviceData)
      
      if (!newDevice) {
        throw new Error('创建设备结果异常')
      }
      
      const formattedDevice = {
        ...newDevice,
        group: newDevice.groupName || '未分组',
        uptime: newDevice.uptimeHours || 0,
        status: newDevice.status?.toLowerCase() || 'unknown',
        power: newDevice.power?.toLowerCase() || 'off'
      }
      
      commit('ADD_DEVICE', formattedDevice)
      return formattedDevice
    } catch (error) {
      const errorMsg = error.response?.data?.message || error.message || '创建设备失败'
      commit('SET_ERROR', errorMsg)
      console.error('创建设备失败:', error)
      throw error
    } finally {
      commit('SET_LOADING', false)
    }
  },
  
  async deleteDevice({ commit }, deviceId) {
    commit('SET_LOADING', true)
    commit('SET_ERROR', null)
    
    try {
      const response = await deviceAPI.deleteDevice(deviceId)
      
      if (response?.status !== 200 && response?.status !== 204) {
        throw new Error('删除设备接口响应异常')
      }
      
      commit('REMOVE_DEVICE', deviceId)
    } catch (error) {
      const errorMsg = error.response?.data?.message || error.message || '删除设备失败'
      commit('SET_ERROR', errorMsg)
      console.error(`删除设备 ${deviceId} 失败:`, error)
      throw error
    } finally {
      commit('SET_LOADING', false)
    }
  }
}

const getters = {
  devices: state => state.devices,
  currentDevice: state => state.currentDevice,
  loading: state => state.loading,
  error: state => state.error,
  onlineDevices: state => state.devices.filter(d => d.status === 'online'),
  offlineDevices: state => state.devices.filter(d => d.status === 'offline'),
  faultDevices: state => state.devices.filter(d => d.status === 'fault'),
  // 关键修复：按 group 分组（已在 action 中映射 groupName→group）
  deviceGroups: state => {
    const groups = {}
    state.devices.forEach(device => {
      const groupName = device.group || '未分组'
      if (!groups[groupName]) {
        groups[groupName] = []
      }
      groups[groupName].push(device)
    })
    return groups
  }
}

export default {
  namespaced: true,
  state,
  mutations,
  actions,
  getters
}