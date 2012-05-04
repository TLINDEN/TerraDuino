#include <iostream>
#include <fstream>
#include <iomanip>
#include <ios>
 
#include <stdlib.h>

using namespace std;

int main () {
  ifstream S;
  S.open ("sample", ios::binary|ios::ate);

  ifstream::pos_type size = S.tellg();
  
  char * memblock = new char [size];
  S.seekg (0, ios::beg);
  S.read (memblock, size);
  S.close();

  int rows=0;
  for(int i=0; i<size; i++) {
    cout << hex << setfill('0') << setw(2) << nouppercase << (unsigned short)memblock[i];
    rows++;
    if(rows == 36) {
      cout << endl;
      rows = 0;
    }
  }

  delete[] memblock;
  return 0;
}
