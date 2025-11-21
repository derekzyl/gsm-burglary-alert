class Alert {
  final int id;
  final int deviceId;
  final int? sensorId;
  final AlertType alertType;
  final AlertStatus status;
  final String message;
  final String severity;
  final int? acknowledgedBy;
  final DateTime? acknowledgedAt;
  final DateTime? resolvedAt;
  final String? metadata;
  final DateTime createdAt;
  
  Alert({
    required this.id,
    required this.deviceId,
    this.sensorId,
    required this.alertType,
    required this.status,
    required this.message,
    required this.severity,
    this.acknowledgedBy,
    this.acknowledgedAt,
    this.resolvedAt,
    this.metadata,
    required this.createdAt,
  });
  
  factory Alert.fromJson(Map<String, dynamic> json) {
    return Alert(
      id: json['id'] as int,
      deviceId: json['device_id'] as int,
      sensorId: json['sensor_id'] as int?,
      alertType: AlertType.fromString(json['alert_type'] as String),
      status: AlertStatus.fromString(json['status'] as String),
      message: json['message'] as String,
      severity: json['severity'] as String,
      acknowledgedBy: json['acknowledged_by'] as int?,
      acknowledgedAt: json['acknowledged_at'] != null
          ? DateTime.parse(json['acknowledged_at'] as String)
          : null,
      resolvedAt: json['resolved_at'] != null
          ? DateTime.parse(json['resolved_at'] as String)
          : null,
      metadata: json['metadata'] as String?,
      createdAt: DateTime.parse(json['created_at'] as String),
    );
  }
}

enum AlertType {
  intrusion,
  tamper,
  lowBattery,
  deviceOffline,
  gsmSignalLow,
  sensorFault;
  
  static AlertType fromString(String type) {
    switch (type.toLowerCase()) {
      case 'intrusion':
        return AlertType.intrusion;
      case 'tamper':
        return AlertType.tamper;
      case 'low_battery':
        return AlertType.lowBattery;
      case 'device_offline':
        return AlertType.deviceOffline;
      case 'gsm_signal_low':
        return AlertType.gsmSignalLow;
      case 'sensor_fault':
        return AlertType.sensorFault;
      default:
        return AlertType.intrusion;
    }
  }
  
  String get displayName {
    switch (this) {
      case AlertType.intrusion:
        return 'Intrusion';
      case AlertType.tamper:
        return 'Tamper';
      case AlertType.lowBattery:
        return 'Low Battery';
      case AlertType.deviceOffline:
        return 'Device Offline';
      case AlertType.gsmSignalLow:
        return 'Low GSM Signal';
      case AlertType.sensorFault:
        return 'Sensor Fault';
    }
  }
  
  String get emoji {
    switch (this) {
      case AlertType.intrusion:
        return '🚨';
      case AlertType.tamper:
        return '⚠️';
      case AlertType.lowBattery:
        return '🔋';
      case AlertType.deviceOffline:
        return '📡';
      case AlertType.gsmSignalLow:
        return '📶';
      case AlertType.sensorFault:
        return '⚙️';
    }
  }
}

enum AlertStatus {
  pending,
  acknowledged,
  falseAlarm,
  resolved;
  
  static AlertStatus fromString(String status) {
    switch (status.toLowerCase()) {
      case 'pending':
        return AlertStatus.pending;
      case 'acknowledged':
        return AlertStatus.acknowledged;
      case 'false_alarm':
        return AlertStatus.falseAlarm;
      case 'resolved':
        return AlertStatus.resolved;
      default:
        return AlertStatus.pending;
    }
  }
  
  String get displayName {
    switch (this) {
      case AlertStatus.pending:
        return 'Pending';
      case AlertStatus.acknowledged:
        return 'Acknowledged';
      case AlertStatus.falseAlarm:
        return 'False Alarm';
      case AlertStatus.resolved:
        return 'Resolved';
    }
  }
}

