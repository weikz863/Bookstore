#pragma once

#ifndef USER_HPP_
#define USER_HPP_

#include <cstring>
#include <compare>
#include <algorithm>
#include <cassert>
#include <vector>
#include "file.hpp"
#include "book.hpp"

extern Vector<ID> & selected;

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
  bool operator == (const UserData &x) const {
    return strcmp(id, x.id) == 0;
  }
  std::weak_ordering operator <=> (const UserData &x) const {
    int t = strcmp(id, x.id);
    if (t < 0) return std::weak_ordering::less;
    else if (t == 0) return std::weak_ordering::equivalent;
    else return std::weak_ordering::greater;
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
  int current_privilege() {
    if (login.empty()) return 0;
    else return login.back().privilege;
  }
  bool su (const string &id, const string &password) {
    db::iterator ret = account.find_one(UserData(id, 0));
    if (ret.pos < 0) return false;
    UserData t;
    ret.read(t);
    if (t.id != id) return false;
    if (password == "") {
      if (current_privilege() <= t.privilege) {
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
    selected.push_back(-1);
    return true;
  }
  bool logout() {
    if (login.empty()) return false;
    LoginUser t = login.back();
    login.pop_back();
    selected.pop_back();
    db::iterator ret = account.find_one(UserData(t.id, 0));
    UserData dat;
    ret.read(dat);
    dat.login_cnt--;
    ret.assign(dat);
    return true;
  }
  bool add_user(const UserData &u) {
    if (login.empty()) {
      if (u.privilege > 1) return false;
    } else {
      if (u.privilege >= login.back().privilege) return false;
    }
    vector<UserData> ret = account.find(UserData(u.id, 0), UserData(u.id, 1));
    if (ret.size()) return false;
    account.insert(u);
    return true;
  }
  bool passwd(const string &userid, const string &cur, const string &newp) {
    assert(newp.size() <= 30);
    if (login.empty()) {
      return false;
    }
    db::iterator ret = account.find_one(UserData(userid, 0));
    if (ret.pos < 0) return false;
    UserData t;
    ret.read(t);
    if (t.id != userid) return false;
    if (cur == "") {
      if (login.back().privilege < 7) return false;
    } else {
      if (t.password != cur) return false;
    }
    strcpy(t.password, newp.c_str());
    ret.assign(t);
    return true;
  }
  bool delet(const string &userid) {
    if (current_privilege() < 7) {
      return false;
    }
    db::iterator ret = account.find_one(UserData(userid, 0));
    if (ret.pos < 0) return false;
    UserData t;
    ret.read(t);
    if (t.id != userid || t.login_cnt) return false;
    account.delet(t);
    return true;
  }
};

#endif