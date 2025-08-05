# SLE星闪手机通讯范例

## 1.1 介绍

**功能介绍：** 手机搜索连接开发板，订阅特征值通知后，开发板按动开关可以上报按动次数给手机显示

**效果图：** 

<img src="./demo.jpg" />

# 1.2 使用介绍

* 步骤一：最简单的测试步骤，在xxx\src\application\samples\peripheral\button文件夹里面的东西删除，把案例的文件都拷贝进去，本案例已经包含了CMake文件和Kconfig文件

* 步骤二：IDE界面点击Kconfig配置功能，打开samples和button，保存后，点击“build”编译

* 步骤三：编译完毕烧录即可，接线可以参考Kconfig文件里面的配置