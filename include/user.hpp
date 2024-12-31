#include <algorithm>
#include <cassert>
#include <file.hpp>

struct UserData : BasicFileStorage {
  static int constexpr STRLEN = 31;
  static int constexpr SIZE = 3 * STRLEN + 2 * sizeof(int);
  char id[STRLEN], password[STRLEN], username[STRLEN];
  int privilege, login_cnt;
  UserData() = default;
  UserData(const string &s, int mode) : password("") {
    assert(s.size() <= 30);
    strcpy(id, s.c_str());
    assert(password[1] == '\0');
    password[0] = (mode ? 0x7f : 0x01);
  }
  constexpr int size() const {
    return SIZE;
  }
  void readfrom(fstream &x) {
    x.read(id, STRLEN);
    x.read(password, STRLEN);
    x.read(username, STRLEN);
    x.read(reinterpret_cast<char*>(&privilege), sizeof(int));
    x.read(reinterpret_cast<char*>(&login_cnt), sizeof(int));
  }
  void writeto(fstream &x) const {
    x.write(id, STRLEN);
    x.write(password, STRLEN);
    x.write(username, STRLEN);
    x.write(reinterpret_cast<const char*>(&privilege), sizeof(int));
    x.write(reinterpret_cast<const char*>(&login_cnt), sizeof(int));
  }
};
struct LoginUser : BasicFileStorage {
  static int constexpr STRLEN = 31;
  static int constexpr SIZE = STRLEN + sizeof(int);
  char id[STRLEN];
  int privilege;
  LoginUser() = default;
  LoginUser(const UserData &x) : privilege(x.privilege) {
    strcpy(id, x.id);
  }
  constexpr int size() const {
    return SIZE;
  }
  void readfrom(fstream &x) {
    x.read(id, STRLEN);
    x.read(reinterpret_cast<char*>(&privilege), sizeof(int));
  }
  void writeto(fstream &x) const {
    x.write(id, STRLEN);
    x.write(reinterpret_cast<const char*>(&privilege), sizeof(int));
  }
};
struct UserManager {
  typedef BlockList<40, UserData> db;
  Vector<LoginUser> login;
  db account;
  UserManager() : login("login"), account("account") {
    login.init();
    account.init();
  };
  bool su (const string &id, const string &password) {
    db::iterator ret = account.find_one(UserData(id, 0));
    if (ret.pos < 0) return false;
    UserData t;
    ret.read(t);
    if (t.id != id) return false;
    if (password == "") {
      if (login.empty() || login.back().privilege <= t.privilege) {
        return false;
      }
    } else {
      if (t.password != password) {
        return false;
      }
    }
    t.login_cnt++;
    ret.assign(t);
    login.push_back(LoginUser(t));
    return true;
  }
  bool logout() {
    if (login.empty()) return false;
    LoginUser t = login.back();
    login.pop_back();
    db::iterator ret = account.find_one(UserData(t.id, 0));
    UserData dat;
    ret.read(dat);
    dat.login_cnt--;
    ret.assign(dat);
  }
  bool add_user(const UserData &u) {
    if (login.empty()) {
      if (u.privilege > 1) return false;
    } else {
      if (u.privilege >= login.back().privilege) return false;
    }
    db::iterator ret = account.find_one(UserData(u.id, 0));
    if (ret.pos >= 0) {
      UserData t;
      ret.read(t);
      if (strcmp(t.id, u.id) == 0) return false;
    }
    account.insert(u);
  }
};