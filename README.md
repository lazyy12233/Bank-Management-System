# Bank-Management-System

# 🏦 Bank Management System (BankSphere)

A web-based banking application built with **C++17** and a plain **HTML / CSS / JavaScript** frontend. The C++ program runs its own local web server using [cpp-httplib](https://github.com/yhirose/cpp-httplib), and the browser pages talk to it through a small JSON API. All data is saved in text files, so no database is needed.

Developed as an **Object Oriented Programming (OOPS) mini project** at the University Institute of Computing, Chandigarh University.

---

## ✨ Features

- **Create accounts** with account number, name, phone, address and opening deposit
- **View and search** accounts by account number
- **Deposit, withdraw and transfer** money between accounts
- **Transaction history**, newest first, with a unique ID and timestamp for every entry
- **Dashboard** showing total customers, total balance and a 7-day activity chart
- **Light and dark theme**
- **Automatic checks** that refuse duplicate account numbers, invalid or negative amounts, unknown accounts, insufficient balance and transfers to the same account

---

## 🧠 How It Works


- **Frontend:** 8 pages in `frontend/`. JavaScript sends requests with `fetch()` and shows the reply.
- **Backend:** `backend/main.cpp` defines 8 API routes. Records are loaded into memory at start-up (`vector<Account>`, `vector<Transaction>`) and saved after every change.
- **Exact money:** amounts are stored in **paise** as whole numbers (1 rupee = 100 paise), which avoids decimal rounding errors.
- **Safe saving:** each save writes a temporary file first, then replaces the old file.

---

## 🔌 API Routes

| Method | Route | Purpose |
|---|---|---|
| GET | `/api/dashboard` | Totals |
| GET | `/api/accounts` | List all accounts |
| POST | `/api/create-account` | Create an account |
| GET | `/api/search?number=` | Find one account |
| POST | `/api/deposit` | Add money |
| POST | `/api/withdraw` | Take money out |
| POST | `/api/transfer` | Move money between accounts |
| GET | `/api/transactions` | Transaction history |

---

## 📁 Project Structure

---

## 🚀 Run (Windows)

1. Install [MSYS2](https://www.msys2.org) and the UCRT64 `g++` compiler.
2. Double-click **`Start Bank System.bat`** (it compiles and starts the server).
3. Open **http://localhost:8081** in your browser.

Manual build:


---

## ⚠️ Limitations

- No login or user roles
- Windows only (uses `winsock2.h` and `MoveFileExA`)
- Text-file storage, so not suitable for large data
- No locking, so it is built for one user at a time
- Written with structs and functions rather than user-defined classes

## 🔮 Future Work

Class-based redesign (Account, SavingsAccount, CurrentAccount, Bank), login, a database such as SQLite or MySQL, account statements and stricter input validation.

---

## 👥 Authors

**Somen** (25BCD10238) and **Emaan** (25BCD10252)
BCA (Data Science), 3rd Semester, Chandigarh University, UIC
Guide: Mrs. Shilpa Mahajan
