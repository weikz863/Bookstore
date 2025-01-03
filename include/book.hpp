#pragma once

#ifndef BOOK_HPP_
#define BOOK_HPP_

#include <compare>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "file.hpp"

struct BookInfo : BasicFileStorage {
  static constexpr int NAMESIZE = 61;
  char ISBN[NAMESIZE], name[NAMESIZE], author[NAMESIZE], keywords[NAMESIZE];
  double price;
  int quantity;
  BookInfo() : BookInfo("") {};
  BookInfo(const string &s) : name(""), author(""), keywords(""),
      price(0.0), quantity(0) {
    strcpy(ISBN, s.c_str());
  }
  void print() const {
    std::cout << ISBN << '\t' << name << '\t' << author << '\t' << 
        std::fixed << std::setprecision(2) << price << '\t' << quantity << '\n';
  }
  std::weak_ordering operator <=> (const BookInfo &x) const {
    int ret = strcmp(ISBN, x.ISBN);
    if (ret < 0) return std::weak_ordering::less;
    else if (ret == 0) return std::weak_ordering::equivalent;
    else return std::weak_ordering::greater;
  }
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
  char index[STRLEN];
  int value;
  Queryable() : index(""), value(0) {};
  Queryable(bool x) {
    index[0] = (x ? 0x7f : 0x01);
    index[1] = '\0';
  }
  Queryable(const string &s, int x) : value(x) {
    strcpy(index, s.c_str());
  }
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
  ID() : ID(0) {};
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

vector<string> parse(const char * const keyword);

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
    PRICE = 4 * BookInfo::NAMESIZE,
    QUANTITY = 4 * BookInfo::NAMESIZE + sizeof(double),
  };
  BookManager();
  bool show(PosType, const string &);
  bool select(const string&);
  double buy(const string&, int);
  bool modify(PosType x, const string &s);
};

#endif