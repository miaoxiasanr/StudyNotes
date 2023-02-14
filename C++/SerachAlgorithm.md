# 搜索算法
## 二分查找法
使用二分查找的前提是数组必须是有序数组
* 难点
  需要明确边界条件，比如到底是while(left<rifht)还是while(left<=right)?
  到底是right=middle还是right=middle+1?
二分法的两种写法
1. 左闭右闭区间
   定义target是在一个左闭右闭的区间里，也就是[left，right]；
   因为target定义在[left,right]中，所以有
   * while(left<=right)要使用<=,因为这样left==right才有意义
   * if(nums[middle]>target)时，right要赋值为middle-1,因为当前这个nums[middle]一定不是target,
    那么接下来要查找的左区间结束下标位置就是middle-1;
代码实例
~~~c++
int BinarySerach(vector<int>nums,int target)
{
    int len=nums.size();
    int left=0,right=len-1;
    while(left<=right)
    {
        int middle=(left+right)/2;
        if(nums[middle]>target)
        {
            right=middle-1;
        }
        else if(nums[middle]<target)
        {
            left=middle+1;
        }
        else
        {
            return middle;
        }
    }
    return -1;
}
~~~
2. 左闭右开区间
   定义target在一个左闭右开区间里，也就是[left,right);
   则有：
   * while(left<right);这里使用<,因为left==right在区间[left,right)是没有意义的
   * if(nums[right]>target)更新为middle，因为当前nums[middle]不等于target，在左区间继续寻找，而寻找区间是
    左闭右开区间，即下一个查询区间不会去比较nums[middle];
代码实例
~~~C++
int BinarySerach(vector<int>nums,int target)
{
    int len=nums.size();
    int left=0,right=len;
    while(left<right)
    {
        int middle=(left+right)/2;
        if(nums[middle]>target)
        {
            right=middle;
        }
        else if(nums[middle]<target)
        {
            left=middle+1;
        }
        else
        {
            return middle;
        }
    }
    return -1;
}
~~~

## KMP算法
KMP算法指的是字符串模式匹配算法，主要解决：在主串T中寻找第一次出现完整子串p时的起始位置
### 字符串匹配问题
![](https://pic4.zhimg.com/80/v2-91db48a5c51253c06c50e5221de6ce67_720w.jpg)
可以是暴力解法(也称Brute_Force算法)
### 暴力解法
可以是暴力解法(也称Brute_Force算法)
![](https://pic2.zhimg.com/v2-c217cea2a7b751e0eea6a2894fdea011_b.webp)
**最坏情况**
![](https://pic4.zhimg.com/v2-7d87a2a6c95818d648b51b30c7891ba3_b.webp)
暴力解法实例
~~~c++
int strStr(string s1,string s2)
{
    //s1:主串     s2:子串
    int len1=s1.size();
    int len2=s2.size();
    int i=0;//主串的位置
    int j=0;//子串的位置
    while(i<len1&&j<len2)
    {
        if(s1[i]==s2[j])
        {
            i++;j++;
        }
        else
        {
            i=i-j+1;//一旦不匹配，i后退
            j=0;
        }
    }
    if(j==len2)
    {
        return i-j;
    }
    else
    {
        return -1;
    }
}
~~~

### KMP算法(暴力算法的优化)
KMP的主要思想：当发现字符串不匹配时，可以知道一部分之前已经匹配的文本内容，可以利用这些信息避免从头再去做匹配
示例文本串
主串：aabaabaaf
子串：aabaaf
####前后缀
前缀：包含首字母，不包含尾字母的所有子串
例如：子串的前缀有：a,aa,aab,aaba,aabaa

后缀：只包含尾字母，不包含首字母的所有子串
例如：子串的后缀有：f,af,aaf,baaf,abaaf

#### 最长公共前缀和
类似于最长相等前缀和
例如
子串                最长公共前缀和
a                       0
aa                      1
aab                     0
aaba                    1
aabaa                   2
aabaaf                  0


####前缀表
前缀表等于公共前缀和
子串  :aabaaf
前缀表:010120

#### next数组等于前缀表

代码示例
~~~c++
vector<int> GetNext(string s)
{
    vector<int>next(s.size());
	int j = 0;//前缀末尾位置
	//i 后缀末尾位置
	for (int i = 1; i < s.size(); i++)
	{
		while (j >0 && s[i] != s[j])//前后缀不相同的情况
		{
			j = next[j-1];//向前回退
		}
        
		if (s[i] == s[j])//相同的前后缀
		{
			j++;
		}
		next[i] = j;//更新next数组
	}
	return next;
}
int strStr(string s1, string s2)
{
	vector<int> next = GetNext(s2);
	int i = 0, j = 0;
	while (i<s1.size()&&j<s2.size())
	{
		if (s1[i]==s2[j])
		{
			i++; 
			j++;
		}
		else
		{
			j = next[j - 1];
		}
	}
	if (j==s2.size())
	{
		return i - j;
	}
	else
	{
		return -1;
	}
}

~~~

![](https://pic4.zhimg.com/80/v2-bb6b018f51f3237465a929fc676e236b_720w.jpg)

