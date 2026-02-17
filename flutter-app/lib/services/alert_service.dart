import 'package:dio/dio.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter_secure_storage/flutter_secure_storage.dart';

import '../config/app_config.dart';
import '../models/alert.dart';

class AlertService with ChangeNotifier {
  final Dio _dio = Dio();
  final FlutterSecureStorage _storage = const FlutterSecureStorage();

  static const int _pageSize = 20;

  List<Alert> _alerts = [];
  int _total = 0;
  int _offset = 0;
  bool _isLoading = false;
  bool _isLoadingMore = false;
  String? _error;

  List<Alert> get alerts => _alerts;
  bool get isLoading => _isLoading;
  bool get isLoadingMore => _isLoadingMore;
  String? get error => _error;
  int get total => _total;
  bool get hasMore => _alerts.length < _total;

  Future<String?> _getToken() async {
    return await _storage.read(key: AppConfig.keyToken);
  }

  Future<void> fetchAlerts({bool reset = true}) async {
    if (reset) {
      _offset = 0;
      _alerts = [];
      _isLoading = true;
    }
    if (reset) _error = null;
    notifyListeners();

    try {
      final token = await _getToken();
      if (token == null) {
        _error = "Authentication required";
        if (reset) _isLoading = false;
        notifyListeners();
        return;
      }

      final response = await _dio.get(
        '${AppConfig.baseUrl}/alert/feeds',
        queryParameters: {'limit': _pageSize, 'offset': _offset},
        options: Options(
          headers: {'Authorization': 'Bearer $token'},
          receiveTimeout: const Duration(
            milliseconds: AppConfig.feedReceiveTimeoutMs,
          ),
          sendTimeout: const Duration(
            milliseconds: AppConfig.connectTimeoutMs,
          ),
        ),
      );

      if (response.statusCode == 200) {
        final raw = response.data;
        List<dynamic>? list;
        int? totalCount;

        if (raw is List) {
          list = raw;
          totalCount = raw.length;
        } else if (raw is Map) {
          // Paginated format: { data: [...], total: N, limit, offset }
          final map = raw;
          list = map['data'] as List<dynamic>?;
          totalCount = map['total'] as int?;
          if (list == null && map['items'] != null) {
            list = map['items'] as List<dynamic>?;
          }
        }

        if (list != null) {
          final page = list
              .map((e) {
            final m = e is Map ? Map<String, dynamic>.from(e) : <String, dynamic>{};
            return Alert.fromJson(m);
          })
              .toList();
          if (reset) {
            _alerts = page;
          } else {
            _alerts = [..._alerts, ...page];
          }
          _total = totalCount ?? _alerts.length;
          _offset = _alerts.length;
        } else {
          _error = 'Unexpected response format from server';
        }
      } else {
        _error = 'Failed to load alerts: ${response.statusCode}';
      }
    } on DioException catch (e) {
      if (e.response != null) {
        final detail = e.response?.data is Map
            ? e.response!.data['detail']?.toString()
            : null;
        _error = detail ?? 'Failed to load alerts';
      } else {
        switch (e.type) {
          case DioExceptionType.connectionTimeout:
          case DioExceptionType.sendTimeout:
          case DioExceptionType.receiveTimeout:
            _error =
                'Request took too long. The server may be slow—pull down to refresh.';
            break;
          default:
            _error = e.message?.isNotEmpty == true
                ? e.message!
                : 'Connection problem. Check network and try again.';
        }
      }
    } catch (e, stack) {
      if (e is TypeError || e.toString().contains('fromJson')) {
        _error = 'Could not parse server response. Try again.';
      } else {
        _error = 'An unexpected error occurred';
      }
      debugPrint('fetchAlerts error: $e\n$stack');
    }

    if (reset) _isLoading = false;
    notifyListeners();
  }

  /// Load next page and append to list. No-op if already loading or no more data.
  Future<void> loadMore() async {
    if (_isLoadingMore || _isLoading || !hasMore) return;
    _isLoadingMore = true;
    notifyListeners();
    await fetchAlerts(reset: false);
    _isLoadingMore = false;
    notifyListeners();
  }
}
