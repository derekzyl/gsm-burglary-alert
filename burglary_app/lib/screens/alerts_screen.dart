import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:intl/intl.dart';
import '../providers/alert_provider.dart';
import '../models/alert.dart';
import 'alert_detail_screen.dart';

class AlertsScreen extends StatelessWidget {
  const AlertsScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Consumer<AlertProvider>(
      builder: (context, provider, child) {
        if (provider.isLoading && provider.alerts.isEmpty) {
          return const Center(child: CircularProgressIndicator());
        }
        
        if (provider.alerts.isEmpty) {
          return const Center(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Icon(Icons.check_circle, size: 64, color: Colors.green),
                SizedBox(height: 16),
                Text('No alerts'),
              ],
            ),
          );
        }
        
        return RefreshIndicator(
          onRefresh: () => provider.loadAlerts(),
          child: ListView.builder(
            padding: const EdgeInsets.all(8),
            itemCount: provider.alerts.length,
            itemBuilder: (context, index) {
              final alert = provider.alerts[index];
              return Card(
                margin: const EdgeInsets.symmetric(vertical: 4, horizontal: 8),
                color: _getAlertColor(alert),
                child: ListTile(
                  leading: Text(
                    alert.alertType.emoji,
                    style: const TextStyle(fontSize: 32),
                  ),
                  title: Text(
                    alert.alertType.displayName,
                    style: const TextStyle(fontWeight: FontWeight.bold),
                  ),
                  subtitle: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(alert.message),
                      Text(
                        DateFormat('MMM d, y HH:mm').format(alert.createdAt),
                        style: const TextStyle(fontSize: 12),
                      ),
                    ],
                  ),
                  trailing: alert.status == AlertStatus.pending
                      ? const Icon(Icons.warning, color: Colors.orange)
                      : null,
                  onTap: () {
                    Navigator.push(
                      context,
                      MaterialPageRoute(
                        builder: (_) => AlertDetailScreen(alert: alert),
                      ),
                    );
                  },
                ),
              );
            },
          ),
        );
      },
    );
  }
  
  Color _getAlertColor(Alert alert) {
    if (alert.status == AlertStatus.pending) {
      return Colors.orange.shade50;
    } else if (alert.status == AlertStatus.resolved) {
      return Colors.green.shade50;
    }
    return Colors.grey.shade50;
  }
}

