#pragma once
class sorting
{
public:
	static int orientation(vec2 p, vec2 q, vec2 r);
	/// <summary>
	/// Merge 2 list of vec2
	/// </summary>
	/// <param name="to_sort"></param>
	/// <param name="left"></param>
	/// <param name="mid"></param>
	/// <param name="right"></param
	static void convex_merge(vec2 list[], int const left, int const mid, int const right);
	/// <summary>
	/// Sort the array of vec2
	/// </summary>
	/// <param name="list"></param>
	/// <param name="begin"></param>
	/// <param name="end"></param>
	static void convex_merge_sort(vec2 list[], int const begin, int const end);
	/// <summary>
	/// Merge 2 list of int
	/// </summary>
	/// <param name="to_sort"></param>
	/// <param name="left"></param>
	/// <param name="mid"></param>
	/// <param name="right"></param>
	static void merge(int to_sort[], int const left, int const mid, int const right);
	/// <summary>
	/// Sort the array from high to low
	/// </summary>
	/// <param name="array_to_sort"></param>
	/// <param name="begin_index"></param>
	/// <param name="end_index"></param>
	static void merge_sort(int array_to_sort[], int const begin_index, int const end_index);
};
