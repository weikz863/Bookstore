#include "book.hpp"
#include "file.hpp"
#include "user.hpp"
#include "log.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>

using std::cin;
using std::cout;
using std::endl;

extern UserManager user_manager;
extern BookManager book_manager;
extern LogManager log_manager;

constexpr const char *fail = "Invalid\n";

int main(void) {
  std::string s;
  while (cin) {
    std::getline(cin, s);
    std::stringstream ss(std::move(s));
    ss >> s;
    if (s.empty()) continue;
    if (s == "quit" || s == "exit") {
      ss >> s;
      if (!ss) return 0;
      else cout << fail;
    } else if (s == "su") {
      string id(""), passwd(""), t("");
      ss >> id >> passwd >> t;
      if (id == "" || t != "") {
        cout << fail;
      } else {
        if (user_manager.su(id, passwd)) {
          continue;
        } else {
          cout << fail;
        }
      }
    } else if (s == "logout") {
      ss >> s;
      if (!ss && user_manager.logout()) {
        continue;
      } else {
        cout << fail;
      }
    } else if (s == "register") {
      string id(""), passwd(""), username(""), t("");
      ss >> id >> passwd >> username >> t;
      if (username != "" && t == "" && 
          user_manager.add_user(UserData(id, passwd, username, 1))) {
        continue;
      } else {
        cout << fail;
      }
    } else if (s == "passwd") {
      string id(""), curp(""), newp(""), t("");
      ss >> id >> curp >> newp >> t;
      if (curp == "" || t != "") {
        cout << fail;
      } else {
        if (newp == "" && user_manager.passwd(id, "", curp) ||
            newp != "" && user_manager.passwd(id, curp, newp)) {
          continue;
        } else {
          cout << fail;
        }
      }
    } else if (s == "useradd") {
      string id(""), passwd(""), priv(""), name(""), t("");
      ss >> id >> passwd >> priv >> name >> t;
      if (name != "" && t == "") {
        if (priv == "1" && user_manager.add_user(UserData(id, passwd, name, 1)) ||
            priv == "3" && user_manager.add_user(UserData(id, passwd, name, 3))) {
          continue;
        } else {
          cout << fail;
        }
      } else {
        cout << fail;
      }
    } else if (s == "delete") {
      string id(""), t("");
      ss >> id >> t;
      if (id != "" && t == "" && user_manager.delet(id)) {
        continue;
      } else {
        cout << fail;
      }
    } else if (s == "show") {
      string req(""), t("");
      ss >> req >> t;
      if (t != "") {
        cout << fail;
      } else {
        if (req == "" && book_manager.show(BookManager::PosType::NONE, "") || 
            req.starts_with("-ISBN=") && book_manager.show(BookManager::PosType::ISBN, req.substr(6)) ||
            req.size() > 8 && req.starts_with("-name=\"") && req.ends_with('\"') && book_manager.show(BookManager::PosType::NAME, req.substr(7, req.size() - 8)) ||
            req.size() > 10 && req.starts_with("-author=\"") && req.ends_with('\"') && book_manager.show(BookManager::PosType::AUTHOR, req.substr(9, req.size() - 10)) ||
            req.size() > 11 && req.starts_with("-keyword=\"") && req.ends_with('\"') && book_manager.show(BookManager::PosType::KEYWORD, req.substr(10, req.size() - 11))) {
          continue;
        } else {
          cout << fail;
        }
      }
    } else if (s == "buy") {
      string isbn(""), t("");
      int quant = 0;
      ss >> isbn >> quant >> t; 
      double cost = 0;
      if (isbn != "" && t == "" && quant > 0 && (cost = book_manager.buy(isbn, quant)) != -1) {
        log_manager.ad(Finance(cost, 0));
      } else {
        cout << fail;
      }
    } else if (s == "select") {
      string isbn(""), t("");
      ss >> isbn >> t;
      if (isbn != "" && t == "" && book_manager.select(isbn)) {
        continue;
      } else {
        cout << fail;
      }
    } else if (s == "modify") {
      vector<BookManager::PosType> moded{};
      vector<string> qs{};
      do {
        qs.push_back("");
        ss >> qs.back();
      } while (qs.back() != "");
      qs.pop_back();
      if (qs.empty()) { // what?
        cout << fail;
        continue;
      }
      bool illegal = false;
      for (auto &str : qs) {
        if (str.starts_with("-ISBN=")) {
          if (std::find(moded.begin(), moded.end(), BookManager::PosType::ISBN) != moded.end()) {
            illegal = true;
            break;
          } else {
            moded.push_back(BookManager::PosType::ISBN);
            str = str.substr(6);
          }
        } else if (str.size() > 8 && str.starts_with("-name=\"") && str.ends_with('\"')) {
          if (std::find(moded.begin(), moded.end(), BookManager::PosType::NAME) != moded.end()) {
            illegal = true;
            break;
          } else {
            moded.push_back(BookManager::PosType::NAME);
            str = str.substr(7, str.size() - 8);
          }
        } else if (str.size() > 10 && str.starts_with("-author=\"") && str.ends_with('\"')) {
          if (std::find(moded.begin(), moded.end(), BookManager::PosType::AUTHOR) != moded.end()) {
            illegal = true;
            break;
          } else {
            moded.push_back(BookManager::PosType::AUTHOR);
            str = str.substr(9, str.size() - 10);
          }
        } else if (str.size() > 11 && str.starts_with("-keyword=\"") && str.ends_with('\"')) {
          if (std::find(moded.begin(), moded.end(), BookManager::PosType::KEYWORD) != moded.end()) {
            illegal = true;
            break;
          } else {
            moded.push_back(BookManager::PosType::KEYWORD);
            str = str.substr(10, str.size() - 11);
            auto t = parse(str.c_str());
            std::sort(t.begin(), t.end());
            if (std::unique(t.begin(), t.end()) != t.end()) {
              illegal = true;
              break;
            }
          }
        } else {
          illegal = true;
          break;
        }
      }
      if (illegal) {
        cout << fail;
        continue;
      }
      if (auto it = std::find(moded.begin(), moded.end(), BookManager::PosType::ISBN); it != moded.end()) {
        if (!book_manager.modify(BookManager::PosType::ISBN, qs[it - moded.begin()])) {
          cout << fail;
          continue;
        }
      }
      for (int i = 0; i < moded.size(); i++) {
        if (moded[i] == BookManager::PosType::ISBN) continue;
        book_manager.modify(moded[i], qs[i]);
      }
    } else if (s == "import") {
      string quantity(""), cost(""), t("");
      ss >> quantity >> cost >> t;
      if (cost != "" && t == "" && book_manager.modify(BookManager::PosType::QUANTITY, cost)) {
        std::stringstream tmpss(std::move(cost));
        double y;
        tmpss >> y;
        log_manager.ad(Finance(0, y));
      } else {
        cout << fail;
      }
    } else if (s == "show") {
      string finance(""), cnt(""), t("");
      ss >> finance >> cnt >> t;
      if (finance != "finance" || t != "") {
        cout << fail;
        continue;
      }
      if (cnt == "") {
        if (log_manager.show()) continue;
        else cout << fail;
      } else {
        std::stringstream tmpss(std::move(cnt));
        int d;
        tmpss >> d >> cnt;
        if (cnt == "" && log_manager.show(d)) {
          continue;
        } else {
          cout << fail;
        }
      }
    }
  }
  return 0;
}