# Slate基本框架
![](https://pic3.zhimg.com/80/v2-8c339a750c2ef4d919c42c73d0c31c76_720w.webp "Slate基本框架")
## Swidget
UE4Slate框架中最基础的类是SWidget，Swidget以智能指针的形式存在，继承自TSharedFromThis,是Slate空间的抽象基类。基于SWidget的子类主要有三类，分别是SCompoudWidget,SLeafWidget,SPanel。

他们三个最主要的区别在于能附加子空间的数目。
* SCompoudWidget
    其子类只能调用一个子控件，常见的子类有SButton,SBorder等，他们的特点是只能附加一个子控件。
* SLeafWidget
  其子类已经是叶子节点，不能在拥有子控件，常见的子类有SImage，STextBlock。这类控件都是没有子控件插槽的。
* SPanel
    其子类的特点是可以无限添加子控件，没有数量限制，常见的子类有SHorizontal,SPanel等

控件的父子关系是依靠插槽(Slot)实现的，每一个Slot可以存放一个Widget，有的控件只有一个Slot(例如SComponundWidget，SBoxPanel)；而有的控件可以有多个Slot(例如SHorizontalBox,SVerticalBox)

[Slate基本框架](https://zhuanlan.zhihu.com/p/262232941)