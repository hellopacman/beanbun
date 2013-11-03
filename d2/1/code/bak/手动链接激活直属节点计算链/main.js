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
T_CHRTR[AMA].INIT_STR = 20;			//起始力量
T_CHRTR[AMA].INIT_DEX = 25;			//起始敏捷
T_CHRTR[AMA].INIT_VIT = 20;			//起始活力
T_CHRTR[AMA].INIT_ENG = 15;			//起始能量
T_CHRTR[AMA].INIT_LIFE = 50;			//起始生命
T_CHRTR[AMA].LIFE_PER_LV = 2;			//每升一级增加的生命
T_CHRTR[AMA].MANA_PER_LV = 1.5;			//每升一级增加的魔法
T_CHRTR[AMA].LIFE_PER_VIT = 3;			//每点活力增加的生命
T_CHRTR[AMA].MANA_PER_ENG = 1.5;		//每点能量增加的魔法



//角色定义
function Character(class_)
{
	//成员变量声明
	this._class = class_;	//职业
	this._name = "";	//名称
	this._lv = 0;		//等级
	
	//--力量 ---
	this._totalStr = 0;				//当前力量
	
	this._dex = 0;		//敏捷
	
	//-- 活力 ---
	this._totalVit = 0;			//总活力
	this._basicVit = 0;			//基础活力
	this._bonusVit = 0;			//活力加成
	this._pctBonusVit = 0;		//活力加成(百分比)
	this._toRecalTotalVit = false;		//将要重算总活力
	
	this._eng = 0;		//能量
	
	//-- 生命 ---
	this._totalLife = 0;			//总生命
	this._basicLife = 0;			//基础生命
	this._bonusLife = 0;			//生命加成
	this._pctBonusLife = 0;			//生命加成(百分比)
	this._toRecalBasicLife = false;		//将要重算基础生命
	this._toRecalTotalLife = false;		//将要重算总生命
	
	this._mana = 0;		//魔法
	
	
	//初始设置
	this.setLv(1);
	this.setBasicVit(T_CHRTR[this._class].INIT_VIT);
}

//----接口 IInvalidate---
//invalidate (可放在基类)
Character.prototype.invalidate = function()
{
	TickMgr.instance.regInvalid(this);
}

Character.prototype.onTick = function()
{
	this.update();
}
//end IInvalidate


//设置lv
Character.prototype.setLv = function(value)
{
	this._lv = value;
	this.invalidate();
	
	this.toRecalBasicLife();
}


//设置基础vit
Character.prototype.setBasicVit = function(value)
{
	this._basicVit = value;
	this.invalidate();	
	
	//连锁
	this.toRecalTotalVit();
}

//-- 计划重算
//总体力
Character.prototype.toRecalTotalVit = function()
{
	this._toRecalTotalVit = true;

	//连锁
	this.toRecalBasicLife();
}

//基本生命
Character.prototype.toRecalBasicLife = function()
{
	this._toRecalBasicLife = true;

	//连锁
	this.toRecalTotalLife();
}

//总生命
Character.prototype.toRecalTotalLife = function()
{
	this._toRecalTotalLife = true;
}


//-- 更新链路
//全部更新
Character.prototype.update = function()
{
	//alert("update");
	
	//总体力
	if(this._toRecalTotalVit)
	{
		this._totalVit = this._basicVit + this._bonusVit + this._pctBonusVit;
	}
	
	//基本生命
	if(this._toRecalBasicLife)
	{
		this._basicLife = T_CHRTR[AMA].INIT_LIFE + (this._lv - 1) * T_CHRTR[AMA].LIFE_PER_LV + this._totalVit * T_CHRTR[AMA].LIFE_PER_VIT;
	}
	
	//总生命
	if(this._toRecalTotalLife)
	{
		this._totalLife = this._basicLife + this._bonusLife + this._pctBonusLife;
	}
	
	
}


