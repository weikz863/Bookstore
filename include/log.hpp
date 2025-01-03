#pragma once

#ifndef LOG_HPP_
#define LOG_HPP_

#include "file.hpp"
#include "user.hpp"
#include <fstream>
#include <iostream>

using std::cout;

extern UserManager user_manager;

struct Finance : BasicFileStorage {
  double in, out;
  Finance() : in(0.0), out(0.0) {};
  Finance(double x, double y) : in(x), out(y) {};
  void print() const {
    cout << "+ " << in << " - " << out << '\n';
  }
  Finance operator - (const Finance &f) const {
    return Finance(in - f.in, out - f.out);
  }
  Finance operator + (const Finance &f) const {
    return Finance(in + f.in, out + f.out);
  }
  constexpr int size() const {
    return 2 * sizeof(double);
  }
  void writeto(fstream &f) const {
    f.write(reinterpret_cast<const char*>(&in), sizeof(double));
    f.write(reinterpret_cast<const char*>(&out), sizeof(double));
  }
  void readfrom(fstream &f) {
    f.read(reinterpret_cast<char*>(&in), sizeof(double));
    f.read(reinterpret_cast<char*>(&out), sizeof(double));
  }
};

struct LogManager {
  Vector<Finance> sum;
  LogManager() : sum("sum") {
    sum.init();
    sum.push_back(Finance());
  }
  bool show() {
    if (user_manager.current_privilege() < 7) return false;
    sum.back().print();
    return true;
  }
  bool show(int last) {
    assert(last >= 0);
    if (user_manager.current_privilege() < 7) return false;
    if (last == 0) {
      cout << '\n';
      return true;
    }
    if (last > sum.size() - 1) return false;
    (sum.back() - sum[sum.size() - last]).print();
    return true;
  }
  void ad(Finance x) {
    sum.push_back(sum.back() + x);
  }
};

#endif