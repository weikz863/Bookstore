#include "book.hpp"
#include "user.hpp"

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
  if (x == PosType::KEYWORD && s.find('|') != string::npos) return false;
};