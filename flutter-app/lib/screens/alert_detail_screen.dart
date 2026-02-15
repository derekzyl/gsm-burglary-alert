import 'package:cached_network_image/cached_network_image.dart';
import 'package:flutter/material.dart';

import '../config/app_config.dart';
import '../models/alert.dart';

class AlertDetailScreen extends StatelessWidget {
  final Alert alert;

  const AlertDetailScreen({super.key, required this.alert});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Alert Details')),
      body: SingleChildScrollView(
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            if (alert.imageUrl != null)
              CachedNetworkImage(
                imageUrl: "${AppConfig.baseUrl}${alert.imageUrl}",
                fit: BoxFit.cover,
                placeholder: (context, url) => const SizedBox(
                  height: 300,
                  child: Center(child: CircularProgressIndicator()),
                ),
                errorWidget: (context, url, error) => const SizedBox(
                  height: 300,
                  child: Center(child: Icon(Icons.broken_image, size: 50)),
                ),
              )
            else
              Container(
                height: 200,
                color: Colors.grey[200],
                child: const Center(
                  child: Icon(
                    Icons.no_photography,
                    size: 50,
                    color: Colors.grey,
                  ),
                ),
              ),

            Padding(
              padding: const EdgeInsets.all(16.0),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  _buildHeader(context),
                  const SizedBox(height: 24),
                  _buildSectionTitle(context, 'Detection Info'),
                  _buildInfoRow(
                    'Confidence',
                    '${(alert.detectionConfidence * 100).toStringAsFixed(1)}%',
                  ),
                  _buildInfoRow(
                    'Sensors Triggered',
                    _formatSensors(alert.pirSensorsTriggered),
                  ),

                  const SizedBox(height: 24),
                  _buildSectionTitle(context, 'System Status'),
                  _buildInfoRow('Network', alert.networkStatus.toUpperCase()),
                  _buildInfoRow('Correlated', alert.correlated ? 'Yes' : 'No'),
                  _buildInfoRow('Alert ID', '#${alert.id}'),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildHeader(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          '${alert.alertType.toUpperCase()} DETECTED',
          style: Theme.of(context).textTheme.headlineSmall?.copyWith(
            color: Colors.red,
            fontWeight: FontWeight.bold,
          ),
        ),
        const SizedBox(height: 8),
        Text(
          alert.timestamp,
          style: Theme.of(
            context,
          ).textTheme.titleMedium?.copyWith(color: Colors.grey[600]),
        ),
      ],
    );
  }

  Widget _buildSectionTitle(BuildContext context, String title) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 8.0),
      child: Text(
        title,
        style: Theme.of(
          context,
        ).textTheme.titleMedium?.copyWith(fontWeight: FontWeight.bold),
      ),
    );
  }

  Widget _buildInfoRow(String label, String value) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4.0),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(label, style: const TextStyle(color: Colors.grey)),
          Text(value, style: const TextStyle(fontWeight: FontWeight.w500)),
        ],
      ),
    );
  }

  String _formatSensors(Map<String, dynamic> sensors) {
    final triggered = sensors.entries
        .where((e) => e.value == true)
        .map((e) => e.key.capitalize())
        .join(', ');
    return triggered.isEmpty ? 'None' : triggered;
  }
}

extension StringExtension on String {
  String capitalize() {
    return "${this[0].toUpperCase()}${substring(1)}";
  }
}
