#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Answer {
    int low;
    int high;
    int sum;
};

Answer find_max_crossing_subarray(int* arr, int low, int high) {
    int left_sum = -1000000;
    int right_sum = -1000000;
    int max_left = 0;
    int max_right = 0;

    int mid = (low + high) / 2;
    int sum = 0;
    for (int i = mid; i >= low; i--) {
        sum += arr[i];
        if (sum > left_sum) {
            left_sum = sum;
            max_left = i;
        }
    }

    sum = 0;
    for (int i = mid + 1; i < high; i++) {
        sum += arr[i];
        if (sum > right_sum) {
            right_sum = sum;
            max_right = i;
        }
    }

    return {max_left, max_right, left_sum + right_sum};
}

Answer find_max_subarray_nlogn(int* arr, int low, int high) {
    if (low == high) {
        return {low, high, arr[low]};
    }

    int mid = (low + high) / 2;

    Answer ans1 = find_max_subarray_nlogn(arr, low, mid);

    Answer ans2 = find_max_subarray_nlogn(arr, mid + 1, high);

    Answer ans3 = find_max_crossing_subarray(arr, low, high);

    if (ans1.sum >= ans2.sum && ans1.sum >= ans3.sum) {
        return ans1;
    }
    if (ans2.sum >= ans3.sum && ans2.sum >= ans1.sum) {
        return ans2;
    } else {
        return ans3;
    }
}


Answer find_max_subbaray_n2(int* arr, int len) {
    int max_sum = -10000000;
    int low, high;

    for (int i = 0; i < len; i++) {
        int sum = 0;
        for (int j = i; j < len; j++) {
            sum += arr[j];
            if (sum > max_sum) {
                low = i;
                high = j;
                max_sum = sum;
            }
        }
    }

    return {low, high, max_sum};
}

int main() {
    int arr[16] = {13, -3, -25, 20, -3, -16, -23, 18, 20, -7, 12, -5, -22, 15, -4, 7};

    Answer ans1 = find_max_subarray_nlogn(arr, 0, 16);

    Answer ans2 = find_max_subbaray_n2(arr, 16);

    printf("Ans1: low: %d, high: %d, sum: %d\n", ans1.low, ans1.high, ans1.sum);
    printf("Ans2: low: %d, high: %d, sum: %d\n", ans2.low, ans2.high, ans2.sum);

    return 0;
}
