import { createStore } from 'vuex'
import auth from './modules/auth'
import devices from './modules/devices'
import alarms from './modules/alarms'
import statistics from './modules/statistics'

export default createStore({
  modules: {
    auth,
    devices,
    alarms,
    statistics
  }
})