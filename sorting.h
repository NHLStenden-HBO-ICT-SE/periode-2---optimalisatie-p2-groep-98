#pragma once
class sorting
{
public:
	static int orientation(vec2 p, vec2 q, vec2 r);
	static void convex_merge(vec2 list[], int const left, int const mid, int const right);
	static void convex_merge_sort(vec2 list[], int const begin, int const end);
	static void merge(int to_sort[], int const left, int const mid, int const right);
	static void merge_sort(int array_to_sort[], int const begin_index, int const end_index);
};
