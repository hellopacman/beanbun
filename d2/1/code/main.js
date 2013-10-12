//角色职业编号 (从0开始的正整数)
var i = 0;
var AMA = i ++;		//亚马逊
var SOR = i ++;		//女巫
var NEC = i ++;		//男巫
var PAL = i ++;		//圣骑士
var BAR = i ++;		//野蛮人
var DRU = i ++;		//德鲁伊
var ASN = i ++;		//刺客

//--角色模板数据 (数组)
var T_CHRTR = new Array();
//亚马逊
T_CHRTR[AMA] = new Object();		
T_CHRTR[AMA].initStr = 20;			//起始力量
T_CHRTR[AMA].initDex = 25;			//起始敏捷
T_CHRTR[AMA].initVit = 20;			//起始活力
T_CHRTR[AMA].initEng = 15;			//起始能量
T_CHRTR[AMA].initLife = 50;			//起始生命
T_CHRTR[AMA].lifePLv = 2;			//每升一级增加的生命
T_CHRTR[AMA].manaPlv = 1.5;			//每升一级增加的魔法
T_CHRTR[AMA].lifePVit = 3;			//每点活力增加的生命
T_CHRTR[AMA].manaPEng = 1.5;		//每点能量增加的魔法



//角色定义
function Character()
{
	//成员变量
	this._class_ = 0;	//职业
	this._name = "";	//名称
	this._lv = 0;		//等级
	
	this._str = 0;		//力量
	this._dex = 0;		//敏捷
	this._vit = 0;		//活力
	this._eng = 0;		//能量
	
	this._life = 0;		//生命
	this._mana = 0;		//魔法
	
}

//设置class
Character.prototype.setClass = function(class_)
{
	this._class_ = class_;
}




