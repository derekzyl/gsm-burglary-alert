import '../core/api_client.dart';
import '../models/alert.dart';

class AlertService {
  final ApiClient _apiClient = ApiClient();
  
  Future<List<Alert>> getAlerts({
    int? deviceId,
    AlertStatus? status,
    int limit = 50,
    int offset = 0,
  }) async {
    try {
      final queryParams = <String, dynamic>{
        'limit': limit,
        'offset': offset,
      };
      
      if (deviceId != null) queryParams['device_id'] = deviceId;
      if (status != null) queryParams['status'] = status.name;
      
      final response = await _apiClient.get(
        '/alerts',
        queryParameters: queryParams,
      );
      
      final List<dynamic> data = response.data as List<dynamic>;
      return data.map((json) => Alert.fromJson(json as Map<String, dynamic>)).toList();
    } catch (e) {
      rethrow;
    }
  }
  
  Future<Alert> getAlert(int alertId) async {
    try {
      final response = await _apiClient.get('/alerts/$alertId');
      return Alert.fromJson(response.data as Map<String, dynamic>);
    } catch (e) {
      rethrow;
    }
  }
  
  Future<Alert> acknowledgeAlert(int alertId) async {
    try {
      final response = await _apiClient.put('/alerts/$alertId/acknowledge');
      return Alert.fromJson(response.data as Map<String, dynamic>);
    } catch (e) {
      rethrow;
    }
  }
  
  Future<Alert> resolveAlert(int alertId, {String? notes}) async {
    try {
      final response = await _apiClient.put(
        '/alerts/$alertId/resolve',
        data: {'status': 'resolved', if (notes != null) 'notes': notes},
      );
      return Alert.fromJson(response.data as Map<String, dynamic>);
    } catch (e) {
      rethrow;
    }
  }
}

