#排序算法
##排序算法总览
![](https://pic.leetcode-cn.com/1629483637-tmENTT-Picture2.png)
##冒泡排序
![动图演示](https://www.runoob.com/wp-content/uploads/2019/03/bubbleSort.gif)
* 原理
比较两个相邻的元素，如果第一个比第二个大，就交换他们两个
对每一对相邻元素作同样的工作，从第一对到最后一对，这步做完后，最后的元素会是最大的数
针对所有的元素重复以上步骤，除了最后一个
持续每次对越来越少的元素重复上面的步骤，直到没有任何一对数字需要比较
* 当输入的数据已经是正序时，排序最快，当输入的数据是反序时，排序最慢
* 代码实例
~~~c++
void BubbleSort(vector<int>& res)
{
	int len = res.size();
	for(int i = 0; i < len-1; i++)
	{
		for (int j = 0; j < len-1-i; j++)
		{
			if (res[j] > res[j + 1])
			{
				swap(res[j], res[j + 1]);
			}
		}
	}
}
~~~
###冒泡排序优化1
* 优化
  冒泡排序的最大问题是不管你有序还是无序，一直在循环，可以用一个临时标记该数组是否有序，如果有序就不用再遍历了；
* 代码实例
~~~c++
void BubbleSort1(vector<int>& res)
{
	int len = res.size();
	bool flag = true;
	for (int i = 0; i < len - 1; i++)
	{
		flag = true;
		for (int j = 0; j < len - 1 - i; j++)
		{
			if (res[j] > res[j + 1])
			{
				swap(res[j], res[j + 1]);
				flag = false;//如果执行到这里，证明数组是无序的
			}
		}
		if (flag)
		{
			break;//有序直接退出
		}
	}
}
~~~ 
###冒泡排序优化2
* 优化
  更新无序数组边界
* 代码实例
~~~c++
void BubbleSort2(vector<int>& res)
{
	int len = res.size();
	int lastswapindex = 0;
	int arrBoundary = len - 1;
	bool flag = true;
	for (int i = 0; i < len - 1; i++)
	{
		flag = true;
		for (int j = 0; j < arrBoundary; j++)
		{
			if (res[j] > res[j + 1])
			{
				swap(res[j], res[j + 1]);
				flag = false;
				lastswapindex = j;//最后一位交换的位置
			}
		}
		//把最后一次交换元素的位置赋值给无序数组的边界
		arrBoundary = lastswapindex;
		if (flag)
		{
			break;
		}
	}
}
~~~
##选择排序
![](https://www.runoob.com/wp-content/uploads/2019/03/selectionSort.gif)
* 原理
  在未排序的数组中找到最大(最小)元素，存放到排列序列的起始位置
  在剩余的未排序中继续寻找最大(最小)元素，然后放到已排序序列的末尾
  以此类推，直至所有元素排序完成
* 代码实例
~~~c++
void SeletionSort(vector<int>& res)
{

	int len = res.size();
	int min = 0;
	for (int i=0;i<len;i++)
	{
		min = i;
		for (int j = i+1; j < len; j++)
		{
			if (res[j] < res[min])
			{
				min = j;
			}
		}
		if (min != i)
		{
			swap(res[i], res[min]);
		}
	}
}
~~~
##插入排序
![](https://www.runoob.com/wp-content/uploads/2019/03/insertionSort.gif)
* 原理
  从第一个元素开始，该元素可以认为是已经被排序
  取出下一个元素，在已经排序的元素从后往前扫描
  如果该元素大于新元素，则将该元素移到下一位置上
  重复步骤三，直到找到已排序的元素小于或等于新元素的位置上
  将新元素插到该位置后
* 代码实例
~~~C++
void InsertSort(vector<int>& res)
{
	int len = res.size();
	for (int i = 1; i < len; i++)
	{
		int temp = res[i];
		for (int j = i - 1; j >= 0; j--)
		{
			if (res[j]>temp)
			{
				res[j + 1] = res[j];
				res[j] = temp;
			}
			else
			{
				break;
			}
		}

	}
}
~~~
##希尔排序
![](https://img-blog.csdnimg.cn/20191204143506608.gif)
* 原理
  插入排序的优化，插入排序每次只能将数据移动一位，希尔排序为了加快插入的速度，让数据移动的时候实现跳跃移动，节省开支
* 代码实例
~~~c++
void shell_sort(vector<int>& res)
{
	int len = res.size();
	int step = len/2;

	while (step)
	{
		for (int i = step; i < len; i+=step)
		{
			int temp = res[i];
			int j = i - step;
			while (j>=0&&res[j]>temp)
			{
				res[j + step] = res[j];
				j -= step;
			}
			res[j + step] = temp;
		}
		step /= 2;
	}
}
~~~
##堆排序
![](https://www.runoob.com/wp-content/uploads/2019/03/heapSort.gif)
* 原理
  将排序的序列构建成一个大顶堆。整个序列的最大值就是堆顶的根节点，然后将它移走，然后将剩余的n-1个序列重新构造成一个堆，这样就会得到n个元素中次大的元素，如此反复执行，得到一个有效序列。
* 代码实例
~~~c++
void HeapSort(vector<int>& res)
{
	int len = res.size();
	int last_node = len - 1;
    //子节点的父节点是(i-1)/2;
	int parent = (last_node - 1) / 2;
	//创建堆
	for (int i = parent; i >= 0; i--)
	{
		HeapAdjust(res, i, len);
	}
	for (int i = last_node; i >= 0; i--)
	{
		swap(res[i], res[0]);
		HeapAdjust(res, 0, i);
	}
}
//这个函数就是为了确保parent大于子节点
void HeapAdjust(vector<int>& res, int curr, int len)
{
	if (curr >= len)
	{
		return;
	}
    //父节点i的两个子节点分别是2*i+1,2*i+2;
	int c1 = 2 * curr + 1;
	int c2 = 2 * curr + 2;
	int max = curr;
	if (c1 < len&&res[c1] > res[max])
	{
		max = c1;
	}
	if (c2 < len&&res[c2] > res[max] )
	{
		max = c2;
	}
	if (max != curr)
	{
		swap(res[max],res[curr]);
		HeapAdjust(res, max, len);
	}
}
~~~
##快速排序
![](https://pic.leetcode-cn.com/1612615552-rifQwI-Picture1.png)
* 原理
  两个核心点:哨兵划分和递归
  * 哨兵划分
    以数组某个元素(一般是首元素)为基准，将所有小于基准数的元素移动到左边，大于基准数的元素移动到右边
  * 递归
    对左子数组和右子数组分别递归执行哨兵划分，直至子数组长度为1时终止递归，即可完成对整个数组的排序
* 代码实例
~~~c++
void  QuickSort(vector<int>& res,int low,int high)
{
	int mid;
	if (low < high)
	{
		mid = Partition(res, low, high);
		QuickSort(res, low, mid - 1);
		QuickSort(res, mid + 1, high);
	}
}

int Partition(vector<int>& res, int low, int high)
{
	int midnum = res[low];
	while (low<high)
	{
		while (low<high&&midnum<=res[high])
		{
			high--;
		}
		swap(res[low], res[high]);
		while (low<high&&midnum>res[low])
		{
			low++;
		}
		swap(res[low], res[high]);
	}
	return low;
}
~~~
##归并排序
![](https://pic.leetcode-cn.com/1632675739-CNHaOu-Picture1.png)
* 原理
  [分]：不断的将数组从中点位置划分开，将原数组的排序问题转化为子数组的排序问题
  [治]：划分到子数组长度为1时，开始向上合并，不断地将左右两个较短排序数组合并为一个较长数组，直至合并值原数组是完成排序

* 代码示例
~~~c++
void MergeSort(vector<int>& res, int low, int high)
{
	if (low >= high)
	{
		return -1;
	}
	///递归划分
	int mid = (low + high) / 2;
	MergeSort(res, low, mid);
	MergeSort(res, mid + 1, high);

	//合并阶段
	vector<int>tmp(high - low + 1);
	for (int i = low; i <= high; i++)
	{
		tmp[i - low] = res[i];
	}

	// 两指针分别指向左/右子数组的首个元素
	int i = 0, j = mid - low + 1;       
	for (int k = low; k <= high; k++)
	{  
		// 遍历合并左/右子数组
		if (i == mid - low + 1)
			res[k] = tmp[j++];
		else if (j == high - low + 1 || tmp[i] <= tmp[j])
			res[k] = tmp[i++];
		else
		{
			res[k] = tmp[j++];
		}
	}
}
~~~

