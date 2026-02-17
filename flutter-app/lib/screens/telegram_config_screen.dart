import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../services/telegram_service.dart';

class TelegramConfigScreen extends StatefulWidget {
  const TelegramConfigScreen({super.key});

  @override
  State<TelegramConfigScreen> createState() => _TelegramConfigScreenState();
}

class _TelegramConfigScreenState extends State<TelegramConfigScreen> {
  final _formKey = GlobalKey<FormState>();
  final _tokenController = TextEditingController();
  final _chatIdController = TextEditingController();
  bool _active = true;
  bool _isLoading = true;

  @override
  void initState() {
    super.initState();
    _loadConfig();
  }

  Future<void> _loadConfig() async {
    final config = await context.read<TelegramService>().getConfig();
    if (config != null && mounted) {
      setState(() {
        // Do NOT put bot_token here: API returns it MASKED (e.g. **********xyz789).
        // Putting it in the field and saving would overwrite the real token in DB and break Telegram.
        _tokenController.text = '';
        _chatIdController.text = config['chat_id'] ?? '';
        _active = config['active'] ?? true;
        _isLoading = false;
      });
    } else {
      setState(() {
        _isLoading = false;
      });
    }
  }

  Future<void> _save() async {
    if (_formKey.currentState!.validate()) {
      // Trim to avoid 404 from trailing newline/space when pasting from BotFather
      final success = await context.read<TelegramService>().saveConfig(
        _tokenController.text.trim(),
        _chatIdController.text.trim(),
        _active,
      );

      if (success && mounted) {
        ScaffoldMessenger.of(
          context,
        ).showSnackBar(const SnackBar(content: Text('Configuration saved!')));
      }
    }
  }

  Future<void> _test() async {
    final success = await context.read<TelegramService>().testConnection();
    if (mounted) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text(
            success ? 'Connection Successful!' : 'Connection Failed',
          ),
          backgroundColor: success ? Colors.green : Colors.red,
        ),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Telegram Config')),
      body: Consumer<TelegramService>(
        builder: (context, telegramService, _) {
          if (_isLoading) {
            return const Center(child: CircularProgressIndicator());
          }

          return SingleChildScrollView(
            padding: const EdgeInsets.all(16.0),
            child: Form(
              key: _formKey,
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  if (telegramService.error != null)
                    Padding(
                      padding: const EdgeInsets.only(bottom: 16),
                      child: Text(
                        telegramService.error!,
                        style: const TextStyle(color: Colors.red),
                      ),
                    ),

                  TextFormField(
                    controller: _tokenController,
                    decoration: const InputDecoration(
                      labelText: 'Bot Token',
                      border: OutlineInputBorder(),
                      helperText: 'Leave blank to keep current token. Enter new only to change. Format: 123456789:AAH...',
                    ),
                    validator: (v) => null, // optional: leave blank to keep existing
                  ),
                  const SizedBox(height: 16),

                  TextFormField(
                    controller: _chatIdController,
                    decoration: const InputDecoration(
                      labelText: 'Chat ID',
                      border: OutlineInputBorder(),
                      helperText: 'Numeric ID (e.g. 123456789 or -100... for groups). Send /start to bot, then get from api.telegram.org/bot<TOKEN>/getUpdates',
                    ),
                    validator: (v) => v!.trim().isEmpty ? 'Required' : null,
                  ),
                  const SizedBox(height: 16),

                  SwitchListTile(
                    title: const Text('Active'),
                    subtitle: const Text('Enable Telegram notifications'),
                    value: _active,
                    onChanged: (v) => setState(() => _active = v),
                  ),
                  const SizedBox(height: 24),

                  ElevatedButton.icon(
                    onPressed: telegramService.isLoading ? null : _save,
                    icon: const Icon(Icons.save),
                    label: const Text('Save Configuration'),
                    style: ElevatedButton.styleFrom(
                      padding: const EdgeInsets.symmetric(vertical: 16),
                    ),
                  ),
                  const SizedBox(height: 16),

                  OutlinedButton.icon(
                    onPressed: telegramService.isLoading ? null : _test,
                    icon: const Icon(Icons.send),
                    label: const Text('Test Connection'),
                  ),
                ],
              ),
            ),
          );
        },
      ),
    );
  }
}
