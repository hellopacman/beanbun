
/*********************************
		道具模板数据
***********************************/

//道具类型枚举
var ITEM_CLASS = new Object();
var i = 0;
ITEM_CLASS.WEAPON = i ++;	//道具类型：武器
ITEM_CLASS.ARMOR = i ++;	//道具类型：护甲

//装备槽位枚举
var EQUIPMENT_SLOT = new Object();
var i = 0;
EQUIPMENT_SLOT.LEFT_HAND = i ++;		//左手位置
EQUIPMENT_SLOT.RIGHT_HAND = i ++;		//右手位置
EQUIPMENT_SLOT.BODY = i ++;			//躯干位置


//武器门类枚举
WEAPON_CLASS = new Object();
var i = 0;
WEAPON_CLASS.MELEE = i ++;		//肉搏武器
WEAPON_CLASS.THROW = i ++;		//投掷武器
WEAPON_CLASS.BOWS = i ++;		//弓
WEAPON_CLASS.CROSSBOWS = i ++;	//弩

//武器类型枚举
WEAPON_TYPE = new Object();
var i = 0;
WEAPON_TYPE.AXE = i ++;			//斧子

//--武器模板数据
var T_WEAPON_LIST = new Object();
var i = 0;		//自增索引
//单手斧
T_WEAPON_LIST[i] = new Object();
T_WEAPON_LIST[i].TID  = i;							//道具TID
T_WEAPON_LIST[i].NAME  = "1 Handed Axe";				//名称
T_WEAPON_LIST[i].ITEM_CLASS = ITEM_CLASS.WEAPON;		//道具类型
T_WEAPON_LIST[i].WEAPON_CLASS = WEAPON_CLASS.MELEE;	//武器门类
T_WEAPON_LIST[i].WEAPON_TYPE = WEAPON_TYPE.AXE;		//武器类型
T_WEAPON_LIST[i].MIN_DAMAGE = 3;						//最小伤害
T_WEAPON_LIST[i].MAX_DAMAGE = 6;						//最大伤害
T_WEAPON_LIST[i].MIN_STR = 0;						//力量需求
T_WEAPON_LIST[i].MIN_DEX = 0;						//敏捷需求
T_WEAPON_LIST[i].DURABLITY = 28;						//耐久度
T_WEAPON_LIST[i].QUALITY_LEVEL = 3;					//品质等级

