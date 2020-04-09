#include <iostream>
#include <limits>

using namespace std;

#define sign(value) (value < 0 ? -1 : 1)

inline bool overflow(int64_t lhs, int64_t rhs) {
    if (sign(lhs) != sign(rhs)) {
        return false;
    }  
    if (lhs < 0) {
        int64_t left = numeric_limits<int64_t>::min() - lhs;
        return left > rhs;
    } 
    int64_t left = numeric_limits<int64_t>::max() - lhs;
    return left < rhs;
}

int main() {
    int64_t lhs, rhs;
    cin >> lhs >> rhs;
    if (overflow(lhs, rhs)) {
        cout << "Overflow!" << endl;
    } else {
        cout << lhs + rhs << endl;
    }
    return 0;
}
