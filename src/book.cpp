#include "book.hpp"
#include "file.hpp"
#include "user.hpp"
#include <algorithm>
#include <vector>

extern UserManager user_manager;

BookManager book_manager;

BookManager::BookManager() : books("books"), selected("selected"), 
      isbn("isbn"), name("name"), author("author"), keyword("keyword") {
  books.init();
  selected.init();
  isbn.init();
  name.init();
  author.init();
  keyword.init();
};
bool BookManager::show(PosType x, const string &s) {
  assert(s.size() <= (x == PosType::ISBN ? 20 : 60));
  if (user_manager.current_privilege() < 1) return false;
  if (s == "") return false;
  switch (x) {
    case PosType::NONE : {
      vector<Queryable> t = isbn.find(Queryable(0), Queryable(1));
      for (const auto &tmp : t) {
        books[tmp.value].print();
      }
      if (t.empty()) std::cout << '\n';
      break;
    }
    case PosType::ISBN : {
      vector<Queryable> t = isbn.find(Queryable(s, 0), Queryable(s, 1));
      vector<BookInfo> tmp;
      for (const auto &q : t) {
        tmp.push_back(books[q.value]);
      }
      std::sort(tmp.begin(), tmp.end());
      for (const auto &q : tmp) {
        q.print();
      }
      if (t.empty()) std::cout << '\n';
      break;
    }
    case PosType::NAME : {
      vector<Queryable> t = name.find(Queryable(s, 0), Queryable(s, 1));
      vector<BookInfo> tmp;
      for (const auto &q : t) {
        tmp.push_back(books[q.value]);
      }
      std::sort(tmp.begin(), tmp.end());
      for (const auto &q : tmp) {
        q.print();
      }
      if (t.empty()) std::cout << '\n';
      break;
    }
    case PosType::AUTHOR : {
      vector<Queryable> t = author.find(Queryable(s, 0), Queryable(s, 1));
      vector<BookInfo> tmp;
      for (const auto &q : t) {
        tmp.push_back(books[q.value]);
      }
      std::sort(tmp.begin(), tmp.end());
      for (const auto &q : tmp) {
        q.print();
      }
      if (t.empty()) std::cout << '\n';
      break;
    }
    case PosType::KEYWORD : {
      if (s.find('|') != string::npos) return false;
      vector<Queryable> t = keyword.find(Queryable(s, 0), Queryable(s, 1));
      vector<BookInfo> tmp;
      for (const auto &q : t) {
        tmp.push_back(books[q.value]);
      }
      std::sort(tmp.begin(), tmp.end());
      for (const auto &q : tmp) {
        q.print();
      }
      if (t.empty()) std::cout << '\n';
      break;
    }
  }
  return true;
};