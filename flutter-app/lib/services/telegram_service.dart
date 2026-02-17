import 'package:dio/dio.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter_secure_storage/flutter_secure_storage.dart';

import '../config/app_config.dart';

class TelegramService with ChangeNotifier {
  final Dio _dio = Dio();
  final FlutterSecureStorage _storage = const FlutterSecureStorage();

  bool _isLoading = false;
  String? _error;
  String? _successMessage;

  bool get isLoading => _isLoading;
  String? get error => _error;
  String? get successMessage => _successMessage;

  Future<String?> _getToken() async {
    return await _storage.read(key: AppConfig.keyToken);
  }

  Future<Map<String, dynamic>?> getConfig() async {
    _isLoading = true;
    _error = null;
    notifyListeners();

    try {
      final token = await _getToken();
      if (token == null) throw Exception("Authentication required");

      final response =       await _dio.get(
        '${AppConfig.baseUrl}/telegram/config',
        options: Options(
          headers: {'Authorization': 'Bearer $token'},
          receiveTimeout: const Duration(
            milliseconds: AppConfig.receiveTimeoutMs,
          ),
          sendTimeout: const Duration(
            milliseconds: AppConfig.connectTimeoutMs,
          ),
        ),
      );

      _isLoading = false;
      notifyListeners();
      return response.data;
    } catch (e) {
      _error = "Failed to load config: $e";
      _isLoading = false;
      notifyListeners();
      return null;
    }
  }

  Future<bool> saveConfig(String botToken, String chatId, bool active) async {
    _isLoading = true;
    _error = null;
    _successMessage = null;
    notifyListeners();

    try {
      final token = await _getToken();
      if (token == null) throw Exception("Authentication required");

      await _dio.post(
        '${AppConfig.baseUrl}/telegram/config',
        data: {'bot_token': botToken, 'chat_id': chatId, 'active': active},
        options: Options(
          headers: {'Authorization': 'Bearer $token'},
          receiveTimeout: const Duration(milliseconds: AppConfig.receiveTimeoutMs),
          sendTimeout: const Duration(milliseconds: AppConfig.connectTimeoutMs),
        ),
      );

      _successMessage = "Configuration saved successfully";
      _isLoading = false;
      notifyListeners();
      return true;
    } catch (e) {
      _error = "Failed to save config: $e";
      _isLoading = false;
      notifyListeners();
      return false;
    }
  }

  Future<bool> testConnection() async {
    _isLoading = true;
    _error = null;
    _successMessage = null;
    notifyListeners();

    try {
      final token = await _getToken();
      if (token == null) throw Exception("Authentication required");

      await _dio.post(
        '${AppConfig.baseUrl}/telegram/test',
        options: Options(
          headers: {'Authorization': 'Bearer $token'},
          receiveTimeout: const Duration(milliseconds: AppConfig.receiveTimeoutMs),
          sendTimeout: const Duration(milliseconds: AppConfig.connectTimeoutMs),
        ),
      );

      _successMessage = "Connection test successful!";
      _isLoading = false;
      notifyListeners();
      return true;
    } on DioException catch (e) {
      final detail = e.response?.data is Map
          ? (e.response!.data['detail'] ?? e.response!.data['message'])
          : null;
      _error = detail?.toString() ??
          "Connection test failed. Check token (no spaces/newlines) and chat ID.";
      _isLoading = false;
      notifyListeners();
      return false;
    } catch (e) {
      _error = "Connection test failed. Check your token and chat ID.";
      _isLoading = false;
      notifyListeners();
      return false;
    }
  }
}
