import '../core/api_client.dart';
import '../models/device.dart';

class DeviceService {
  final ApiClient _apiClient = ApiClient();
  
  Future<List<Device>> getDevices() async {
    try {
      final response = await _apiClient.get('/devices');
      final List<dynamic> data = response.data as List<dynamic>;
      return data.map((json) => Device.fromJson(json as Map<String, dynamic>)).toList();
    } catch (e) {
      rethrow;
    }
  }
  
  Future<Device> getDevice(int deviceId) async {
    try {
      final response = await _apiClient.get('/devices/$deviceId');
      return Device.fromJson(response.data as Map<String, dynamic>);
    } catch (e) {
      rethrow;
    }
  }
  
  Future<Device> createDevice(String name, String deviceId, {String? location}) async {
    try {
      final response = await _apiClient.post(
        '/devices',
        data: {
          'name': name,
          'device_id': deviceId,
          'location': location,
        },
      );
      return Device.fromJson(response.data as Map<String, dynamic>);
    } catch (e) {
      rethrow;
    }
  }
  
  Future<Device> updateDevice(int deviceId, {String? name, String? location, bool? isActive}) async {
    try {
      final response = await _apiClient.put(
        '/devices/$deviceId',
        data: {
          if (name != null) 'name': name,
          if (location != null) 'location': location,
          if (isActive != null) 'is_active': isActive,
        },
      );
      return Device.fromJson(response.data as Map<String, dynamic>);
    } catch (e) {
      rethrow;
    }
  }
  
  Future<void> armDevice(int deviceId) async {
    try {
      await _apiClient.post('/devices/$deviceId/arm');
    } catch (e) {
      rethrow;
    }
  }
  
  Future<void> disarmDevice(int deviceId) async {
    try {
      await _apiClient.post('/devices/$deviceId/disarm');
    } catch (e) {
      rethrow;
    }
  }
  
  Future<DeviceConfig> getDeviceConfig(int deviceId) async {
    try {
      final response = await _apiClient.get('/devices/$deviceId/config');
      return DeviceConfig.fromJson(response.data as Map<String, dynamic>);
    } catch (e) {
      rethrow;
    }
  }
  
  Future<DeviceConfig> updateDeviceConfig(int deviceId, DeviceConfig config) async {
    try {
      final response = await _apiClient.put(
        '/devices/$deviceId/config',
        data: config.toJson(),
      );
      return DeviceConfig.fromJson(response.data as Map<String, dynamic>);
    } catch (e) {
      rethrow;
    }
  }
}

