#include <iostream>
#include <fstream>
#include <iomanip>
#include <ios>
 
#include <stdlib.h>

using namespace std;

int main () {
  ifstream S;
  S.open ("insample", ios::binary|ios::ate);

  ifstream::pos_type size = S.tellg();
  
  char * memblock = new char [size];
  S.seekg (0, ios::beg);
  S.read (memblock, size);
  S.close();

  char c;
  char hexnum[3];
  int hexpos = 0;
  hexnum[0] = '\0';

  for(int i=0; i<size; i++) {
    if(memblock[i] == '\n' || memblock[i] == '\r') {
      continue;
    }

    //cout << "hexpos: " << hexpos << ", i: " << memblock[i] << ", hexnum: " << hexnum << endl;

    if(hexpos == 2) {
      // start of next hex value reached
      hexnum[2] = '\0';
      c = (char)strtol(hexnum, (char **)NULL, 16);
      cout << c;
      hexpos = 0;
    }

    hexnum[hexpos] = memblock[i];
    hexnum[hexpos+1] = '\0';
    hexpos++;
  }

  if(hexnum[0] != '\0') {
    c = (char)strtol(hexnum, (char **)NULL, 16);
    cout << c;
  }

  delete[] memblock;
  return 0;
}
