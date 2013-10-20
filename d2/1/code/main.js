
/*********************************
		Tick管理器
***********************************/

//pacman 2013-10-13 构造
function TickMgr()
{
	this._intervalID = undefined;			//intevalID
	this._tickNo = 0;
	this._interval = undefined;
	this._arrInvalidateList = new Array();			//失效列表
	this._arrTickCbList = new Array();			//Tick回调列表

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
	this._tickNo ++;
	var tick = {tickNo: this._tickNo};		//传递给onTickListener的Tick对象

	//先执行invalidate回调
	//因为invalidate回调与基本属性 计算有关，需要先计算
	for (var i = 0 ; i < this._arrInvalidateList.length; i ++)
	{
		if (this._arrInvalidateList[i] != undefined)
		{
			//执行回调
			this._arrInvalidateList[i].invalidateHandler();
		}
	}
	
	//清空invalidate回调列表
	this._arrInvalidateList = new Array();


	//执行tick回调
	for (var j = 0 ; j < this._arrTickCbList.length; j ++)
	{
		var cbPair = this._arrTickCbList[j];
		if(cbPair.cbCaller != undefined && cbPair.cbFunc != undefined)
		{
			cbPair.cbFunc.call(cbPair.cbCaller, tick);
		}
	}

}

//pacman 2013-10-13 注册invalidate对象
TickMgr.prototype.regInvalid = function(invalidater)
{
	if(this._arrInvalidateList.indexOf(invalidater) == -1)
	{
		this._arrInvalidateList.push(invalidater);
	}
}

//pacman 2013-10-19 启动tick
TickMgr.prototype.clearAndStartTick = function(interval)
{
	if (this._intervalID != undefined)
	{
		clearInterval(this._intervalID);
		this._tickNo = 0;
	}
	this._intervalID = setInterval("TickMgr.instance.tick();", interval);		//启动tick
}

//pacman 2013-10-19 暂停tick
TickMgr.prototype.pauseTick = function()
{
}

//pacman 2013-10-19 继续tick
TickMgr.prototype.resumeTick = function()
{
}

//pacman 2013-10-19 添加Tick回调
//pacman 2013-10-19 测试重复添加 -ok- 多次执行不会重复添加
TickMgr.prototype.addTickListener = function(cbCaller, cbFunc)
{
	//检查是否已经添加
	for (var i =0; i < this._arrTickCbList.length;  i ++)
	{
		if (this._arrTickCbList[i].cbCaller == cbCaller && this._arrTickCbList[i].cbFunc == cbFunc)
		{
			return;
		}
	}

	//添加
	var cbPair = new Object();
	cbPair.cbCaller = cbCaller;
	cbPair.cbFunc = cbFunc;
	this._arrTickCbList.push(cbPair);
}

//pacman 2013-10-19 移除Tick回调
//pacman 2013-10-19 测试移除	-ok-
TickMgr.prototype.removeTickListener = function(cbCaller, cbFunc)
{
	for (var i = this._arrTickCbList.length - 1; i >= 0 ;  i --)
	{
		if (this._arrTickCbList[i].cbCaller == cbCaller && this._arrTickCbList[i].cbFunc == cbFunc)
		{
			this._arrTickCbList.splice(i, 1);
		}
	}


}


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

//--角色模板数据 (数组)
var T_CHAR = new Array();
//亚马逊
T_CHAR[AMA] = new Object();		
T_CHAR[AMA].INIT_STR = 20;			//起始力量
T_CHAR[AMA].INIT_DEX = 25;			//起始敏捷
T_CHAR[AMA].INIT_VIT = 20;			//起始活力
T_CHAR[AMA].INIT_ENG = 15;			//起始能量
T_CHAR[AMA].INIT_LIFE = 50;			//起始生命
T_CHAR[AMA].LIFE_PER_LV = 2;			//每升一级增加的生命
T_CHAR[AMA].MANA_PER_LV = 1.5;			//每升一级增加的魔法
T_CHAR[AMA].LIFE_PER_VIT = 3;			//每点活力增加的生命
T_CHAR[AMA].MANA_PER_ENG = 1.5;		//每点能量增加的魔法


/*********************************
			角色类
***********************************/
//pacman 2013-10-19 使用失效和手动激活临近节点连锁重算的功能来优化数据重算
//为了图省事，没写成员属性的getter

//pacman 2013-10-19  构造
function Character(class_)
{
	//成员变量声明
	this._class = class_;	//职业
	this._name = "";	//名称
	this._lv = 0;		//等级
	
	//--力量 ---
	this._totalStr = 0;		//总力量
	this._basicStr = 0;		//基础力量		
	this._bonusStr = 0;		//力量加成
	this._pctBonusStr = 0;	//（百分比）力量加成
	this._toRecalTotalStr = false;		//是否将要重算总力量
	
	//--敏捷 ---
	this._totalDex = 0;		//总敏捷
	this._basicDex = 0;		//基础敏捷
	this._bonusDex = 0;		//敏捷加成
	this._pctBonusDex = 0;	//（百分比）敏捷加成
	this._toRecalTotalDex = false;		//是否将要重算总敏捷
	
	//-- 活力 ---
	this._totalVit = 0;			//总活力
	this._basicVit = 0;			//基础活力
	this._bonusVit = 0;			//活力加成
	this._pctBonusVit = 0;		//活力加成(百分比)
	this._toRecalTotalVit = false;		//将要重算总活力
	
	//-- 能量 ---
	this._totalEng = 0;			//总能量
	this._basicEng = 0;			//基础能量
	this._bonusEng = 0;			//能量加成
	this._pctBonusEng = 0;		//（百分比）能量加成
	this._toRecalTotalEng = false;		//将要重算总能量		
	
	//-- 生命 ---
	this._totalLife = 0;			//总生命
	this._basicLife = 0;			//基础生命
	this._bonusLife = 0;			//生命加成
	this._pctBonusLife = 0;			//生命加成(百分比)
	this._toRecalBasicLife = false;		//将要重算基础生命
	this._toRecalTotalLife = false;		//将要重算总生命
	
	//-- 魔法 ---
	this._totalMana = 0;			//总魔法
	this._basicMana = 0;			//基础魔法		
	this._bonusMana = 0;		//魔法加成
	this._pctBonusMana = 0;		//（百分比）魔法加成
	this._toRecalBasicMana = false;		//是否将要重算基础魔法
	this._toRecalTotalMana = false;		//是否将要重算总魔法	
	
	//初始设置
	this.setLv(1);
	this.setBasicVit(T_CHAR[this._class].INIT_VIT);
}

//---------------------
//-- 等级计算 ---
//---------------------
//设置lv
Character.prototype.setLv = function(value)
{
	this._lv = value;
	this.invalidate();

	//激活相邻节点重算	
	this.toRecalBasicLife();
}


//---------------------
//-- 力量计算 ---
//---------------------
//设置基础力量
Character.prototype.setBasicStr = function(value)
{
	this._basicStr = value;
	this.invalidate();
	
	//连锁重算	
	this.toRecalTotalStr();
}

//准备重算总力量
Character.prototype.toRecalTotalStr = function(value)
{
	this._toRecalTotalStr = true;
	
	//连锁重算
}

//重算总力量
Character.prototype.updateTotalStr = function()
{
	this._totalStr = this._basicStr * (1 + this._pctBonusStr) + this._bonusStr;
	this._toRecalTotalStr = false;	
}


//---------------------
//-- 敏捷计算 ---
//---------------------
//设置基础敏捷
Character.prototype.setBasicDex = function(value)
{
	this._basicDex = value;
	this.invalidate();
	
	//连锁
	this.toRecalTotalDex();	
}

//准备重算总敏捷
Character.prototype.toRecalTotalDex = function()
{
	this._toRecalTotalDex = true;
		
	//连锁
}

//重算总敏捷
Character.prototype.updateTotalDex = function()
{
	this._totalDex = this._basicDex * (1 + this._pctBonusDex) + this._bonusDex;
	this._toRecalTotalDex  = false;
}


//---------------------
//-- 活力计算 ---
//---------------------
//设置基础vit
Character.prototype.setBasicVit = function(value)
{
	this._basicVit = value;
	this.invalidate();	
	
	//连锁
	this.toRecalTotalVit();
}

//准备重算总活力
Character.prototype.toRecalTotalVit = function()
{
	this._toRecalTotalVit = true;

	//连锁
	this.toRecalBasicLife();
}

//重算总活力
Character.prototype.updateTotalVit = function()
{
	this._totalVit = this._basicVit  * (1 + this._pctBonusVit) + this._bonusVit;
	this._toRecalTotalVit = false;	
}

//---------------------
//-- 能量计算 ---
//---------------------
//设置基础能量
Character.prototype.setBasicEng = function(value)
{
	this._basicEng = value;
	this.invalidate();

	//连锁
	this.toRecalTotalEng();
}

//准备重算总能量
Character.prototype.toRecalTotalEng = function()
{
	this._toRecalTotalEng = true;

	//连锁
}

//重算总能量
Character.prototype.updateTotalEng = function()
{
	this._totalEng = this._basicEng * (1 + this._pctBonusEng) + this._bonusEng;
	this._toRecalTotalEng = false;
}

//---------------------
//-- 生命计算 ---
//---------------------
//准备重算基本生命
Character.prototype.toRecalBasicLife = function()
{
	this._toRecalBasicLife = true;

	//连锁
	this.toRecalTotalLife();
}

//重算基本生命
Character.prototype.updateBasicLife = function()
{
	this._basicLife = T_CHAR[AMA].INIT_LIFE + (this._lv - 1) * T_CHAR[AMA].LIFE_PER_LV + this._totalVit * T_CHAR[AMA].LIFE_PER_VIT;
	this._toRecalBasicLife = false;
}

//准备重算总生命
Character.prototype.toRecalTotalLife = function()
{
	this._toRecalTotalLife = true;
	
	//连锁
}

//重算总生命
Character.prototype.updateTotalLife = function()
{
	this._totalLife = this._basicLife + this._bonusLife + this._pctBonusLife;
	this._toRecalTotalLife = false;
}

//---------------------
//-- 魔法计算 ---
//---------------------
//todo


//---------------------
//-- 总重算 ---
//---------------------
Character.prototype.update = function()
{
	//alert("update");
	
	//总力量
	if (this._toRecalTotalStr)	this.updateTotalStr();
	
	//总敏捷
	if (this._toRecalTotalDex)	this.updateTotalDex();

	//总体力
	if (this._toRecalTotalVit)	this.updateTotalVit();
	
	//总能量
	if (this._toRecalTotalEng) this.updateTotalEng();

	//基本生命
	if(this._toRecalBasicLife)	this.updateBasicLife();
	
	//总生命
	if(this._toRecalTotalLife)	this.updateTotalLife();
	
	
}

//----接口 IInvalidate---
//invalidate (可放在基类)
Character.prototype.invalidate = function()
{
	TickMgr.instance.regInvalid(this);
}

Character.prototype.invalidateHandler = function()
{
	this.update();
}
//end IInvalidate

