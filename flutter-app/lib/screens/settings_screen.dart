import 'package:dio/dio.dart';
import 'package:flutter/material.dart';
import 'package:flutter_secure_storage/flutter_secure_storage.dart';
import 'package:provider/provider.dart';

import '../config/app_config.dart';
import '../services/auth_service.dart';
import 'telegram_config_screen.dart';

class SettingsScreen extends StatefulWidget {
  const SettingsScreen({super.key});

  @override
  State<SettingsScreen> createState() => _SettingsScreenState();
}

class _SettingsScreenState extends State<SettingsScreen> {
  bool _isLoading = false;
  Map<String, dynamic>? _systemStatus;

  @override
  void initState() {
    super.initState();
    _fetchSystemStatus();
  }

  Future<void> _fetchSystemStatus() async {
    setState(() {
      _isLoading = true;
    });

    try {
      final token = await const FlutterSecureStorage().read(
        key: AppConfig.keyToken,
      );
      if (token == null) return;

      final response = await Dio().get(
        '${AppConfig.baseUrl}/status',
        options: Options(
          headers: {'Authorization': 'Bearer $token'},
          receiveTimeout: const Duration(
            milliseconds: AppConfig.receiveTimeoutMs,
          ),
          sendTimeout: const Duration(milliseconds: AppConfig.connectTimeoutMs),
        ),
      );

      if (mounted && response.statusCode == 200) {
        setState(() {
          _systemStatus = response.data;
        });
      }
    } catch (e) {
      // Handle error silently or show snackbar
      debugPrint("Error fetching status: $e");
    } finally {
      if (mounted) {
        setState(() {
          _isLoading = false;
        });
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Settings')),
      body: ListView(
        children: [
          _buildStatusCard(),
          const Divider(),
          ListTile(
            leading: const Icon(Icons.telegram),
            title: const Text('Telegram Configuration'),
            subtitle: const Text('Manage bot token and chat ID'),
            trailing: const Icon(Icons.arrow_forward_ios, size: 16),
            onTap: () {
              Navigator.push(
                context,
                MaterialPageRoute(builder: (_) => const TelegramConfigScreen()),
              );
            },
          ),
          const Divider(),
          ListTile(
            leading: const Icon(Icons.logout, color: Colors.red),
            title: const Text('Logout', style: TextStyle(color: Colors.red)),
            onTap: () {
              context.read<AuthService>().logout();
              Navigator.of(context).popUntil((route) => route.isFirst);
            },
          ),
        ],
      ),
    );
  }

  Widget _buildStatusCard() {
    if (_isLoading) {
      return const Padding(
        padding: EdgeInsets.all(16.0),
        child: Center(child: CircularProgressIndicator()),
      );
    }

    if (_systemStatus == null) {
      return ListTile(
        leading: const Icon(Icons.error_outline, color: Colors.orange),
        title: const Text('System Status Unavailable'),
        trailing: IconButton(
          icon: const Icon(Icons.refresh),
          onPressed: _fetchSystemStatus,
        ),
      );
    }

    return Card(
      margin: const EdgeInsets.all(16),
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              'System Health',
              style: Theme.of(context).textTheme.titleLarge,
            ),
            const SizedBox(height: 16),
            _buildStatusRow(
              'Status',
              _systemStatus!['status'] ?? 'Unknown',
              Colors.green,
            ),
            _buildStatusRow(
              'Total Alerts',
              '${_systemStatus!['total_alerts']}',
            ),
            _buildStatusRow('Today', '${_systemStatus!['alerts_today']}'),
            _buildStatusRow(
              'Last Alert',
              _systemStatus!['last_alert'] ?? 'None',
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildStatusRow(String label, String value, [Color? color]) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(label, style: const TextStyle(fontWeight: FontWeight.bold)),
          Text(value, style: TextStyle(color: color)),
        ],
      ),
    );
  }
}
