
/*********************************
		角色模板数据
***********************************/
//角色职业编号 (从0开始的正整数)
var i = 0;
var AMA = i ++;		//亚马逊
var SOR = i ++;		//女巫
var NEC = i ++;		//男巫
var PAL = i ++;			//圣骑士
var BAR = i ++;		//野蛮人
var DRU = i ++;		//德鲁伊
var ASN = i ++;		//刺客

//pacmna 空手min/max伤害
//这个数值是我杜撰的
var BARE_HAND_DAMAGE_MIN = 1;		
var BARE_HAND_DAMAGE_MAX = 2;


//--角色模板数据 (数组)
var T_CHAR = new Array();
//亚马逊
T_CHAR[AMA] = new Object();		
T_CHAR[AMA].INIT_STR = 20;			//起始力量
T_CHAR[AMA].INIT_DEX = 25;			//起始敏捷
T_CHAR[AMA].INIT_VIT = 20;			//起始体力
T_CHAR[AMA].INIT_ENG = 15;			//起始能量
T_CHAR[AMA].INIT_LIFE = 50;			//起始生命
T_CHAR[AMA].INIT_MANA = 15;			//起始魔法
T_CHAR[AMA].LIFE_PER_LV = 2;			//每升一级增加的生命
T_CHAR[AMA].MANA_PER_LV = 1.5;			//每升一级增加的魔法
T_CHAR[AMA].LIFE_PER_VIT = 3;			//每点体力增加的生命
T_CHAR[AMA].MANA_PER_ENG = 1.5;		//每点能量增加的魔法
T_CHAR[AMA].ATTACK_RATING_FACTOR = 5;	//攻击准确度补偿

