import 'package:flutter/material.dart';
import 'comms/socket_client.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'SciCalc',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
      ),
      home: const MyHomePage(title: 'SciCalc'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  String display = "0";
  bool showTrig = false;

  void addChar(String c) {
    setState(() {
      bool isOp(String x) => x == '+' || x == '-' || x == '*' || x == '/'||x=='^';

      final lastChar = display.isNotEmpty ? display[display.length - 1] : '';
      
      if (display == "0") {
        if (c == '-') {
          display = c;
          return;
        }
        if (isOp(c)) return;
        display = c;
        return;
      }
      
      if (isOp(lastChar) && isOp(c)) {
        display = display.substring(0, display.length - 1) + c;
        return;
      }
      display += c;
    });
  }

  void backspace() {
    setState(() {
      if (display.length <= 1) {
        display = "0";
      } else {
        display = display.substring(0, display.length - 1);
      }
    });
  }

  Widget buildKey(String c, {int flex = 1, Color? bg, Color? fg}) {
    return Expanded(
      flex: flex,
      child: Container(
        margin: const EdgeInsets.all(6), 
        child: ElevatedButton(
          onPressed: () {
            if (c == "⌫") {
              backspace();
            } else if (c == "C") {
              setState(() => display = "0");
            } else if (c == "=") {
              evaluate();
            } else {
              addChar(c);
            }
          },
          style: ElevatedButton.styleFrom(
            backgroundColor: bg ?? const Color.fromARGB(255, 104, 104, 104),
            foregroundColor: fg ?? Colors.white,
            padding: EdgeInsets.zero,
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(100),
            ),
          ),
          child: Text(
            c,
            style: const TextStyle(fontSize: 32, fontWeight: FontWeight.w600),
          ),
        ),
      ),
    );
  }

  Widget circleKeys(String c) {
    return SizedBox(
      width: 48,
      height: 48,
      child: ElevatedButton(
        onPressed: () {
          if (c == "⌫") {
            backspace();
          } else if (c == "C") {
            setState(() => display = "0");
          } else {
            addChar(c);
          }
        },
        style: ElevatedButton.styleFrom(
          backgroundColor: const Color.fromARGB(255, 104, 104, 104),
          foregroundColor: Colors.white,
          padding: EdgeInsets.zero,
          shape: const CircleBorder(),
        ),
        child: Text(c, style: const TextStyle(fontSize: 16)),
      ),
    );
  }

  Widget trigRow() {
    return AnimatedSize(
      duration: const Duration(milliseconds: 200),
      curve: Curves.easeInOut,
      child: showTrig
          ? Padding(
              padding: const EdgeInsets.only(bottom: 12),
              child: Column(
                children: [
                  Row(
                    mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                    children: [
                      trigKey("sin("),
                      trigKey("cos("),
                      trigKey("tan("),
                      trigKey("ln("),
                    ],
                  ),
                  const SizedBox(height: 8),
                  Row(
                    mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                    children: [
                      trigKey("arcsin("),
                      trigKey("arccos("),
                      trigKey("arctan("),
                    ],
                  ),
                ],
              ),
            )
          : const SizedBox.shrink(),
    );
  }

  Widget trigKey(String c) {
    return SizedBox(
      width: 82,
      height: 54,
      child: ElevatedButton(
        onPressed: () => addChar(c),
        style: ElevatedButton.styleFrom(
          backgroundColor: const Color.fromARGB(255, 105, 106, 105),
          foregroundColor: Colors.white,
        ),
        child: Text(
          c,
          style: const TextStyle(fontSize: 16, fontWeight: FontWeight.w600),
        ),
      ),
    );
  }

  void evaluate() async {
    final result = await sendToEngine(display);
    setState(() {
      display = result;
    });
  }

  @override
  Widget build(BuildContext context) {
    const darkGrey = Color.fromARGB(255, 56, 55, 55);
    const lightGrey = Color.fromARGB(255, 104, 104, 104);
    const green = Color.fromARGB(255, 0, 255, 17);

    return Scaffold(
      backgroundColor: Colors.black,
      appBar: AppBar(
        centerTitle: true,
        backgroundColor: Colors.grey,
        title: Text(
          widget.title,
          style: const TextStyle(
            fontSize: 32,
            fontWeight: FontWeight.w600,
            letterSpacing: 1.2,
            color: Colors.white,
          ),
        ),
      ),
      body: SafeArea(
        child: Column(
          children: [
            Container(
              padding: const EdgeInsets.all(20),
              alignment: Alignment.bottomRight,
              child: Text(
                display,
                style: const TextStyle(
                  fontSize: 42,
                  fontWeight: FontWeight.w500,
                  color: Colors.white,
                ),
              ),
            ),

            IconButton(
              onPressed: () {
                setState(() {
                  showTrig = !showTrig;
                });
              },
              icon: const Icon(Icons.functions, color: Colors.grey),
            ),

            trigRow(),
            Padding(
              padding: const EdgeInsets.symmetric(horizontal: 12),
              child: Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  circleKeys("C"),
                  circleKeys("("),
                  circleKeys(")"),
                  circleKeys("/"),
                  circleKeys("^"),
                  circleKeys("⌫"),
                ],
              ),
            ),
            Expanded(
              child: Padding(
                padding: const EdgeInsets.all(6),
                child: Column(
                  children: [
                    Expanded(
                      child: Row(
                        crossAxisAlignment: CrossAxisAlignment.stretch,
                        children: [
                          buildKey("7", bg: darkGrey),
                          buildKey("8", bg: darkGrey),
                          buildKey("9", bg: darkGrey),
                          buildKey("*", bg: lightGrey),
                        ],
                      ),
                    ),
                    Expanded(
                      child: Row(
                        crossAxisAlignment: CrossAxisAlignment.stretch,
                        children: [
                          buildKey("4", bg: darkGrey),
                          buildKey("5", bg: darkGrey),
                          buildKey("6", bg: darkGrey),
                          buildKey("-", bg: lightGrey),
                        ],
                      ),
                    ),
                    Expanded(
                      child: Row(
                        crossAxisAlignment: CrossAxisAlignment.stretch,
                        children: [
                          buildKey("1", bg: darkGrey),
                          buildKey("2", bg: darkGrey),
                          buildKey("3", bg: darkGrey),
                          buildKey("+", bg: lightGrey),
                        ],
                      ),
                    ),
                    Expanded(
                      child: Row(
                        crossAxisAlignment: CrossAxisAlignment.stretch,
                        children: [
                          buildKey("0", bg: darkGrey),
                          buildKey(".", bg: lightGrey),
                          buildKey("=", flex: 2, bg: green), 
                        ],
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}