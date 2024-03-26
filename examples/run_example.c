int main() {
    int res = 1 + 1;

    if (res) {
        res = res * 2 + 3 * (res + 2);
    } else {
        res = 2 + 4 * 7;
    }

    return 0;
}
