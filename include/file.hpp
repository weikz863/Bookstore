#include <cstddef>
#include <ios>
#include <iostream>
#include <fstream>
#include <string>
#include <type_traits>
#include <vector>
#include <cassert>
#include <cstring>
#include <climits>

using std::string;
using std::vector;
using std::fstream;
using std::ifstream;
using std::ofstream;

struct BasicFileStorage {
  virtual constexpr int size() const = 0;
  virtual void readfrom(fstream &) = 0;
  virtual void writeto(fstream &) const = 0;
};

struct Map : BasicFileStorage {
  static constexpr int STRLEN = 65;
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
  Map& operator = (const Map &x) {
    if (&x != this) {
      strcpy(index, x.index);
      value = x.value;
    }
    return *this;
  }
  */
  bool operator < (const Map& x) const {
    int f = std::strcmp(index, x.index);
    if (f) return f < 0;
    else return value < x.value;
  }
  bool operator > (const Map& x) const {
    return x < *this;
  }
  bool operator <= (const Map& x) const {
    return !(x < *this);
  }
  bool operator >= (const Map& x) const {
    return !(*this < x);
  }
  bool operator == (const Map& x) const {
    return value == x.value && !std::strcmp(index, x.index);
  }
  bool operator != (const Map& x) const {
    return value != x.value || std::strcmp(index, x.index);
  }
};

template<int SIZE, typename Data>
class BlockList {
  static_assert(std::is_base_of<BasicFileStorage, Data>(), "");
  static const Data what;
  static constexpr int ADDITIONAL = 2 * sizeof(int);
  static constexpr char buf[SIZE * what.size()] = "";
  const char * const filename;
  fstream file{};
  struct Block {
    Data first;
    int pos;
    int size;
  };
  vector<Block> blocks{};
  void newblock(const int id, const vector<Data> &nxt) {
    file.seekp(0);
    int t = blocks.size() + 1;
    file.write(reinterpret_cast<char*>(&t), sizeof(int));
    file.seekp(0, std::ios_base::end);
    int x = file.tellg();
    if (id) {
      file.seekg(blocks[id - 1].pos);
    } else {
      file.seekp(ADDITIONAL / 2);
    }
    file.write(reinterpret_cast<char*>(&x), sizeof(int));
    file.seekp(0, std::ios_base::end);
    if (id == blocks.size()) {
      file.write(buf, sizeof(int)); // whatever
    } else {
      file.write(reinterpret_cast<char*>(&blocks[id].pos), sizeof(int));
    }
    t = nxt.size();
    file.write(reinterpret_cast<char*>(&t), sizeof(int));
    for (const auto &tmp : nxt) {
      tmp.writeto(file);
    }
    int zeros = (SIZE - t) * what.size();
    file.write(buf, zeros);
    blocks.insert(blocks.begin() + id, {nxt[0], x, t});
  }
  void delblock(const int id) {
    if (id) {
      file.seekg(blocks[id - 1].pos);
    } else {
      file.seekg(ADDITIONAL / 2);
    }
    file.write(reinterpret_cast<char*>(&blocks[id + (id != blocks.size() - 1)].pos), sizeof(int));
    blocks.erase(blocks.begin() + id);
    file.seekg(0);
    int t = blocks.size();
    file.write(reinterpret_cast<char*>(&t), sizeof(int));
  }

public:
  struct iterator {
    fstream &file;
    int const pos; 
    void assign(const Data &x) {
      file.seekg(pos);
      x.writeto(file);
    }
    void read(Data &x) {
      file.seekg(pos);
      x.readfrom(file);
    }
  };
  BlockList (const char *s) : filename(s) {}; 
  void init() {
    int blockcnt, thisblock;
    #ifdef ONLINE_JUDGE
    file.open(filename, std::ios_base::in | std::ios_base::out | std::ios::binary);
    #endif
    if (!file.is_open()) {
      file.open(filename, std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios::binary);
      blockcnt = 0;
      thisblock = 2 * sizeof(int);
      file.write(reinterpret_cast<char*>(&blockcnt), sizeof(int));
      file.write(reinterpret_cast<char*>(&thisblock), sizeof(int));
    } else {
      file.read(reinterpret_cast<char*>(&blockcnt), sizeof(int));
      file.read(reinterpret_cast<char*>(&thisblock), sizeof(int));
      blocks.resize(blockcnt);
      for (auto &block : blocks) {
        file.seekp(thisblock);
        block.pos = thisblock;
        file.read(reinterpret_cast<char*>(&thisblock), sizeof(int));
        file.read(reinterpret_cast<char*>(&block.size), sizeof(int));
        block.first.readfrom(file);
      }
    }
  }
  void close() {
    file.close();
  }
  void insert(const Data &x) {
    if (blocks.empty()) {
      newblock(0, vector<Data>{x});
      return;
    }
    int blockid = 0;
    while (blockid < blocks.size() && blocks[blockid].first <= x) {
      blockid++;
    }
    blockid--;
    if (blockid < 0) {
      blockid = 0;
    }
    int pos = 0;
    Data cur;
    file.seekp(blocks[blockid].pos + ADDITIONAL);
    cur.readfrom(file);
    pos++;
    while (pos < blocks[blockid].size && cur < x) {
      cur.readfrom(file);
      pos++;
    }
    if (cur >= x) {
      file.seekp(-what.size(), std::ios_base::cur);
      x.writeto(file);
    } else {
      cur = x;
    }
    Data t;
    while (pos < blocks[blockid].size) {
      t.readfrom(file);
      file.seekp(-what.size(), std::ios_base::cur);
      cur.writeto(file);
      cur = t;
      pos++;
    }
    if (blocks[blockid].size == SIZE) {
      int const rem = SIZE * 2 / 3;
      file.seekp(blocks[blockid].pos + rem * what.size() + ADDITIONAL);
      vector<Data> split(SIZE - rem);
      for (auto &tmp : split) tmp.readfrom(file);
      split.push_back(cur);
      newblock(blockid + 1, split);
      blocks[blockid].size = rem;
    } else {
      blocks[blockid].size++;
      cur.writeto(file);
    }
    file.seekp(blocks[blockid].pos + ADDITIONAL / 2);
    file.write(reinterpret_cast<char*>(&blocks[blockid].size), sizeof(int));
    blocks[blockid].first.readfrom(file);
  }
  vector<Data> find(const Data &begin, const Data &end) {
    vector<Data> ret{};
    int blockid = 0;
    while (blockid < blocks.size() && blocks[blockid].first <= begin) blockid++;
    blockid = std::max(blockid - 1, 0);
    Data x;
    for (int i = blockid; i < blocks.size(); i++) {
      file.seekp(blocks[i].pos + ADDITIONAL);
      for (int j = 0; j < blocks[i].size; j++) {
        x.readfrom(file);
        if (x > end) return ret;
        if (x >= begin) ret.push_back(x);
      }
    }
    return ret;
  }
  iterator find_one (const Data &begin) {
    int blockid = 0;
    while (blockid < blocks.size() && blocks[blockid].first < begin) blockid++;
    blockid = std::max(blockid - 1, 0);
    Data x;
    for (int i = blockid; i < blocks.size(); i++) {
      file.seekp(blocks[i].pos + ADDITIONAL);
      for (int j = 0; j < blocks[i].size; j++) {
        x.readfrom(file);
        if (x >= begin) return iterator(file, int(file.tellg()) - x.size());
      }
    }
    return iterator(file, -1);
  }
  void delet(const Data &x) {
    int blockid = 0; 
    while (blockid < blocks.size() && blocks[blockid].first <= x) blockid++;
    blockid = std::max(blockid - 1, 0);
    Data t;
    for (int i = blockid; i < blocks.size(); i++) {
      file.seekp(blocks[i].pos + ADDITIONAL);
      for (int j = 0; j < blocks[i].size; j++) {
        t.readfrom(file);
        if (t > x) return;
        else if (t == x) {
          int pos = int(file.tellg()) - what.size();
          vector<Data> tmp(blocks[i].size - j - 1);
          for (auto &dat : tmp) dat.readfrom(file);
          file.seekg(pos);
          for (auto &dat : tmp) dat.writeto(file);
          blocks[i].size--;
          if (blocks[i].size <= 0) {
            delblock(i);
          } else {
            file.seekg(blocks[i].pos + ADDITIONAL / 2);
            file.write(reinterpret_cast<char*>(&blocks[i].size), sizeof(int));
            blocks[i].first.readfrom(file);
          }
          return;
        }
      }
    }
  }
};

template<typename Data>
class Vector {
  static_assert(std::is_base_of<BasicFileStorage, Data>(), "");
  const char * const filename;
  fstream file{};
  int siz;
public:
  Vector(const char *s) : filename(s), siz(0) {};
  void init() {
    #ifdef ONLINE_JUDGE
    file.open(filename, std::ios_base::in | std::ios_base::out | std::ios::binary);
    #endif
    if (!file.is_open()) {
      file.open(filename, std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios::binary);
    }
  }
  void close() {
    file.close();
  }
  int size() const {
    return siz;
  }
  Data operator[](size_t x) {
    Data ret;
    file.seekg(x * ret.size());
    ret.readfrom(file);
    return ret;
  }
  Data back() {
    return this->operator[](siz - 1);
  }
  void assign(size_t x, const Data &t) {
    file.seekg(x * t.size());
    t.writeto(file);
  }
  void push_back(const Data &t) {
    assign(siz, t);
    siz++;
  }
  void pop_back() {
    siz--;
  }
  void clear() {
    siz = 0;
  }
  bool empty() const {
    return siz == 0;
  }
};