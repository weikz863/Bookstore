#include "book.hpp"
#include "file.hpp"
#include "user.hpp"
#include <algorithm>
#include <cassert>
#include <climits>
#include <sstream>
#include <vector>

vector<string> parse(const char * const keyword) {
  vector<string> ret{};
  string tmp("");
  for (int i = 0; i < BookInfo::NAMESIZE; i++) {
    switch (keyword[i]) {
      case '\0' : {
        if (!tmp.empty()) ret.push_back(tmp);
        return ret;
        break;
      }
      case '|' : {
        assert(tmp != "");
        ret.push_back(tmp);
        tmp.clear();
        break;
      }
      default: {
        tmp.push_back(keyword[i]);
      }
    }
  }
  assert(false);
  return ret;
}

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
      vector<Queryable> t = isbn.find(Queryable(s, -1), Queryable(s, INT_MAX));
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
      vector<Queryable> t = name.find(Queryable(s, -1), Queryable(s, INT_MAX));
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
      vector<Queryable> t = author.find(Queryable(s, -1), Queryable(s, INT_MAX));
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
      vector<Queryable> t = keyword.find(Queryable(s, -1), Queryable(s, INT_MAX));
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
    default: {
      assert(false);
    }
  }
  return true;
};
bool BookManager::select(const string &s) {
  assert(s.size() <= 20);
  if (user_manager.current_privilege() < 3) return false;
  vector<Queryable> t = isbn.find(Queryable(s, -1), Queryable(s, INT_MAX));
  assert(t.size() <= 1);
  if (t.empty()) {
    isbn.insert(Queryable(s, books.size()));
    books.push_back(s);
  } else {
    selected.pop_back();
    selected.push_back(t[0].value);
  }
  return true;
}
double BookManager::buy(const string &s, int qu) {
  assert(s.size() <= 20);
  if (qu < 0 || user_manager.current_privilege() < 1) return -1;
  vector<Queryable> t = isbn.find(Queryable(s, -1), Queryable(s, INT_MAX));
  assert(t.size() <= 1);
  if (t.empty()) return -1;
  BookInfo book = books[t[0].value];
  if (qu > book.quantity) return -1;
  book.quantity -= qu;
  books.assign(t[0].value, book);
  return book.price * qu;
}
bool BookManager::modify(PosType x, const string &s) {
  if (user_manager.current_privilege() < 3 || s.empty()) return false;
  int val = selected.back().value;
  if (val == -1) return false;
  switch (x) {
    case PosType::ISBN : {
      BookInfo tmp = books[val];
      if (tmp.ISBN == s) return false;
      strcpy(tmp.ISBN, s.c_str());
      books.assign(val, tmp);
      break;
    }
    case PosType::NAME : {
      BookInfo tmp = books[selected.back().value];
      strcpy(tmp.name, s.c_str());
      books.assign(val, tmp);
      break;
    }
    case PosType::AUTHOR : {
      BookInfo tmp = books[val];
      strcpy(tmp.author, s.c_str());
      books.assign(val, tmp);
      break;
    }
    case PosType::KEYWORD : {
      BookInfo tmp = books[val];
      auto kwds = parse(tmp.keywords);
      strcpy(tmp.keywords, s.c_str());
      for (const auto &s : kwds) {
        keyword.delet(Queryable(s, val));
      }
      kwds = parse(s.c_str());
      std::sort(kwds.begin(), kwds.end());
      if (std::unique(kwds.begin(), kwds.end()) != kwds.end()) return false;
      for (const auto &s : kwds) {
        keyword.insert(Queryable(s, val));
      }
      books.assign(val, tmp);
      break;
    }
    case PosType::PRICE: {
      BookInfo tmp = books[val];
      std::stringstream ss(s);
      ss >> tmp.price;
      books.assign(val, tmp);
      break;
    }
    case PosType::QUANTITY: {
      int buying;
      std::stringstream ss(s);
      ss >> buying;
      if (buying <= 0) return false;
      BookInfo tmp = books[val];
      tmp.quantity += buying;
      books.assign(val, tmp);
      break;
    }
    case PosType::NONE: {
      assert(false);
      break;
    }
  }
  return true;
}