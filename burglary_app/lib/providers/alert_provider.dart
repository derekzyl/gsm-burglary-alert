import 'package:flutter/foundation.dart';
import '../models/alert.dart';
import '../services/alert_service.dart';

class AlertProvider with ChangeNotifier {
  final AlertService _alertService = AlertService();
  List<Alert> _alerts = [];
  Alert? _selectedAlert;
  bool _isLoading = false;
  String? _error;
  
  List<Alert> get alerts => _alerts;
  Alert? get selectedAlert => _selectedAlert;
  bool get isLoading => _isLoading;
  String? get error => _error;
  
  List<Alert> get pendingAlerts => _alerts.where((a) => a.status == AlertStatus.pending).toList();
  
  Future<void> loadAlerts({int? deviceId, AlertStatus? status}) async {
    _isLoading = true;
    _error = null;
    notifyListeners();
    
    try {
      _alerts = await _alertService.getAlerts(deviceId: deviceId, status: status);
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _error = e.toString();
      _isLoading = false;
      notifyListeners();
    }
  }
  
  Future<void> acknowledgeAlert(int alertId) async {
    try {
      await _alertService.acknowledgeAlert(alertId);
      await loadAlerts();
    } catch (e) {
      _error = e.toString();
      notifyListeners();
    }
  }
  
  Future<void> resolveAlert(int alertId, {String? notes}) async {
    try {
      await _alertService.resolveAlert(alertId, notes: notes);
      await loadAlerts();
    } catch (e) {
      _error = e.toString();
      notifyListeners();
    }
  }
  
  void selectAlert(Alert alert) {
    _selectedAlert = alert;
    notifyListeners();
  }
}

