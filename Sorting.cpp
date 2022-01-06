#include "precomp.h"
#include "Sorting.h"
void Sorting::merge(int to_sort[], int const left, int const mid, int const right)
{
    int sub_array_1 = mid - left + 1;
    int sub_array_2 = right - mid;

    // Temp arrays for the values
    int* arr_left = new int[sub_array_1];
    int* arr_right = new int[sub_array_2];

    // Split the data into the 2 sub arrays
    for (int i = 0; i < sub_array_1; i++)
        arr_left[i] = to_sort[left + i];
    for (int j = 0; j < sub_array_2; j++)
        arr_right[j] = to_sort[mid + 1 + j];

    // Start index first array
    int index_arr_1 = 0;
    // Start index second array
    int index_arr_2 = 0;
    // Start index merged array
    int array_merged_index = left;

    // Merge the temp arrays back into array[left..right]
    while (index_arr_1 < sub_array_1 && index_arr_2 < sub_array_2) {
        if (arr_left[index_arr_1] <= arr_right[index_arr_2]) {
            to_sort[array_merged_index] = arr_left[index_arr_1];
            index_arr_1++;
        }
        else {
            to_sort[array_merged_index] = arr_right[index_arr_2];
            index_arr_2++;
        }
        array_merged_index++;
    }
    //If left has elements left, copy them
    while (index_arr_1 < sub_array_1) {
        to_sort[array_merged_index] = arr_left[index_arr_1];
        index_arr_1++;
        array_merged_index++;
    }
    //If right has elements left, copy them
    while (index_arr_2 < sub_array_2) {
        to_sort[array_merged_index] = arr_right[index_arr_2];
        index_arr_2++;
        array_merged_index++;
    }

    delete[] arr_left;
    delete[] arr_right;

}


void Sorting::merge_sort(int array_to_sort[], int const begin_index, int const end_index)
{
    if (begin_index >= end_index) return;

    int middle = begin_index + (end_index - begin_index) / 2;
    merge_sort(array_to_sort, begin_index, middle);
    merge_sort(array_to_sort, middle + 1, end_index);
    merge(array_to_sort, begin_index, middle, end_index);
}