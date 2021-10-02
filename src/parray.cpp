#include <iostream>

#include "type.h"

using namespace std;

int main() {
    
    parray<int, prange<-1, 1>> items;
    items[-1] = -1;
    items[0]  = 0;
    items[1]  = 1;

    for (int i = items.min(); i <= items.max(); ++i) {
        std::cout << i << " = " << items[i] << std::endl;
    }
    return 0;
}
