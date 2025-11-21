import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:intl/intl.dart';
import '../models/alert.dart';
import '../providers/alert_provider.dart';

class AlertDetailScreen extends StatelessWidget {
  final Alert alert;
  
  const AlertDetailScreen({super.key, required this.alert});

  @override
  Widget build(BuildContext context) {
    final alertProvider = Provider.of<AlertProvider>(context);
    
    return Scaffold(
      appBar: AppBar(
        title: Text(alert.alertType.displayName),
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Card(
              child: Padding(
                padding: const EdgeInsets.all(16),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Row(
                      children: [
                        Text(
                          alert.alertType.emoji,
                          style: const TextStyle(fontSize: 48),
                        ),
                        const SizedBox(width: 16),
                        Expanded(
                          child: Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              Text(
                                alert.alertType.displayName,
                                style: Theme.of(context).textTheme.headlineSmall,
                              ),
                              Text('Status: ${alert.status.displayName}'),
                            ],
                          ),
                        ),
                      ],
                    ),
                    const Divider(),
                    Text('Message: ${alert.message}'),
                    const SizedBox(height: 8),
                    Text('Severity: ${alert.severity}'),
                    const SizedBox(height: 8),
                    Text(
                      'Created: ${DateFormat('MMM d, y HH:mm:ss').format(alert.createdAt)}',
                    ),
                    if (alert.acknowledgedAt != null)
                      Text(
                        'Acknowledged: ${DateFormat('MMM d, y HH:mm:ss').format(alert.acknowledgedAt!)}',
                      ),
                    if (alert.resolvedAt != null)
                      Text(
                        'Resolved: ${DateFormat('MMM d, y HH:mm:ss').format(alert.resolvedAt!)}',
                      ),
                  ],
                ),
              ),
            ),
            const SizedBox(height: 16),
            if (alert.status == AlertStatus.pending)
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                children: [
                  ElevatedButton.icon(
                    onPressed: () async {
                      await alertProvider.acknowledgeAlert(alert.id);
                      if (!context.mounted) return;
                      Navigator.pop(context);
                    },
                    icon: const Icon(Icons.check),
                    label: const Text('Acknowledge'),
                  ),
                  ElevatedButton.icon(
                    onPressed: () async {
                      await alertProvider.resolveAlert(alert.id);
                      if (!context.mounted) return;
                      Navigator.pop(context);
                    },
                    icon: const Icon(Icons.done),
                    label: const Text('Resolve'),
                  ),
                ],
              ),
          ],
        ),
      ),
    );
  }
}

