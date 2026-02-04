import 'dart:io';
import 'dart:convert';

String _serverIp = "127.0.0.1";
const int _serverPort = 8080;

void setServerIp(String ip) {
  _serverIp = ip.trim();
}

Future<String> sendToEngine(String expression) async {
  try {
    final socket = await Socket.connect(_serverIp, _serverPort);

    socket.write(expression + "\n");
    await socket.flush();

    final response = await utf8.decoder.bind(socket).first;

    socket.close();
    return response.trim();
  } catch (_) {
    return "ERR";
  }
}
