import 'package:cached_network_image/cached_network_image.dart'; // Import cached_network_image
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../config/app_config.dart';
import '../models/alert.dart';
import '../services/alert_service.dart';
import 'alert_detail_screen.dart';

class AlertFeedScreen extends StatefulWidget {
  const AlertFeedScreen({super.key});

  @override
  State<AlertFeedScreen> createState() => _AlertFeedScreenState();
}

class _AlertFeedScreenState extends State<AlertFeedScreen> {
  @override
  void initState() {
    super.initState();
    // Fetch alerts when screen loads
    WidgetsBinding.instance.addPostFrameCallback((_) {
      context.read<AlertService>().fetchAlerts();
    });
  }

  Future<void> _refresh() async {
    await context.read<AlertService>().fetchAlerts();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Consumer<AlertService>(
        builder: (context, alertService, child) {
          if (alertService.isLoading && alertService.alerts.isEmpty) {
            return const Center(child: CircularProgressIndicator());
          }

          if (alertService.error != null && alertService.alerts.isEmpty) {
            return Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  Text('Error: ${alertService.error}'),
                  const SizedBox(height: 16),
                  ElevatedButton(
                    onPressed: _refresh,
                    child: const Text('Retry'),
                  ),
                ],
              ),
            );
          }

          if (alertService.alerts.isEmpty) {
            return const Center(child: Text('No alerts found.'));
          }

          return RefreshIndicator(
            onRefresh: _refresh,
            child: ListView.builder(
              itemCount: alertService.alerts.length,
              itemBuilder: (context, index) {
                final alert = alertService.alerts[index];
                return AlertCard(alert: alert);
              },
            ),
          );
        },
      ),
    );
  }
}

class AlertCard extends StatelessWidget {
  final Alert alert;

  const AlertCard({super.key, required this.alert});

  @override
  Widget build(BuildContext context) {
    return Card(
      margin: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
      child: ListTile(
        leading: alert.imageUrl != null
            ? SizedBox(
                width: 60,
                height: 60,
                child: CachedNetworkImage(
                  imageUrl: alert.imageUrl!.startsWith('http')
                      ? alert.imageUrl!
                      : "${AppConfig.baseUrl}${alert.imageUrl}",
                  placeholder: (context, url) =>
                      const CircularProgressIndicator(),
                  errorWidget: (context, url, error) => const Icon(Icons.error),
                  fit: BoxFit.cover,
                ),
              )
            : const Icon(
                Icons.warning_amber_rounded,
                size: 40,
                color: Colors.orange,
              ),
        title: Text('${alert.alertType.toUpperCase()} ALERT'),
        subtitle: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(alert.formattedTimestamp),
            Text(
              'Confidence: ${(alert.detectionConfidence * 100).toStringAsFixed(1)}%',
            ),
          ],
        ),
        isThreeLine: true,
        onTap: () {
          Navigator.push(
            context,
            MaterialPageRoute(
              builder: (context) => AlertDetailScreen(alert: alert),
            ),
          );
        },
      ),
    );
  }
}
