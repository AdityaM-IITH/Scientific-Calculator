# Scientific Calculator

A scientific calculator application consisting of:
- a Flutter-based frontend (UI), and
- a C-based backend computation engine developed and tested in Linux/Termux.

The project separates user interface logic from computation logic and connects them using socket-based communication.

---

## Project Structure

```text
Scientific-Calculator/
├── frontend/          # Flutter UI
├── backend/           # C backend (Linux/Termux)
│   └── main.c
├── README.md
└── .gitignore
```


---

## Frontend (Flutter UI)

The frontend handles:
- Calculator layout and UI
- User input handling
- Displaying results
- Sending expressions to the backend

### Tech Stack
- Flutter
- Dart

### Run Frontend

```bash
cd frontend
flutter pub get
flutter run
