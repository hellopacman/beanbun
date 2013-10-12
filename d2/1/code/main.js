var out = document.getElementById("out");
/**************
Tick管理器
****************/
var TICK_INTERVAL = 40;		//tick间隔(毫秒)

//pacman 2013-10-13 构造
function TickMgr()
{
	this._invalidateList = new Array();
	
	
	
	this.test = 0;
}

//pacman 2013-10-13 单例
TickMgr.instance = new TickMgr();

//pacman tick
TickMgr.prototype.tick = function()
{
	//pacman 2013-10-13 很坑爹，当这个函数由setInterval或者setTimeout所执行时
	//this指向的是window，而不是TickMgr.instance
	//解决的办法是setInerval的时候调用"TickMgr.instance.tick();"
	//而不是TickMgr.instance.tick
	this.test ++;
	out.innerHTML = "tick: " + this.test;
	
	//执行invalidate回调
	for (var i = 0 ; i < this._invalidateList.length; i ++)
	{
		if (this._invalidateList[i] != undefined)
		{
			//执行回调
			this._invalidateList[i].onTick();
		}
	}
	
	//清空回调列表
	this._invalidateList = new Array();
}

//pacman 2013-10-13 注册invalidate对象
TickMgr.prototype.regInvalid = function(invalidater)
{
	if(this._invalidateList.indexOf(invalidater) == -1)
	{
		this._invalidateList.push(invalidater);
	}
}

setInterval("TickMgr.instance.tick();", TICK_INTERVAL);

/**************
角色
****************/
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

//----接口 IInvalidate---
//invalidate (可放在基类)
Character.prototype.invalidate = function()
{
	TickMgr.instance.regInvalid(this);
}

Character.prototype.onTick = function()
{
	this.updateAll();
}
//end IInvalidate



//设置class
Character.prototype.setClass = function(class_)
{
	this._class_ = class_;
	
	this.invalidate();
}

//-- 更新链路
//全部更新
Character.prototype.updateAll = function()
{
	alert("updateAll");
}


