class AppConfig {
  // Point to the deployed backend URL
  static const String baseUrl =
      "https://xenophobic-netta-cybergenii-1584fde7.koyeb.app/api/v1/burglary";

  // Timeouts: backend may be cold or return large payloads (e.g. 50 alerts)
  static const int connectTimeoutMs = 15000;  // 15s to establish connection
  static const int receiveTimeoutMs = 30000;  // 30s to receive full response
  static const int feedReceiveTimeoutMs = 45000;  // 45s for alert feed (can be large)

  // Legacy names for compatibility
  static const int connectTimeout = connectTimeoutMs;
  static const int receiveTimeout = receiveTimeoutMs;

  // Storage keys
  static const String keyToken = "auth_token";
  static const String keyUsername = "username";
}
