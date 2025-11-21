import 'package:shared_preferences/shared_preferences.dart';
import '../core/api_client.dart';
import '../core/config.dart';
import '../models/user.dart';

class AuthService {
  final ApiClient _apiClient = ApiClient();
  
  Future<Token> login(String email, String password) async {
    try {
      final response = await _apiClient.post(
        '/auth/login',
        data: UserLogin(email: email, password: password).toJson(),
      );
      
      final token = Token.fromJson(response.data as Map<String, dynamic>);
      
      // Save token
      final prefs = await SharedPreferences.getInstance();
      await prefs.setString(AppConfig.keyAccessToken, token.accessToken);
      
      return token;
    } catch (e) {
      rethrow;
    }
  }
  
  Future<User> register(String email, String password, {String? fullName, String? phoneNumber}) async {
    try {
      final response = await _apiClient.post(
        '/auth/register',
        data: UserCreate(
          email: email,
          password: password,
          fullName: fullName,
          phoneNumber: phoneNumber,
        ).toJson(),
      );
      
      return User.fromJson(response.data as Map<String, dynamic>);
    } catch (e) {
      rethrow;
    }
  }
  
  Future<User> getCurrentUser() async {
    try {
      final response = await _apiClient.get('/auth/me');
      return User.fromJson(response.data as Map<String, dynamic>);
    } catch (e) {
      rethrow;
    }
  }
  
  Future<void> logout() async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.remove(AppConfig.keyAccessToken);
    await prefs.remove(AppConfig.keyRefreshToken);
    await prefs.remove(AppConfig.keyUserId);
    await prefs.remove(AppConfig.keyUserEmail);
  }
  
  Future<bool> isLoggedIn() async {
    final prefs = await SharedPreferences.getInstance();
    final token = prefs.getString(AppConfig.keyAccessToken);
    return token != null && token.isNotEmpty;
  }
}

