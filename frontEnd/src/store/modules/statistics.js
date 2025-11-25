import { statisticsAPI } from '../../api'

const state = {
  energyData: {},
  lightingRateData: {},
  deviceStats: {},
  alarmStatistics: {},
  loading: false,
  error: null,
  dateRange: {
    start: null,
    end: null
  }
}

const mutations = {
  SET_LOADING(state, loading) {
    state.loading = loading
  },
  SET_ERROR(state, error) {
    state.error = error
  },
  SET_ENERGY_DATA(state, data) {
    state.energyData = data
  },
  SET_LIGHTING_RATE_DATA(state, data) {
    state.lightingRateData = data
  },
  SET_DEVICE_STATS(state, data) {
    state.deviceStats = data
  },
  SET_ALARM_STATISTICS(state, data) {
    state.alarmStatistics = data
  },
  SET_DATE_RANGE(state, range) {
    state.dateRange = { ...state.dateRange, ...range }
  }
}

const actions = {
  async fetchEnergyConsumption({ commit, state }, params = {}) {
    commit('SET_LOADING', true)
    commit('SET_ERROR', null)
    
    try {
      const queryParams = {
        // 补充后端常见必传参数（根据接口文档调整，示例：统计粒度）
        type: params.type || 'daily', // 可选：daily/weekly/monthly
        ...params
      }
      // 条件添加日期（格式兼容）
      if (state.dateRange.start) {
        queryParams.startDate = new Date(state.dateRange.start).toISOString().split('T')[0]
      }
      if (state.dateRange.end) {
        queryParams.endDate = new Date(state.dateRange.end).toISOString().split('T')[0]
      }
      // 过滤空参数
      Object.keys(queryParams).forEach(key => !queryParams[key] && delete queryParams[key])

      console.log('📊 能耗数据请求 - URL:', statisticsAPI.getEnergyConsumption.toString())
      console.log('📊 能耗数据请求参数:', queryParams)
      const response = await statisticsAPI.getEnergyConsumption(queryParams)

      if (!response || !response) throw new Error('能耗接口返回异常')

      commit('SET_ENERGY_DATA', response)
      console.log('✅ 能耗数据加载成功')
      return response
    } catch (error) {
      const errorMsg = `获取能耗数据失败: ${error.response?.status || error.message}`
      commit('SET_ERROR', errorMsg)
      console.error('❌ 能耗数据加载失败-详情:', {
        status: error.response?.status,
        responseData: error.response?.data,
        message: error.message
      })
      return {}
    } finally {
      commit('SET_LOADING', false)
    }
  },
  
  async fetchLightingRate({ commit, state }, params = {}) {
    commit('SET_LOADING', true)
    commit('SET_ERROR', null)
    
    try {
      const queryParams = {
        type: params.type || 'daily', // 补充常见必传参数
        ...params
      }
      if (state.dateRange.start) queryParams.startDate = new Date(state.dateRange.start).toISOString().split('T')[0]
      if (state.dateRange.end) queryParams.endDate = new Date(state.dateRange.end).toISOString().split('T')[0]
      Object.keys(queryParams).forEach(key => !queryParams[key] && delete queryParams[key])

      console.log('📊 亮灯率数据请求 - URL:', statisticsAPI.getLightingRate.toString())
      console.log('📊 亮灯率数据请求参数:', queryParams)
      const response = await statisticsAPI.getLightingRate(queryParams)
      
      if (!response) throw new Error('亮灯率接口返回异常')

      commit('SET_LIGHTING_RATE_DATA', response)
      console.log('✅ 亮灯率数据加载成功')
      return response
    } catch (error) {
      const errorMsg = `获取亮灯率数据失败: ${error.response?.status || error.message}`
      commit('SET_ERROR', errorMsg)
      console.error('❌ 亮灯率数据加载失败-详情:', {
        status: error.response?.status,
        responseData: error.response?.data,
        message: error.message
      })
      return {}
    } finally {
      commit('SET_LOADING', false)
    }
  },
  
  async fetchDeviceStats({ commit }, params = {}) {
    commit('SET_LOADING', true)
    commit('SET_ERROR', null)
    
    try {
      console.log('📊 设备统计请求 - URL:', statisticsAPI.getDeviceStats.toString())
      console.log('📊 设备统计请求参数:', params)
      const response = await statisticsAPI.getDeviceStats(params) // 允许传递参数

      if (!response || !response) throw new Error('设备统计接口返回异常')

      commit('SET_DEVICE_STATS', response)
      console.log('✅ 设备统计加载成功')
      return response
    } catch (error) {
      const errorMsg = `获取设备统计失败: ${error.response?.status || error.message}`
      commit('SET_ERROR', errorMsg)
      console.error('❌ 设备统计加载失败-详情:', {
        status: error.response?.status,
        responseData: error.response?.data,
        message: error.message
      })
      return {}
    } finally {
      commit('SET_LOADING', false)
    }
  },
  
  async fetchAlarmStatistics({ commit, state }, params = {}) {
    commit('SET_LOADING', true)
    commit('SET_ERROR', null)
    
    try {
      const queryParams = {
        // type: params.type || 'daily', // 补充常见必传参数
        ...params
      }
      if (state.dateRange.start) queryParams.startDate = new Date(state.dateRange.start).toISOString().split('T')[0]
      if (state.dateRange.end) queryParams.endDate = new Date(state.dateRange.end).toISOString().split('T')[0]
      Object.keys(queryParams).forEach(key => !queryParams[key] && delete queryParams[key])

      // console.log('📊 告警统计请求 - URL:', statisticsAPI.getAlarmStatistics.toString())
      console.log('📊 告警统计请求参数:', queryParams)
      const response = await statisticsAPI.getAlarmStatistics(queryParams)

      if (!response || !response) throw new Error('告警统计接口返回异常')

      commit('SET_ALARM_STATISTICS', response)
      console.log('✅ 告警统计加载成功')
      return response
    } catch (error) {
      const errorMsg = `获取告警统计失败: ${error.response?.status || error.message}`
      commit('SET_ERROR', errorMsg)
      console.error('❌ 告警统计加载失败-详情:', {
        status: error.response?.status,
        responseData: error.response?.data,
        message: error.message
      })
      return {}
    } finally {
      commit('SET_LOADING', false)
    }
  },
  
  async fetchAllStatistics({ commit, dispatch }) {
    commit('SET_LOADING', true)
    try {
      console.log('📊 开始加载所有统计数据...')
      // 并行加载，精准捕获每个接口错误
      const results = await Promise.allSettled([
        dispatch('fetchEnergyConsumption'),
        dispatch('fetchLightingRate'),
        dispatch('fetchDeviceStats'),
        dispatch('fetchAlarmStatistics')
      ])

      // 统计成功/失败情况
      const successCount = results.filter(res => res.status === 'fulfilled').length
      const failedItems = results
        .map((res, idx) => ({ idx, res }))
        .filter(({ res }) => res.status === 'rejected')
        .map(({ idx }) => ['能耗', '亮灯率', '设备统计', '告警统计'][idx])

      console.log(`📊 统计数据加载完成：成功${successCount}/4项，失败项：${failedItems.join(', ') || '无'}`)
      
      if (failedItems.length > 0) {
        commit('SET_ERROR', `部分统计数据加载失败：${failedItems.join(', ')}`)
      }
      return results
    } catch (error) {
      console.error('❌ 统计数据加载流程异常:', error)
      commit('SET_ERROR', '统计数据加载流程发生未知错误')
    } finally {
      commit('SET_LOADING', false)
    }
  },
  
  updateDateRange({ commit, dispatch }, range) {
    if (range.start) range.start = new Date(range.start)
    if (range.end) range.end = new Date(range.end)
    commit('SET_DATE_RANGE', range)
    return dispatch('fetchAllStatistics')
  }
}

const getters = {
  energyData: state => state.energyData,
  lightingRateData: state => state.lightingRateData,
  deviceStats: state => state.deviceStats,
  alarmStatistics: state => state.alarmStatistics,
  loading: state => state.loading,
  error: state => state.error,
  dateRange: state => state.dateRange
}

export default {
  namespaced: true,
  state,
  mutations,
  actions,
  getters
}