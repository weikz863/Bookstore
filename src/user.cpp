#include "user.hpp"
#include "book.hpp"

extern BookManager book_manager;

UserManager user_manager;
Vector<ID> & selected = book_manager.selected;