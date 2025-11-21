import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../models/device.dart';
import '../providers/device_provider.dart';

class DeviceDetailScreen extends StatelessWidget {
  final Device device;
  
  const DeviceDetailScreen({super.key, required this.device});

  @override
  Widget build(BuildContext context) {
    final deviceProvider = Provider.of<DeviceProvider>(context);
    
    return Scaffold(
      appBar: AppBar(
        title: Text(device.name),
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
                    Text(
                      'Status: ${device.status.displayName}',
                      style: Theme.of(context).textTheme.titleLarge,
                    ),
                    const SizedBox(height: 8),
                    Text('Device ID: ${device.deviceId}'),
                    if (device.location != null)
                      Text('Location: ${device.location}'),
                    if (device.firmwareVersion != null)
                      Text('Firmware: ${device.firmwareVersion}'),
                    if (device.batteryLevel != null)
                      Text('Battery: ${device.batteryLevel!.toStringAsFixed(0)}%'),
                    if (device.gsmSignalStrength != null)
                      Text('GSM Signal: ${device.gsmSignalStrength}/31'),
                  ],
                ),
              ),
            ),
            const SizedBox(height: 16),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                ElevatedButton.icon(
                  onPressed: () => deviceProvider.armDevice(device.id),
                  icon: const Icon(Icons.lock),
                  label: const Text('Arm'),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.green,
                    foregroundColor: Colors.white,
                  ),
                ),
                ElevatedButton.icon(
                  onPressed: () => deviceProvider.disarmDevice(device.id),
                  icon: const Icon(Icons.lock_open),
                  label: const Text('Disarm'),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.red,
                    foregroundColor: Colors.white,
                  ),
                ),
              ],
            ),
            const SizedBox(height: 16),
            if (device.sensors.isNotEmpty) ...[
              Text(
                'Sensors',
                style: Theme.of(context).textTheme.titleLarge,
              ),
              const SizedBox(height: 8),
              ...device.sensors.map((sensor) => Card(
                child: ListTile(
                  title: Text(sensor.name),
                  subtitle: Text('${sensor.sensorType.name} - ${sensor.location ?? "Unknown"}'),
                  trailing: sensor.isActive
                      ? const Icon(Icons.check, color: Colors.green)
                      : const Icon(Icons.cancel, color: Colors.grey),
                ),
              )),
            ],
          ],
        ),
      ),
    );
  }
}

