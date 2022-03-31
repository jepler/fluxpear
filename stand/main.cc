#include <fstream>
#include <iostream>

void seekg(std::ifstream *f, std::streampos offset) { f->clear(); f->seekg(offset); }
size_t tellg(std::ifstream *f) { return (size_t)f->tellg(); }
size_t read(std::ifstream *f, void *ptr, size_t n) { f->read((char *)ptr, n); return f->gcount(); }

#include "libwoz.h"

using Woz = WozFile<std::ifstream>;

void fatal(const char *msg) {
    std::cerr << msg << "\n";
    abort();
}

int main(int argc, char **argv) {
    std::ifstream f(argv[1], std::ios_base::in | std::ios::binary);

    Woz woz;
    if (!woz.open(&f)) { fatal("Could not open woz"); }

    uint8_t buf[woz.largest_track()*512*8];
    size_t sz = woz.readtrack(0, buf, sizeof(buf));
    if(sz == (size_t)-1) { fatal("could not read track"); }

    std::cout << "Content of track 0:\n";

    size_t j = 8; 
    for(size_t i=0; i < 256 && i < sz; i++) {
        if(j >= 8 && buf[i]) {
            std::cout << "\n";
            j = 0;
        }
        std::cout << (char)('0' + buf[i]);
        j ++;
    }
    std::cout << "...\n";
}
