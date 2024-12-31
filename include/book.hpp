#pragma once

#ifndef BOOK_HPP_
#define BOOK_HPP_

#include "file.hpp"
#include "user.hpp"
#include <cassert>
#include <fstream>

extern UserManager user_manager;

struct BookInfo : BasicFileStorage {
  static constexpr int NAMESIZE = 61;
  char ISBN[NAMESIZE], name[NAMESIZE], author[NAMESIZE], keywords[NAMESIZE];
  double price;
  int quantity;
  int constexpr size() const {
    return 4 * NAMESIZE + sizeof(double) + sizeof(int);
  }
  void readfrom(fstream &x) {
    x.read(ISBN, NAMESIZE);
    x.read(name, NAMESIZE);
    x.read(author, NAMESIZE);
    x.read(keywords, NAMESIZE);
    x.read(reinterpret_cast<char*>(&price), sizeof(double));
    x.read(reinterpret_cast<char*>(&quantity), sizeof(int));
  }
  void writeto(fstream &x) const {
    x.write(ISBN, NAMESIZE);
    x.write(name, NAMESIZE);
    x.write(author, NAMESIZE);
    x.write(keywords, NAMESIZE);
    x.write(reinterpret_cast<const char*>(&price), sizeof(double));
    x.write(reinterpret_cast<const char*>(&quantity), sizeof(int));
  }
};

struct Queryable : BasicFileStorage {
  static constexpr int STRLEN = BookInfo::NAMESIZE;
  static constexpr int SIZE = STRLEN + sizeof(int);
  char index[STRLEN] = "";
  int value = 0;
  int constexpr size() const {
    return SIZE;
  }
  void readfrom(fstream &x) {
    x.read(index, STRLEN);
    x.read(reinterpret_cast<char*>(&value), sizeof(int));
  }
  void writeto(fstream &x) const {
    x.write(index, STRLEN);
    x.write(reinterpret_cast<const char*>(&value), sizeof(int));
  }
  /*
  Queryable& operator = (const Queryable &x) {
    if (&x != this) {
      strcpy(index, x.index);
      value = x.value;
    }
    return *this;
  }
  */
  bool operator < (const Queryable& x) const {
    int f = std::strcmp(index, x.index);
    if (f) return f < 0;
    else return value < x.value;
  }
  bool operator > (const Queryable& x) const {
    return x < *this;
  }
  bool operator <= (const Queryable& x) const {
    return !(x < *this);
  }
  bool operator >= (const Queryable& x) const {
    return !(*this < x);
  }
  bool operator == (const Queryable& x) const {
    return value == x.value && !std::strcmp(index, x.index);
  }
  bool operator != (const Queryable& x) const {
    return value != x.value || std::strcmp(index, x.index);
  }
};

struct ID : BasicFileStorage {
  int value;
  ID(int x) : value(x) {};
  int constexpr size() const {
    return sizeof(int);
  }
  void readfrom(fstream &x) {
    x.read(reinterpret_cast<char*>(&value), sizeof(int));
  }
  void writeto(fstream &x) const {
    x.write(reinterpret_cast<const char*>(&value), sizeof(int));
  }
};

struct BookManager {
  typedef BlockList<50, Queryable> database;
  Vector<BookInfo> books;
  Vector<ID> selected;
  database isbn, name, author, keyword;
  enum PosType {
    NONE = -1, 
    ISBN = 0, 
    NAME = BookInfo::NAMESIZE, 
    AUTHOR = 2 * BookInfo::NAMESIZE,
    KEYWORD = 3 * BookInfo::NAMESIZE,
  };
  bool show(PosType x, const string &s) {
    assert(s.size() <= (x == PosType::ISBN ? 20 : 60));
    if (user_manager.current_privilege() < 1) return false;
    if (s == "") return false;
    if (x == PosType::KEYWORD && s.find('|') != string::npos) return false;
  }
};

#endif