import 'dart:io';
import 'dart:convert';

Future<String> sendToEngine(String expression) async {
  try {
    final socket = await Socket.connect('127.0.0.1', 8080);

    socket.write(expression);
    socket.write('\n');
    await socket.flush();

    final response = await utf8.decoder.bind(socket).first;

    socket.close();
    return response.trim();
  } catch (e) {
    return 'ERR';
  }
}
