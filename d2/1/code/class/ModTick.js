
/*********************************
		Tick管理器
***********************************/
//TickMgr的核心是一个计时管理器，提供主要功能有
//tick回调注册、执行
//html5有一个基于帧频的回调管理机制，但缺少一个基于时间的
//我用TickMgr实现基于时间的类似as3的invalidate功能
//最大的用途在于优化数据对象的重复数值计算，

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
	使用TickInvalidate功能的基类
***********************************/
//pacman 2013-10-27 要使用TickMgr的invalidate功能，继承该类

//构造
function TickInvalidateObject()
{
}

//----接口 IInvalidate---
//pacman 2013-10-27 注册invalidate
TickInvalidateObject.prototype.invalidate = function()
{
	TickMgr.instance.regInvalid(this);
}

//pacman 2013-10-27 invalidate回调
TickInvalidateObject.prototype.invalidateHandler = function()
{
}
//end IInvalidate
