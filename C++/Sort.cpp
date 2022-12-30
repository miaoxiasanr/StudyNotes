int BubbleSort(vector<int>&res)
{
    int len=res.size();
    for(int i=0;i<len-1;i++)
    {
        for(int j=0;j<len-1-i;j++)
        {
            if(res[j]>res[j+1])
            {
                swap(res[j],res[j+1]);
            }
        }
    }
}

int BubbleSort(vector<int>&res)
{
    int len=res.size();
    for(int i=0;i<len-1;i++)
    {
        bool flag=true;
        for(int j=0;j<len-1-i;j++)
        {
            if(res[j]>res[j+1])
            {
                swap(res[j],res[j+1]);
                flag=false;
            }
        }
        if(flag)
        {
            break;
        }
    }
}

void SeleteSort(vector<int>&res)
{
    int len=res.size();
    int min=0;
    for(int i=0;i<len;i++)
    {
        min=i;
        for(int j=i+1;j<len;j++)
        {
            if(res[j]<res[min])
            {
                min=j;
            }
        }
        if(min!=i)
        {
            swap(res[i],res[min]);
        }
    }
}

void InsertSort(vector<int>&res)
{
    int len=res.size();
    for(int i=0;i<len;i++)
    {
        int temp=res[i];
        for(int j=i-1;j>=0;j--)
        {
            if(res[j]>temp)
            {
                res[j+1]=res[j];
                res[j]=temp;
            }
            else
            {
                break;
            }
        }
    }
}

void ShellSort(vector<int>&res)
{
    int len=res.size();
    int step=len/2;
    while(step)
    {
        for(int i=step;i<len;i+=step)
        {
            int temp=res[i];
            int j=i-step;
            while(i>=0&&res[i]>temp)
            {
                res[j+step]=res[j];
                j-=temp;
            }
            res[j+step]=temp;
        }
        step/=2;
    }
}

void HeapAdjust(vector<int>&res,int curr,int len)
{
    if(curr>=len)return;
    int c1=2*curr+1;
    int c2=2*curr+2;
    int max=curr;
    if(c1<len&&res[c1]>res[max])
    {
        max=c1;
    }
    if(c2<len&&res[c2]>res[max])
    {
        max=c2;
    }
    if(max!=curr)
    {
        swap(res[curr],res[max]);
        HeapAdjust(res,max,len);
    }
}

void HeapSort(vector<int>&res)
{
    int len=res.size();
    int lastnode=len-1;
    int parent=(lastnode-1)/2;
    for(int i=parent;i>=0;i--)
    {
        HeapAdjust(res,i,len);
    }
    for(int i=lastnode;i>=0;i--)
    {
        swap(res[i],res[0]);
        HeapAdjust(res,0,i);
    }
}
int Partition(vector<int>&res,int low,int high)
{
    int midnum=res[low];
    while(low<high)
    {
        while(low<high&&midnum<=res[high])
        {
            high--;
        }
        swap(res[low],res[high]);
        while(low<high&&midnum>res[low])
        {
            low++;
        }
        swap(res[low],res[high]);
    }
    return low;
}
void QuickSort(vector<int>&,int low,int high)
{
    int mid;
    while(low<high)
    {
        mid=Partition(res,low,high);
        QuickSort(res,low,mid-1);
        QuickSort(res,low+1,high);
    }
}
//智能指针
CustomSharedptr(CustomSharedptr&_ptr)
{
    this->ptr=_ptr->ptr;
    this->count=_ptr->count;
    ++(*count);
}
CustomSharedptr operator=(CustomSharedptr&_ptr)
{
    if(this->ptr=_ptr.ptr)
        return _ptr;
    if(this->ptr)
    {
        --(*count);
        if(*count<=0)
        {
            delete this->ptr;
            this->ptr=nullptr;
            delete this->count;
            this->count=nullptr;
        }


    }
        this->ptr=_ptr->ptrl;
        this->count=_ptr->count;
        ++(*_ptr->count);
        return *this;
}
