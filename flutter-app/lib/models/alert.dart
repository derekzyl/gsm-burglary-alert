import 'package:intl/intl.dart';

class Alert {
  final int id;
  final String timestamp;
  final String alertType;
  final double detectionConfidence;
  final Map<String, dynamic> pirSensorsTriggered;
  final String networkStatus;
  final int? imageId;
  final bool correlated;
  final String? imageUrl;

  Alert({
    required this.id,
    required this.timestamp,
    required this.alertType,
    required this.detectionConfidence,
    required this.pirSensorsTriggered,
    required this.networkStatus,
    this.imageId,
    required this.correlated,
    this.imageUrl,
  });

  /// Parses API timestamp (ISO 8601 UTC e.g. with Z) and returns formatted local date & time.
  String get formattedTimestamp {
    try {
      final dt = DateTime.parse(timestamp);
      final local = dt.toLocal();
      return DateFormat.yMMMd().add_Hm().format(local);
    } catch (_) {
      return timestamp;
    }
  }

  factory Alert.fromJson(Map<String, dynamic> json) {
    return Alert(
      id: json['id'],
      timestamp: json['timestamp'] as String? ?? '',
      alertType: json['alert_type'],
      detectionConfidence: (json['detection_confidence'] as num).toDouble(),
      pirSensorsTriggered: json['pir_sensors_triggered'],
      networkStatus: json['network_status'],
      imageId: json['image_id'],
      correlated: json['correlated'],
      imageUrl: json['image_url'],
    );
  }
}
