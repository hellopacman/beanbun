欢迎大家来我的主页。
游戏小站，另类模式修改之地，给你另类的感觉！:)

http://ylycloak.yeah.net
http://game.9ww.net
http://diablo.9ww.net


--------------------------------------------------------

Affix Editor 1.2 - Readme

这个工具通过修改MagicPrefix.bin和MagicSuffix.bin文件来达到修改Diablo II装备前后缀的目的。

更新历史

  1.2  2003.7  支持直接操作MPQ文件 (Important!!!)
               可以与.bin文件和.mpq文件相关联

  1.1  2003.7  支持批处理和过滤过程中复杂表达式
               修正了一些小错误

  1.0b 2003.7  软件发布

文件说明

  MagicPrefix.bin  记录装备前缀的文件
  MagicSuffix.bin  记录装备后缀的文件
  AutoMagic.bin    记录装备自带属性的文件，如：AmaBow加技能，PalShield加ED或四抗等

  这三个文件结构基本相同

功能说明

  Open: 打开.bin文件
  Save: 保存当前的文件
  Help: 帮助
  Exit: 退出，不保存

  Add: 添加一个新的词缀
  Delete: 删除一个词缀

  Batch Modify: 对一列属性修改
  Filter: 滤选满足条件的词缀
  Lock: 锁定某些属性，当选择其它记录时，其对应属性会被修改为被锁定的属性值

属性值说明

    软件中属性值的名称是选用MagicPrefix.txt、MagicSuffix.txt和AutoMagic.txt的名称，因此，有些属性的意义还不清楚。

  Name: 词缀在游戏中所显示的名称
  Version: 不详，可能标记词缀所存在的游戏版本
  ModxCode，ModxParam，ModxMin，ModxMax: x取值为1-3，每个词缀可有三个Mod，这几个属性分别代表Mod的代码、参数和最大、最小值
  Level: 装备能出现该词缀的最小iLvl
  MaxLevel: 装备能出现该词缀的最大iLvl
  LevelReq: 装备该词缀的等级需求
  Class,ClassSpecific,ClassLevelReq: 不详，和人物有关
  Rare: 该词缀可否出现在Rare装备上
  Group: 属性的组号，同一组的属性在一件装备上最多出现一个
  Frequency: 该词缀能出现的概率数
  Spawnable: 不详
  TransformColor: 装备带有该词缀时所变成的颜色
  iType1-7: 能带有该词缀的装备类
  eType1-5: 不能带有该词缀的装备类
  Divide,Multiply,Add: 不详，加减乘除

复杂表达式

  在批处理操作和过滤操作中使用的表达式
  变量：所有的字段都可作为变量使用，不区分大小写。如: name,Mod1Code,RARE...
  常量：由一对双引号""括起来的字符串，整数常量双引号加不加意义相同。如: "dmg%",1,"1"...
  支持运算符：算术运算符(+,-,*,/)，关系运算符(=,>,<,<>,>=,<=)，逻辑运算符(AND,OR)和括号。

  例如：只显示Rare装备可以出现的ED的词缀(过滤)
  在Filter对话框中选择Advance，手动输入或用Insert功能生成表达示 Rare="1" and Mod1Code="dmg%"，即可

Q:MagicPrefix.bin、MagicSuffix.bin和AutoMagic.bin文件如何获取？
A:这两个文件可以用MPQ2K等工具从Patch_d2.mpq中提取。

Q:为什么改出的词缀在游戏中显示时会有一对括号？
A:这是因为新修改的字符串在游戏原字符串表中不存在，需要用工具修改Patch_d2.mpq里的字符串表文件patchstring.tbl，将新修改的字符串加入。

* 软件中所有用到的列表都是从MagicPrefix.txt、MagicSuffix.txt和AutoMagic.txt中整理，由于没有完整的资料，所以里面的表项可以有缺漏，若哪们有完整资料，一定要通知我哦
* 欢迎大家对这个工具进行测试，若发现有错误，请通知我


作者：Edison
2003.7

附：本程序调用了SFmpq.dll，以下为SFmpq.dll的官方声明

  This library is freeware, you can do anything you want with it but with
  one exception.  If you use it in your program, you must specify this fact
  in Help|About box or in similar way.  You can obtain version string using
  SFMpqGetVersionString call.

  THIS LIBRARY IS DISTRIBUTED "AS IS".  NO WARRANTY OF ANY KIND IS EXPRESSED
  OR IMPLIED. YOU USE AT YOUR OWN RISK. THE AUTHOR WILL NOT BE LIABLE FOR 
  DATA LOSS, DAMAGES, LOSS OF PROFITS OR ANY OTHER KIND OF LOSS WHILE USING
  OR MISUSING THIS SOFTWARE.