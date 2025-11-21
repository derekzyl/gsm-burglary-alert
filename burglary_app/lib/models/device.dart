class Device {
  final int id;
  final String name;
  final String deviceId;
  final DeviceStatus status;
  final String? firmwareVersion;
  final DateTime? lastSeen;
  final String? location;
  final int? gsmSignalStrength;
  final double? batteryLevel;
  final bool isActive;
  final int userId;
  final DateTime createdAt;
  final List<DeviceSensor> sensors;
  final DeviceConfig? config;
  
  Device({
    required this.id,
    required this.name,
    required this.deviceId,
    required this.status,
    this.firmwareVersion,
    this.lastSeen,
    this.location,
    this.gsmSignalStrength,
    this.batteryLevel,
    required this.isActive,
    required this.userId,
    required this.createdAt,
    this.sensors = const [],
    this.config,
  });
  
  factory Device.fromJson(Map<String, dynamic> json) {
    return Device(
      id: json['id'] as int,
      name: json['name'] as String,
      deviceId: json['device_id'] as String,
      status: DeviceStatus.fromString(json['status'] as String),
      firmwareVersion: json['firmware_version'] as String?,
      lastSeen: json['last_seen'] != null
          ? DateTime.parse(json['last_seen'] as String)
          : null,
      location: json['location'] as String?,
      gsmSignalStrength: json['gsm_signal_strength'] as int?,
      batteryLevel: json['battery_level'] != null
          ? (json['battery_level'] as num).toDouble()
          : null,
      isActive: json['is_active'] as bool,
      userId: json['user_id'] as int,
      createdAt: DateTime.parse(json['created_at'] as String),
      sensors: json['sensors'] != null
          ? (json['sensors'] as List)
              .map((s) => DeviceSensor.fromJson(s as Map<String, dynamic>))
              .toList()
          : [],
      config: json['config'] != null
          ? DeviceConfig.fromJson(json['config'] as Map<String, dynamic>)
          : null,
    );
  }
}

enum DeviceStatus {
  online,
  offline,
  armed,
  disarmed,
  alarm,
  maintenance;
  
  static DeviceStatus fromString(String status) {
    switch (status.toLowerCase()) {
      case 'online':
        return DeviceStatus.online;
      case 'offline':
        return DeviceStatus.offline;
      case 'armed':
        return DeviceStatus.armed;
      case 'disarmed':
        return DeviceStatus.disarmed;
      case 'alarm':
        return DeviceStatus.alarm;
      case 'maintenance':
        return DeviceStatus.maintenance;
      default:
        return DeviceStatus.offline;
    }
  }
  
  String get displayName {
    switch (this) {
      case DeviceStatus.online:
        return 'Online';
      case DeviceStatus.offline:
        return 'Offline';
      case DeviceStatus.armed:
        return 'Armed';
      case DeviceStatus.disarmed:
        return 'Disarmed';
      case DeviceStatus.alarm:
        return 'Alarm';
      case DeviceStatus.maintenance:
        return 'Maintenance';
    }
  }
}

class DeviceSensor {
  final int id;
  final int deviceId;
  final String sensorId;
  final SensorType sensorType;
  final String name;
  final String? location;
  final bool isActive;
  final DateTime? lastTriggered;
  final DateTime createdAt;
  
  DeviceSensor({
    required this.id,
    required this.deviceId,
    required this.sensorId,
    required this.sensorType,
    required this.name,
    this.location,
    required this.isActive,
    this.lastTriggered,
    required this.createdAt,
  });
  
  factory DeviceSensor.fromJson(Map<String, dynamic> json) {
    return DeviceSensor(
      id: json['id'] as int,
      deviceId: json['device_id'] as int,
      sensorId: json['sensor_id'] as String,
      sensorType: SensorType.fromString(json['sensor_type'] as String),
      name: json['name'] as String,
      location: json['location'] as String?,
      isActive: json['is_active'] as bool,
      lastTriggered: json['last_triggered'] != null
          ? DateTime.parse(json['last_triggered'] as String)
          : null,
      createdAt: DateTime.parse(json['created_at'] as String),
    );
  }
}

enum SensorType {
  pir,
  magnetic,
  shock,
  vibration,
  tamper;
  
  static SensorType fromString(String type) {
    switch (type.toLowerCase()) {
      case 'pir':
        return SensorType.pir;
      case 'magnetic':
        return SensorType.magnetic;
      case 'shock':
        return SensorType.shock;
      case 'vibration':
        return SensorType.vibration;
      case 'tamper':
        return SensorType.tamper;
      default:
        return SensorType.pir;
    }
  }
}

class DeviceConfig {
  final int id;
  final int deviceId;
  final int alarmDelaySeconds;
  final bool notificationEnabled;
  final bool smsEnabled;
  final bool callEnabled;
  final bool httpEnabled;
  final bool autoArmEnabled;
  final String? autoArmTime;
  final bool silentMode;
  final List<String> phoneNumbers;
  final DateTime createdAt;
  final DateTime? updatedAt;
  
  DeviceConfig({
    required this.id,
    required this.deviceId,
    required this.alarmDelaySeconds,
    required this.notificationEnabled,
    required this.smsEnabled,
    required this.callEnabled,
    required this.httpEnabled,
    required this.autoArmEnabled,
    this.autoArmTime,
    required this.silentMode,
    required this.phoneNumbers,
    required this.createdAt,
    this.updatedAt,
  });
  
  factory DeviceConfig.fromJson(Map<String, dynamic> json) {
    return DeviceConfig(
      id: json['id'] as int,
      deviceId: json['device_id'] as int,
      alarmDelaySeconds: json['alarm_delay_seconds'] as int,
      notificationEnabled: json['notification_enabled'] as bool,
      smsEnabled: json['sms_enabled'] as bool,
      callEnabled: json['call_enabled'] as bool,
      httpEnabled: json['http_enabled'] as bool,
      autoArmEnabled: json['auto_arm_enabled'] as bool,
      autoArmTime: json['auto_arm_time'] as String?,
      silentMode: json['silent_mode'] as bool,
      phoneNumbers: (json['phone_numbers'] as List? ?? [])
          .map((e) => e.toString())
          .toList(),
      createdAt: DateTime.parse(json['created_at'] as String),
      updatedAt: json['updated_at'] != null
          ? DateTime.parse(json['updated_at'] as String)
          : null,
    );
  }
  
  Map<String, dynamic> toJson() {
    return {
      'alarm_delay_seconds': alarmDelaySeconds,
      'notification_enabled': notificationEnabled,
      'sms_enabled': smsEnabled,
      'call_enabled': callEnabled,
      'http_enabled': httpEnabled,
      'auto_arm_enabled': autoArmEnabled,
      'auto_arm_time': autoArmTime,
      'silent_mode': silentMode,
      'phone_numbers': phoneNumbers,
    };
  }
}

