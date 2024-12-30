#include "file.hpp"

int main(void) {
  Vector<Map> ve("tempfile");
  Map tmp;
  tmp.value = 1;
  strcpy(tmp.index, "tmp");
  ve.init();
  ve.assign(0, tmp);
  ve.assign(3, tmp);
  tmp = ve[2];
  cout << tmp.value << endl;
  tmp = ve[3];
  cout << tmp.value << endl;
  ve.close();
  return 0;
}