import 'package:dio/dio.dart';
import 'package:flutter/material.dart';
import 'package:flutter_secure_storage/flutter_secure_storage.dart';

import '../config/app_config.dart';

class AuthService with ChangeNotifier {
  final Dio _dio = Dio();
  final FlutterSecureStorage _storage = const FlutterSecureStorage();

  String? _token;
  bool _isLoading = false;
  String? _error;

  bool get isAuthenticated => _token != null;
  bool get isLoading => _isLoading;
  String? get error => _error;

  AuthService() {
    _loadToken();
  }

  Future<void> _loadToken() async {
    _token = await _storage.read(key: AppConfig.keyToken);
    notifyListeners();
  }

  Future<bool> login(String username, String password) async {
    _isLoading = true;
    _error = null;
    notifyListeners();

    try {
      final response = await _dio.post(
        '${AppConfig.baseUrl}/auth/login',
        data: {'username': username, 'password': password},
        options: Options(
          receiveTimeout: const Duration(
            milliseconds: AppConfig.receiveTimeoutMs,
          ),
          sendTimeout: const Duration(milliseconds: AppConfig.connectTimeoutMs),
        ),
      );

      if (response.statusCode == 200) {
        final data = response.data;
        _token = data['access_token'];
        if (_token != null) {
          await _storage.write(key: AppConfig.keyToken, value: _token);
          await _storage.write(key: AppConfig.keyUsername, value: username);
        }
        _isLoading = false;
        notifyListeners();
        return true;
      } else {
        _error = 'Login failed: ${response.statusCode}';
      }
    } on DioException catch (e) {
      if (e.response != null) {
        _error = e.response?.data['detail'] ?? 'Login failed';
      } else {
        _error = 'Connection error: ${e.message}';
      }
    } catch (e) {
      _error = 'An unexpected error occurred';
    }

    _isLoading = false;
    notifyListeners();
    return false;
  }

  Future<void> logout() async {
    _token = null;
    await _storage.delete(key: AppConfig.keyToken);
    await _storage.delete(key: AppConfig.keyUsername);
    notifyListeners();
  }
}
