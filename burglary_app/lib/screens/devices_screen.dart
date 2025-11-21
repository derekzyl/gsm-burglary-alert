import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:intl/intl.dart';
import '../providers/device_provider.dart';
import '../models/device.dart';
import 'device_detail_screen.dart';

class DevicesScreen extends StatelessWidget {
  const DevicesScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Consumer<DeviceProvider>(
      builder: (context, provider, child) {
        if (provider.isLoading && provider.devices.isEmpty) {
          return const Center(child: CircularProgressIndicator());
        }
        
        if (provider.devices.isEmpty) {
          return Center(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                const Icon(Icons.devices, size: 64, color: Colors.grey),
                const SizedBox(height: 16),
                const Text('No devices found'),
                const SizedBox(height: 16),
                ElevatedButton(
                  onPressed: () {
                    // TODO: Show add device dialog
                  },
                  child: const Text('Add Device'),
                ),
              ],
            ),
          );
        }
        
        return RefreshIndicator(
          onRefresh: provider.loadDevices,
          child: ListView.builder(
            padding: const EdgeInsets.all(8),
            itemCount: provider.devices.length,
            itemBuilder: (context, index) {
              final device = provider.devices[index];
              return Card(
                margin: const EdgeInsets.symmetric(vertical: 4, horizontal: 8),
                child: ListTile(
                  leading: _getStatusIcon(device.status),
                  title: Text(device.name),
                  subtitle: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text('Location: ${device.location ?? "Unknown"}'),
                      if (device.lastSeen != null)
                        Text(
                          'Last seen: ${DateFormat('MMM d, y HH:mm').format(device.lastSeen!)}',
                          style: const TextStyle(fontSize: 12),
                        ),
                      if (device.batteryLevel != null)
                        Text('Battery: ${device.batteryLevel!.toStringAsFixed(0)}%'),
                    ],
                  ),
                  trailing: device.status == DeviceStatus.armed
                      ? IconButton(
                          icon: const Icon(Icons.lock_open, color: Colors.green),
                          onPressed: () => provider.disarmDevice(device.id),
                        )
                      : IconButton(
                          icon: const Icon(Icons.lock, color: Colors.red),
                          onPressed: () => provider.armDevice(device.id),
                        ),
                  onTap: () {
                    Navigator.push(
                      context,
                      MaterialPageRoute(
                        builder: (_) => DeviceDetailScreen(device: device),
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
  
  Widget _getStatusIcon(DeviceStatus status) {
    IconData icon;
    Color color;
    
    switch (status) {
      case DeviceStatus.online:
      case DeviceStatus.armed:
        icon = Icons.check_circle;
        color = Colors.green;
        break;
      case DeviceStatus.alarm:
        icon = Icons.warning;
        color = Colors.red;
        break;
      case DeviceStatus.offline:
      case DeviceStatus.disarmed:
        icon = Icons.cancel;
        color = Colors.grey;
        break;
      default:
        icon = Icons.help;
        color = Colors.orange;
    }
    
    return Icon(icon, color: color);
  }
}

