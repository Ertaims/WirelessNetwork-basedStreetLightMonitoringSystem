// 设备状态常量
export const DEVICE_STATUS = {
  ONLINE: 'online',
  OFFLINE: 'offline',
  FAULT: 'fault'
}

// 设备电源状态
export const DEVICE_POWER = {
  ON: 'on',
  OFF: 'off'
}

// 告警级别
export const ALARM_LEVEL = {
  HIGH: 'high',
  MEDIUM: 'medium',
  LOW: 'low'
}

// 告警状态
export const ALARM_STATUS = {
  PENDING: 'pending',
  CONFIRMED: 'confirmed',
  RESOLVED: 'resolved'
}

// 告警类型
export const ALARM_TYPES = {
  VOLTAGE_ABNORMAL: '电压异常',
  CURRENT_ABNORMAL: '电流异常',
  DEVICE_OFFLINE: '设备离线',
  DEVICE_FAULT: '设备故障',
  BRIGHTNESS_ABNORMAL: '亮度异常',
  POWER_ABNORMAL: '电源异常'
}

// 设备分组
export const DEVICE_GROUPS = {
  AREA_A: 'A区',
  AREA_B: 'B区',
  AREA_C: 'C区',
  AREA_D: 'D区'
}

// 控制命令
export const CONTROL_COMMANDS = {
  TURN_ON: 'turn_on',
  TURN_OFF: 'turn_off',
  SET_BRIGHTNESS: 'set_brightness',
  SET_SCHEDULE: 'set_schedule',
  RESET_DEVICE: 'reset_device'
}

// API 响应码
export const RESPONSE_CODES = {
  SUCCESS: 200,
  CREATED: 201,
  BAD_REQUEST: 400,
  UNAUTHORIZED: 401,
  FORBIDDEN: 403,
  NOT_FOUND: 404,
  INTERNAL_ERROR: 500
}

// 日期格式
export const DATE_FORMATS = {
  DATE: 'YYYY-MM-DD',
  DATETIME: 'YYYY-MM-DD HH:mm:ss',
  TIME: 'HH:mm:ss'
}

// 图表颜色
export const CHART_COLORS = {
  PRIMARY: '#3498db',
  SUCCESS: '#2ecc71',
  WARNING: '#f39c12',
  DANGER: '#e74c3c',
  SECONDARY: '#95a5a6',
  INFO: '#1abc9c'
}

// 默认配置
export const DEFAULT_CONFIG = {
  PAGE_SIZE: 20,
  AUTO_REFRESH_INTERVAL: 30000, // 30秒
  CHART_ANIMATION_DURATION: 1000,
  MAX_BRIGHTNESS: 100,
  MIN_BRIGHTNESS: 0
}

// 用户角色
export const USER_ROLES = {
  ADMIN: 'admin',
  OPERATOR: 'operator',
  VIEWER: 'viewer'
}