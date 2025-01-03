#include "book.hpp"
#include "user.hpp"
#include "log.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

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
      
    }
  }
  return 0;
}