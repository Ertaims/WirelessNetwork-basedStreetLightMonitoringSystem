import { alarmAPI } from '../../api'

const state = {
  alarms: [],
  currentAlarm: null,
  loading: false,
  error: null,
  filters: {
    status: 'all',
    level: 'all',
    startDate: null,
    endDate: null
  },
  pagination: {
    page: 1,
    limit: 20,
    total: 0
  },
  stats: {
    pending: 0,
    confirmed: 0,
    resolved: 0,
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
  SET_ALARMS(state, alarms) {
    state.alarms = alarms
  },
  SET_CURRENT_ALARM(state, alarm) {
    state.currentAlarm = alarm
  },
  UPDATE_ALARM(state, updatedAlarm) {
    const index = state.alarms.findIndex(a => a.id === updatedAlarm.id)
    if (index !== -1) {
      state.alarms.splice(index, 1, updatedAlarm)
    }
    if (state.currentAlarm && state.currentAlarm.id === updatedAlarm.id) {
      state.currentAlarm = updatedAlarm
    }
  },
  ADD_ALARM(state, alarm) {
    state.alarms.unshift(alarm)
  },
  REMOVE_ALARM(state, alarmId) {
    state.alarms = state.alarms.filter(a => a.id !== alarmId)
    if (state.currentAlarm && state.currentAlarm.id === alarmId) {
      state.currentAlarm = null
    }
  },
  SET_FILTERS(state, filters) {
    state.filters = { ...state.filters, ...filters }
  },
  SET_PAGINATION(state, pagination) {
    state.pagination = { ...state.pagination, ...pagination }
  },
  SET_STATS(state, stats) {
    state.stats = { ...state.stats, ...stats }
  }
}

const actions = {
  async fetchAlarms({ commit, state }, params = {}) {
    commit('SET_LOADING', true)
    commit('SET_ERROR', null)
    
    try {
      const queryParams = { ...state.filters, ...params }
      
      // 过滤无效参数（避免传递 status=all 等后端不支持的值）
      if (queryParams.status === 'all') delete queryParams.status
      if (queryParams.level === 'all') delete queryParams.level
      if (!queryParams.startDate) delete queryParams.startDate
      if (!queryParams.endDate) delete queryParams.endDate
      
      // 核心：直接使用接口返回结果（已是后端 response.data）
      const data = await alarmAPI.getAlarms(queryParams)
      
      // 打印日志验证数据结构
      console.log('告警列表请求参数:', queryParams)
      console.log('告警列表后端返回原始数据:', data)
      
      if (!data) {
        throw new Error('告警接口未返回任何响应')
      }
      
      let alarms = []
      let pagination = {}
      
      // 关键修复1：从后端 content 字段提取告警数组（而非 alarms）
      if (Array.isArray(data.content)) {
        alarms = data.content.map(item => ({
          ...item, // 保留后端所有字段
          // 关键修复2：level 转为小写（适配前端判断逻辑）
          level: item.level?.toLowerCase() || 'unknown',
          // 关键修复3：status 转为小写（适配前端判断逻辑）
          status: item.status?.toLowerCase() || 'unknown',
          // 关键修复4：映射后端 occurredAt 到前端 timestamp（时间字段统一）
          timestamp: item.occurredAt
        }))
      }
      
      // 关键修复5：适配后端分页字段结构
      pagination = {
        page: data.pageable?.pageNumber + 1 || 1, // 后端 pageNumber 从 0 开始，前端+1 转为 1 起始
        limit: data.pageable?.pageSize || 20,
        total: data.totalElements || alarms.length // 后端总条数字段是 totalElements
      }
      
      commit('SET_ALARMS', alarms)
      commit('SET_PAGINATION', pagination)
      console.log('告警列表加载成功，数据量:', alarms.length)
      return alarms
    } catch (error) {
      const errorMsg = error.response?.data?.message || error.message || '获取告警列表失败'
      commit('SET_ERROR', errorMsg)
      console.error('加载告警列表失败-完整错误:', error)
      return []
    } finally {
      commit('SET_LOADING', false)
    }
  },
  
  async fetchAlarmStats({ commit }) {
    try {
      // 修复：直接使用接口返回结果，无需 .data
      const statsData = await alarmAPI.getAlarmStats()
      commit('SET_STATS', statsData)
      console.log('告警统计加载成功:', statsData)
      return statsData
    } catch (error) {
      commit('SET_ERROR', error.response?.data?.message || '获取告警统计失败')
      console.error('加载告警统计失败:', error)
      throw error
    }
  },
  
  async confirmAlarm({ commit }, { id, remark = '' }) {
    commit('SET_LOADING', true)
    commit('SET_ERROR', null)
    
    try {
      // 修复：直接使用接口返回结果，无需 .data
      const updatedAlarm = await alarmAPI.confirmAlarm(id, { remark })
      // 同步转换返回数据的大小写，保持前端一致性
      const formattedAlarm = {
        ...updatedAlarm,
        level: updatedAlarm.level?.toLowerCase() || 'unknown',
        status: updatedAlarm.status?.toLowerCase() || 'unknown',
        timestamp: updatedAlarm.occurredAt
      }
      commit('UPDATE_ALARM', formattedAlarm)
      console.log('确认告警成功，更新后数据:', formattedAlarm)
      return formattedAlarm
    } catch (error) {
      commit('SET_ERROR', error.response?.data?.message || '确认告警失败')
      console.error('确认告警失败:', error)
      throw error
    } finally {
      commit('SET_LOADING', false)
    }
  },
  
  async resolveAlarm({ commit }, { id, remark = '' }) {
    commit('SET_LOADING', true)
    commit('SET_ERROR', null)
    
    try {
      // 修复：直接使用接口返回结果，无需 .data
      const updatedAlarm = await alarmAPI.resolveAlarm(id, { remark })
      // 同步转换返回数据的大小写，保持前端一致性
      const formattedAlarm = {
        ...updatedAlarm,
        level: updatedAlarm.level?.toLowerCase() || 'unknown',
        status: updatedAlarm.status?.toLowerCase() || 'unknown',
        timestamp: updatedAlarm.occurredAt
      }
      commit('UPDATE_ALARM', formattedAlarm)
      console.log('解决告警成功，更新后数据:', formattedAlarm)
      return formattedAlarm
    } catch (error) {
      commit('SET_ERROR', error.response?.data?.message || '解决告警失败')
      console.error('解决告警失败:', error)
      throw error
    } finally {
      commit('SET_LOADING', false)
    }
  },
  
  async batchProcessAlarms({ commit, dispatch }, { alarmIds, action }) {
    commit('SET_LOADING', true)
    commit('SET_ERROR', null)
    
    try {
      await alarmAPI.batchProcessAlarms(alarmIds, action)
      console.log(`批量${action}告警成功，重新加载列表`)
      await dispatch('fetchAlarms')
    } catch (error) {
      commit('SET_ERROR', error.response?.data?.message || '批量处理告警失败')
      console.error('批量处理告警失败:', error)
      throw error
    } finally {
      commit('SET_LOADING', false)
    }
  },
  
  updateFilters({ commit, dispatch }, filters) {
    commit('SET_FILTERS', filters)
    return dispatch('fetchAlarms', { page: 1 })
  }
}

const getters = {
  alarms: state => state.alarms,
  currentAlarm: state => state.currentAlarm,
  loading: state => state.loading,
  error: state => state.error,
  filters: state => state.filters,
  stats: state => state.stats,
  pendingAlarms: state => state.alarms.filter(a => a.status === 'pending'),
  pendingAlarmsCount: (state, getters) => getters.pendingAlarms.length,
  confirmedAlarms: state => state.alarms.filter(a => a.status === 'confirmed'),
  confirmedAlarmsCount: (state, getters) => getters.confirmedAlarms.length,
  resolvedAlarms: state => state.alarms.filter(a => a.status === 'resolved'),
  resolvedAlarmsCount: (state, getters) => getters.resolvedAlarms.length,
  highPriorityAlarms: state => state.alarms.filter(a => a.level === 'high'),
  highPriorityAlarmsCount: (state, getters) => getters.highPriorityAlarms.length,
  filteredAlarms: state => {
    let filtered = state.alarms
    
    if (state.filters.status !== 'all') {
      filtered = filtered.filter(a => a.status === state.filters.status)
    }
    
    if (state.filters.level !== 'all') {
      filtered = filtered.filter(a => a.level === state.filters.level)
    }
    
    if (state.filters.startDate) {
      filtered = filtered.filter(a => new Date(a.timestamp) >= new Date(state.filters.startDate))
    }
    
    if (state.filters.endDate) {
      filtered = filtered.filter(a => new Date(a.timestamp) <= new Date(state.filters.endDate))
    }
    
    return filtered
  },
  filteredAlarmsCount: (state, getters) => getters.filteredAlarms.length
}

export default {
  namespaced: true,
  state,
  mutations,
  actions,
  getters
}