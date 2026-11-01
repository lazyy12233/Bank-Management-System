#include <winsock2.h>
#include <windows.h>
#include "httplib.h"
#include <climits>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <algorithm>

using namespace std;
using namespace httplib;

struct Account {
    string number, name, phone, address;
    long long balance;
};

struct Transaction {
    string id, account, type, date;
    long long amount;
};

const string ACCOUNT_FILE = "data/accounts.txt";
const string TRANSACTION_FILE = "data/transactions.txt";

vector<Account> accounts;
vector<Transaction> transactions;

string money(long long paise) {
    stringstream ss;
    ss << fixed << setprecision(2) << paise / 100.0;
    return ss.str();
}

string jsonEscape(const string& s) {
    string result;
    for (char c : s) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else result += c;
    }
    return result;
}

string now() {
    time_t t = time(nullptr);
    string s = ctime(&t);
    if (!s.empty() && s.back() == '\n') s.pop_back();
    return s;
}

bool validText(const string& s) {
    return !s.empty() &&
        s.find('|') == string::npos &&
        s.find('\n') == string::npos &&
        s.find('\r') == string::npos;
}

void saveAccounts() {
    ofstream file(ACCOUNT_FILE + ".tmp");
    for (const auto& a : accounts) {
        file << a.number << "|" << a.name << "|" << a.phone
             << "|" << a.address << "|" << a.balance << "\n";
    }
    file.close();
    MoveFileExA((ACCOUNT_FILE + ".tmp").c_str(),
                ACCOUNT_FILE.c_str(),
                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
}

void saveTransactions() {
    ofstream file(TRANSACTION_FILE + ".tmp");
    for (const auto& t : transactions) {
        file << t.id << "|" << t.account << "|" << t.type
             << "|" << t.amount << "|" << t.date << "\n";
    }
    file.close();
    MoveFileExA((TRANSACTION_FILE + ".tmp").c_str(),
                TRANSACTION_FILE.c_str(),
                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
}

void loadData() {
    accounts.clear();
    transactions.clear();

    ifstream af(ACCOUNT_FILE);
    string line;
    while (getline(af, line)) {
        stringstream ss(line);
        Account a;
        string balance;
        if (getline(ss, a.number, '|') &&
            getline(ss, a.name, '|') &&
            getline(ss, a.phone, '|') &&
            getline(ss, a.address, '|') &&
            getline(ss, balance)) {
            try {
                a.balance = stoll(balance);
                accounts.push_back(a);
            } catch (...) {}
        }
    }

    ifstream tf(TRANSACTION_FILE);
    while (getline(tf, line)) {
        stringstream ss(line);
        Transaction t;
        string amount;
        if (getline(ss, t.id, '|') &&
            getline(ss, t.account, '|') &&
            getline(ss, t.type, '|') &&
            getline(ss, amount, '|') &&
            getline(ss, t.date)) {
            try {
                t.amount = stoll(amount);
                transactions.push_back(t);
            } catch (...) {}
        }
    }
}

Account* findAccount(const string& number) {
    for (auto& a : accounts) {
        if (a.number == number) return &a;
    }
    return nullptr;
}

void addTransaction(const string& account, const string& type,
                    long long amount) {
    Transaction t;
    t.id = to_string(time(nullptr)) + "-" +
           to_string(transactions.size() + 1);
    t.account = account;
    t.type = type;
    t.amount = amount;
    t.date = now();
    transactions.push_back(t);
    saveTransactions();
}

void sendMessage(Response& res, bool success, const string& message) {
    res.set_content(
        string("{\"success\":") + (success ? "true" : "false") +
        ",\"message\":\"" + jsonEscape(message) + "\"}",
        "application/json"
    );
}

int main() {
    loadData();

    Server server;

    // Website files
    server.set_mount_point("/", "./frontend");

    // Dashboard information
    server.Get("/api/dashboard", [](const Request&, Response& res) {
        long long total = 0;
        for (const auto& a : accounts) total += a.balance;

        string json = "{\"customers\":" + to_string(accounts.size()) +
            ",\"totalBalance\":\"" + money(total) +
            "\",\"transactions\":" + to_string(transactions.size()) + "}";

        res.set_content(json, "application/json");
    });

    // View all accounts
    server.Get("/api/accounts", [](const Request&, Response& res) {
        string json = "[";
        for (size_t i = 0; i < accounts.size(); i++) {
            const auto& a = accounts[i];
            if (i) json += ",";
            json += "{\"number\":\"" + jsonEscape(a.number) +
                "\",\"name\":\"" + jsonEscape(a.name) +
                "\",\"phone\":\"" + jsonEscape(a.phone) +
                "\",\"address\":\"" + jsonEscape(a.address) +
                "\",\"balance\":\"" + money(a.balance) + "\"}";
        }
        json += "]";
        res.set_content(json, "application/json");
    });

    // Create account
    server.Post("/api/create-account", [](const Request& req, Response& res) {
        if (!req.has_param("number") || !req.has_param("name") ||
            !req.has_param("phone") || !req.has_param("address") ||
            !req.has_param("amount")) {
            sendMessage(res, false, "Please fill in all fields.");
            return;
        }

        string number = req.get_param_value("number");
        string name = req.get_param_value("name");
        string phone = req.get_param_value("phone");
        string address = req.get_param_value("address");
        string amount = req.get_param_value("amount");

        if (!validText(number) || !validText(name) ||
            !validText(phone) || !validText(address)) {
            sendMessage(res, false, "Invalid characters in the fields.");
            return;
        }

        if (findAccount(number)) {
            sendMessage(res, false, "This account number already exists.");
            return;
        }

        try {
            double rupees = stod(amount);
            if (!isfinite(rupees) || rupees < 0 || rupees > 1000000000) {
                sendMessage(res, false, "Enter a valid initial deposit.");
                return;
            }

            Account a;
            a.number = number;
            a.name = name;
            a.phone = phone;
            a.address = address;
            a.balance = llround(rupees * 100);

            accounts.push_back(a);
            saveAccounts();

            if (a.balance > 0)
                addTransaction(number, "Initial Deposit", a.balance);

            sendMessage(res, true, "Bank account created successfully!");
        } catch (...) {
            sendMessage(res, false, "Please enter a valid deposit amount.");
        }
    });

    // Search account
    server.Get("/api/search", [](const Request& req, Response& res) {
        if (!req.has_param("number")) {
            sendMessage(res, false, "Enter an account number.");
            return;
        }

        Account* a = findAccount(req.get_param_value("number"));
        if (!a) {
            sendMessage(res, false, "Account not found.");
            return;
        }

        string json = "{\"success\":true,\"number\":\"" +
            jsonEscape(a->number) + "\",\"name\":\"" +
            jsonEscape(a->name) + "\",\"phone\":\"" +
            jsonEscape(a->phone) + "\",\"address\":\"" +
            jsonEscape(a->address) + "\",\"balance\":\"" +
            money(a->balance) + "\"}";

        res.set_content(json, "application/json");
    });

    // Deposit money
    server.Post("/api/deposit", [](const Request& req, Response& res) {
        if (!req.has_param("number") || !req.has_param("amount")) {
            sendMessage(res, false, "Enter account number and amount.");
            return;
        }

        Account* a = findAccount(req.get_param_value("number"));
        if (!a) {
            sendMessage(res, false, "Account not found.");
            return;
        }

        try {
            double value = stod(req.get_param_value("amount"));
            if (!isfinite(value) || value <= 0 || value > 1000000000) {
                sendMessage(res, false, "Enter a valid positive amount.");
                return;
            }

            long long amount = llround(value * 100);
            if (amount <= 0 || a->balance > LLONG_MAX - amount) {
                sendMessage(res, false, "Invalid amount.");
                return;
            }

            a->balance += amount;
            saveAccounts();
            addTransaction(a->number, "Deposit", amount);
            sendMessage(res, true, "Money deposited successfully!");
        } catch (...) {
            sendMessage(res, false, "Enter a valid amount.");
        }
    });

    // Withdraw money
    server.Post("/api/withdraw", [](const Request& req, Response& res) {
        if (!req.has_param("number") || !req.has_param("amount")) {
            sendMessage(res, false, "Enter account number and amount.");
            return;
        }

        Account* a = findAccount(req.get_param_value("number"));
        if (!a) {
            sendMessage(res, false, "Account not found.");
            return;
        }

        try {
            double value = stod(req.get_param_value("amount"));
            if (!isfinite(value) || value <= 0 || value > 1000000000) {
                sendMessage(res, false, "Enter a valid positive amount.");
                return;
            }

            long long amount = llround(value * 100);
            if (amount <= 0) {
                sendMessage(res, false, "Invalid amount.");
                return;
            }

            if (amount > a->balance) {
                sendMessage(res, false, "Insufficient account balance.");
                return;
            }

            a->balance -= amount;
            saveAccounts();
            addTransaction(a->number, "Withdrawal", amount);
            sendMessage(res, true, "Money withdrawn successfully!");
        } catch (...) {
            sendMessage(res, false, "Enter a valid amount.");
        }
    });

    // Transfer money
    server.Post("/api/transfer", [](const Request& req, Response& res) {
        if (!req.has_param("from") || !req.has_param("to") ||
            !req.has_param("amount")) {
            sendMessage(res, false, "Fill in all transfer fields.");
            return;
        }

        string from = req.get_param_value("from");
        string to = req.get_param_value("to");

        Account* sender = findAccount(from);
        Account* receiver = findAccount(to);

        if (!sender || !receiver) {
            sendMessage(res, false, "One or both accounts were not found.");
            return;
        }

        if (from == to) {
            sendMessage(res, false, "Choose two different accounts.");
            return;
        }

        try {
            double value = stod(req.get_param_value("amount"));
            if (!isfinite(value) || value <= 0 || value > 1000000000) {
                sendMessage(res, false, "Enter a valid positive amount.");
                return;
            }

            long long amount = llround(value * 100);
            if (amount <= 0 || amount > sender->balance) {
                sendMessage(res, false, "Insufficient balance or invalid amount.");
                return;
            }

            if (receiver->balance > LLONG_MAX - amount) {
                sendMessage(res, false, "Transfer amount is too large.");
                return;
            }

            sender->balance -= amount;
            receiver->balance += amount;
            saveAccounts();

            addTransaction(from, "Transfer Sent to " + to, amount);
            addTransaction(to, "Transfer Received from " + from, amount);

            sendMessage(res, true, "Money transferred successfully!");
        } catch (...) {
            sendMessage(res, false, "Enter a valid amount.");
        }
    });

    // Transaction history
    server.Get("/api/transactions", [](const Request& req, Response& res) {
        string filter = "";
        if (req.has_param("number"))
            filter = req.get_param_value("number");

        string json = "[";
        bool first = true;

        for (const auto& t : transactions) {
            if (!filter.empty() && t.account != filter) continue;

            if (!first) json += ",";
            first = false;

            json += "{\"id\":\"" + jsonEscape(t.id) +
                "\",\"account\":\"" + jsonEscape(t.account) +
                "\",\"type\":\"" + jsonEscape(t.type) +
                "\",\"amount\":\"" + money(t.amount) +
                "\",\"date\":\"" + jsonEscape(t.date) + "\"}";
        }

        json += "]";
        res.set_content(json, "application/json");
    });

    cout << "Bank Management System is running!\n";
    cout << "Open http://localhost:8081 in your browser\n";

    if (!server.listen("localhost", 8081)) {
        cerr << "Could not start server. Port 8081 may already be in use.\n";
        return 1;
    }

    return 0;
}
