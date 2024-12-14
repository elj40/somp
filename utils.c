/*
* Filename:	utils.c
* Date:		14/12/2024 
* Name:		EL Joubert
*
* Some simple quality of life functions
*/
#define ArrayCount(array) (sizeof(array)/sizeof(array[0])) // NOTE: this only works in same scope as when the array was made

int ArrayMax(int nums[], int n)
{
	int max = nums[0];
	for (int i = 1; i < n; i++)
	{
		max = (nums[i] > max) ? nums[i] : max;
	}

	return max;
}
float ArrayMaxf(float nums[], int n)
{
	float max = nums[0];
	for (int i = 1; i < n; i++)
	{
		max = (nums[i] > max) ? nums[i] : max;
	}

	return max;
}
