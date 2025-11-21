import 'package:flutter/foundation.dart';
import '../models/device.dart';
import '../services/device_service.dart';

class DeviceProvider with ChangeNotifier {
  final DeviceService _deviceService = DeviceService();
  List<Device> _devices = [];
  Device? _selectedDevice;
  bool _isLoading = false;
  String? _error;
  
  List<Device> get devices => _devices;
  Device? get selectedDevice => _selectedDevice;
  bool get isLoading => _isLoading;
  String? get error => _error;
  
  Future<void> loadDevices() async {
    _isLoading = true;
    _error = null;
    notifyListeners();
    
    try {
      _devices = await _deviceService.getDevices();
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _error = e.toString();
      _isLoading = false;
      notifyListeners();
    }
  }
  
  Future<void> selectDevice(Device device) async {
    _selectedDevice = device;
    try {
      _selectedDevice = await _deviceService.getDevice(device.id);
      notifyListeners();
    } catch (e) {
      _error = e.toString();
      notifyListeners();
    }
  }
  
  Future<void> armDevice(int deviceId) async {
    try {
      await _deviceService.armDevice(deviceId);
      await loadDevices();
    } catch (e) {
      _error = e.toString();
      notifyListeners();
    }
  }
  
  Future<void> disarmDevice(int deviceId) async {
    try {
      await _deviceService.disarmDevice(deviceId);
      await loadDevices();
    } catch (e) {
      _error = e.toString();
      notifyListeners();
    }
  }
  
  Future<void> createDevice(String name, String deviceId, {String? location}) async {
    try {
      await _deviceService.createDevice(name, deviceId, location: location);
      await loadDevices();
    } catch (e) {
      _error = e.toString();
      notifyListeners();
    }
  }
}

