#include "precomp.h"
#include "sorting.h"

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 = p, q and r are collinear
// 1 = Clockwise
// 2 = Counterclockwise
int sorting::orientation(vec2 p, vec2 q, vec2 r)
{
    float val = (q.y - p.y) * (r.x - q.x) -
        (q.x - p.x) * (r.y - q.y);

    if (val == 0)
        return 0;  // collinear
    if (val > 0) // clock or counterclock wise
        return 1;
    else
        return 2;
}

// Square of distance between p1 and p2
float distance_square(vec2 p1, vec2 p2)
{

    return (p1.x - p2.x) * (p1.x - p2.x) +
        (p1.y - p2.y) * (p1.y - p2.y);

}

// A function used by library function qsort() to sort an array of points with respect to the first point
int compare(vec2 point1, vec2 point2, vec2 p0)
{
    vec2 p1 = point1;
    vec2 p2 = point2;
    int dir = sorting::orientation(p0, p1, p2);

    if (dir == 0)
        return (distance_square(p0, p2) >= distance_square(p0, p1)) ? -1 : 1;
    return (dir == 2) ? -1 : 1;
}

/// <summary>
/// Merge two arrays of vec2
/// </summary>
/// <param name="to_sort"></param>
/// <param name="left"></param>
/// <param name="mid"></param>
/// <param name="right"></param>
void sorting::convex_merge(vec2 to_sort[], int const left, int const mid, int const right)
{
    int sub_array_1 = mid - left + 1;
    int sub_array_2 = right - mid;

    // Temp arrays for the values
    vec2* arr_left = new vec2[sub_array_1];
    vec2* arr_right = new vec2[sub_array_2];

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
        if (compare(arr_left[index_arr_1], arr_right[index_arr_2], to_sort[0]) == -1) {
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

/// <summary>
/// Sort the array of vec2
/// </summary>
/// <param name="list"></param>
/// <param name="begin"></param>
/// <param name="end"></param>
void sorting::convex_merge_sort(vec2 list[], int const begin, int const end)
{
    if (begin >= end) {
        return;
    }
    int mid = begin + (end - begin) / 2;
    convex_merge_sort(list, begin, mid);
    convex_merge_sort(list, mid + 1, end);
    convex_merge(list, begin, mid, end);

}

/// <summary>
/// Merge 2 list of int
/// </summary>
/// <param name="to_sort"></param>
/// <param name="left"></param>
/// <param name="mid"></param>
/// <param name="right"></param>
void sorting::merge(int to_sort[], int const left, int const mid, int const right)
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

/// <summary>
/// Sort the array from high to low
/// </summary>
/// <param name="array_to_sort"></param>
/// <param name="begin_index"></param>
/// <param name="end_index"></param>
void sorting::merge_sort(int array_to_sort[], int const begin_index, int const end_index)
{
    if (begin_index >= end_index) return;

    int middle = begin_index + (end_index - begin_index) / 2;
    merge_sort(array_to_sort, begin_index, middle);
    merge_sort(array_to_sort, middle + 1, end_index);
    merge(array_to_sort, begin_index, middle, end_index);
}