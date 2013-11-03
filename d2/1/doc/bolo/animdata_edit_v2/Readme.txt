游戏小站，另类模式修改基地！:)

http://www.cnmods.com    http://www.cnmods.net

http://ylycloak.yeah.net

ylycloak注:

Animdata.d2编辑工具。

说明：这恐怕是目前唯一的一个针对Animdata.d2文件的编辑工具，如果打算定义自己怪物的动作，那这个工具是必备、也是唯一的选择！:)


___________________________________________________________________
======================
| Animdata_edit v. 2 |
| freeware           |
| by Paul SIRAMY     |
======================

The program extracts the data from AnimData.d2 into AnimData.txt.
You can edit this .txt under Ms-Excel or any similar spreadsheet,
then re-use the program to make a new AnimData.d2.

   1. Launch Extract.bat, AnimData.txt is created
   2. Edit this .txt under MS-Excel / D2Excel / other spreadsheet
   3. Save >>>and close<<< the file (MS-Excel problem)
   4. Launch Pack.bat

You'll have a new AnimData.d2 wich have your modifications. Use this
new .d2 file into your Mod.




CHANGES :
=========

9th February 2005 : version 2
   - Removes the 5000 records limit.
   - AnimData.txt records are now sorted by COF filenames
   - check for duplicate COF entries in AnimData.d2 when creating it
   - Changed almost completly the program sources (animdata_edit.c)
 
1st August 2002
   - 1st release




NOTE !
======
In D2Exp.mpq, AnimData.d2 already contains some duplicated COF
entries. When packing back the original datas, here is the output
of the program :


   animdata_edit v. 2
   3558 records writen into AnimData.d2

   warning : '3DNUHTH' is present more than once in AnimData.d2
   warning : '64A1HTH' is present more than once in AnimData.d2
   warning : '64NUHTH' is present more than once in AnimData.d2
   warning : 'AITN1HS' is present more than once in AnimData.d2
   warning : 'AITN1HT' is present more than once in AnimData.d2
   warning : 'AITN2HS' is present more than once in AnimData.d2
   warning : 'AITN2HT' is present more than once in AnimData.d2
   warning : 'AITNBOW' is present more than once in AnimData.d2
   warning : 'AITNHT1' is present more than once in AnimData.d2
   warning : 'AITNHT2' is present more than once in AnimData.d2
   warning : 'AITNHTH' is present more than once in AnimData.d2
   warning : 'AITNSTF' is present more than once in AnimData.d2
   warning : 'AITNXBW' is present more than once in AnimData.d2
   warning : 'DZTN1HS' is present more than once in AnimData.d2
   warning : 'DZTN1HT' is present more than once in AnimData.d2
   warning : 'DZTN2HS' is present more than once in AnimData.d2
   warning : 'DZTN2HT' is present more than once in AnimData.d2
   warning : 'DZTNBOW' is present more than once in AnimData.d2
   warning : 'DZTNHTH' is present more than once in AnimData.d2
   warning : 'DZTNSTF' is present more than once in AnimData.d2
   warning : 'DZTNXBW' is present more than once in AnimData.d2
   warning : 'MINUHTH' is present more than once in AnimData.d2
   warning : 'VMA1HTH' is present more than once in AnimData.d2
   warning : 'VMDDHTH' is present more than once in AnimData.d2
   warning : 'VMDTHTH' is present more than once in AnimData.d2
   warning : 'VMGHHTH' is present more than once in AnimData.d2
   warning : 'VMNUHTH' is present more than once in AnimData.d2
   warning : 'VMS1HTH' is present more than once in AnimData.d2
   warning : 'VMWLHTH' is present more than once in AnimData.d2
