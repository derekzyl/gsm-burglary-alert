
class AppConfig {
  // Point to the deployed backend URL
  static const String baseUrl = "https://xenophobic-netta-cybergenii-1584fde7.koyeb.app/api/v1/burglary";
  
  // Use a shorter timeout for mobile
  static const int connectTimeout = 10000;
  static const int receiveTimeout = 10000;
  
  // Storage keys
  static const String keyToken = "auth_token";
  static const String keyUsername = "username";
}
