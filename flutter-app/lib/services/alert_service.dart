import 'package:dio/dio.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter_secure_storage/flutter_secure_storage.dart';

import '../config/app_config.dart';
import '../models/alert.dart';

class AlertService with ChangeNotifier {
  final Dio _dio = Dio();
  final FlutterSecureStorage _storage = const FlutterSecureStorage();

  List<Alert> _alerts = [];
  bool _isLoading = false;
  String? _error;

  List<Alert> get alerts => _alerts;
  bool get isLoading => _isLoading;
  String? get error => _error;

  Future<String?> _getToken() async {
    return await _storage.read(key: AppConfig.keyToken);
  }

  Future<void> fetchAlerts({int limit = 50, int offset = 0}) async {
    _isLoading = true;
    _error = null;
    notifyListeners();

    try {
      final token = await _getToken();
      if (token == null) {
        _error = "Authentication required";
        _isLoading = false;
        notifyListeners();
        return;
      }

      final response = await _dio.get(
        '${AppConfig.baseUrl}/alert/feeds',
        queryParameters: {'limit': limit, 'offset': offset},
        options: Options(
          headers: {'Authorization': 'Bearer $token'},
          receiveTimeout: const Duration(
            milliseconds: AppConfig.receiveTimeout,
          ),
          sendTimeout: const Duration(milliseconds: AppConfig.connectTimeout),
        ),
      );

      if (response.statusCode == 200) {
        final List<dynamic> data = response.data;
        _alerts = data.map((json) => Alert.fromJson(json)).toList();
      } else {
        _error = 'Failed to load alerts: ${response.statusCode}';
      }
    } on DioException catch (e) {
      if (e.response != null) {
        _error = e.response?.data['detail'] ?? 'Failed to load alerts';
      } else {
        _error = 'Connection error: ${e.message}';
      }
    } catch (e) {
      _error = 'An unexpected error occurred';
    }

    _isLoading = false;
    notifyListeners();
  }
}
