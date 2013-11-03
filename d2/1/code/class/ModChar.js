//include game_const.js
//include util.js
//include t_char.js
//include t_item.js


/*********************************
			角色类
***********************************/
//pacman 2013-10-19 使用失效和手动激活临近节点连锁重算的功能来优化数据重算
//为了图省事，没写成员属性的getter

//pacman 2013-10-19  构造
function Character(class_)
{
	//继承TickInvalidateObject
	TickInvalidateObject.call(this);

	//成员变量声明
	this._TData = undefined;		//模板数据		
	this._class = undefined;		//职业
	this._name = "";	//名称
	this._lv = 0;		//等级
	this._freeAttribPoints = 0;		//自由分配点数
	
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
	
	//-- 体力 ---
	this._totalVit = 0;			//总体力
	this._basicVit = 0;			//基础体力
	this._bonusVit = 0;			//体力加成
	this._pctBonusVit = 0;		//体力加成(百分比)
	this._toRecalTotalVit = false;		//将要重算总体力
	
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

	/////////////////////////////////////////
	//-- 命中 防御  --
	/////////////////////////////////////////
	//攻击准确度
	this._totalAttackRating = 0;		//总准确度
	this._basicAttackRating = 0;		//基本准确度
	this._bonusAttackRating = 0;		//准确度加成
	this._pctBonusAttackRating = 0;		//（百分比）准确度加成
	this._toRecalBasicAttackRating = false;		//是否将要重算基本准确度
	this._toRecalTotalAttackRating = false;		//是否将要重算总准确度

	//防御率
	this._totalDefenseRating = 0;		//总防御率
	this._basicDefenseRating = 0;		//基本防御率
	this._bonusDefenseRating = 0;		//防御率加成
	this._pctBonusDefenseRating = 0;	//（百分比）防御率加成
	this._toRecalBasicDefenseRating = false;		//是否将要重算基础防御率
	this._toRecalTotalDefenseRating = false;		//是否将要重算总防御率

	/////////////////////////////////////////
	//-- 攻击输出 --
	/////////////////////////////////////////
	//属性加成伤害值
	this._pctStatsBonusDamage = 0;				//（百分比）属性点对伤害的加成
	this._toRecalStatsBonusDamage = false;		//是否将要重算属性点对伤害的加成

	//-- 最小伤害 ---
	this._minDamage = 0;			//最小伤害最终值 
	this._bonusMinDamage = 0;		//最小伤害加成
	this._pctBonusMinDamage = 0;	//(百分比)最小伤害加成
	this._toRecalMinDamage = false;		//是否将要重算最小伤害

	//-- 最大伤害 ---
	this._maxDamage = 0;			//最大伤害最终值 
	this._bonusMaxDamage = 0;		//最大伤害加成
	this._pctBonusMaxDamage = 0;	//(百分比)最大伤害加成
	this._toRecalMaxDamage = false;		//是否将要重算最大伤害


	/////////////////////////////////////////
	//-- 装备数据 ---
	/////////////////////////////////////////
	this._equiptmentSlots = new Array();		//装备栏容器


	/////////////////////////////////////////
	//-- init ---
	/////////////////////////////////////////
	this.resetWithClass(class_);			//重置属性
	//-- 装备 ---
	this.wearHandItem(null, EQUIPMENT_SLOT.LEFT_HAND);
	this.wearHandItem(null, EQUIPMENT_SLOT.RIGHT_HAND);

}

//继承TickInvalidateObject
Character.prototype = new TickInvalidateObject();


//---------------------
//-- reset ---
//---------------------
//pacman 2013-10-20 按照指定的职业重置数据
//其实就是setClass ，但是setClass这个动作比较特殊，游戏逻辑上某个角色不可以中途改变class
//因此更换职业必须同时数据重置，所以叫resetWithClass
//这个函数更多是为了重复使用Character对象而设立的
Character.prototype.resetWithClass = function(class_)
{
	this._class = class_;
	this._TData = T_CHAR[this._class];
	this.setLv(1);
	//只手动重置底层值，其余高级值的重置将自动被连锁执行
	this.setBasicStr(this._TData.INIT_STR);	
	this.setBasicDex(this._TData.INIT_DEX);	
	this.setBasicVit(this._TData.INIT_VIT);	
	this.setBasicEng(this._TData.INIT_ENG);	

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
	this.toRecalBasicMana();
}

//pacman 2013-10-22 升级
Character.prototype.addLv = function(value)
{
	if(!value) value = 1;

	//奖励自由点数
	this._freeAttribPoints += value * GAME_CONST.ATTRIBPOINTS_PER_LV;

	//升级
	this.setLv(this._lv + value);
}

//---------------------
//-- 力量计算 ---
//---------------------
//从自由点数中给基础力量配点
Character.prototype.addBasicStr = function(value)
{
	if(!value) value = 1;

	//超限
	if (value > this._freeAttribPoints) return;
	else
	{
		this._freeAttribPoints -= value;
		this.setBasicStr(this._basicStr + value);
	}
}

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
	this.toRecalStatsBonusDamage();
}

//重算总力量
Character.prototype.updateTotalStr = function()
{
	this._totalStr = (this._basicStr + this._bonusStr) * (1 + this._pctBonusStr) ;
	this._totalStr = Util.roundTo(this._totalStr, GAME_CONST.NUM_DIGITS);
	this._toRecalTotalStr = false;	
}


//---------------------
//-- 敏捷计算 ---
//---------------------
//从自由点数中给基础敏捷配点
Character.prototype.addBasicDex = function(value)
{
	if(!value) value = 1;

	//超限
	if (value > this._freeAttribPoints) return;
	else
	{
		this._freeAttribPoints -= value;
		this.setBasicDex(this._basicDex + value);
	}
}

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
	this.toRecalStatsBonusDamage();
	this.toRecalBasicAttackRating();	
	this.toRecalBasicDefenseRating();
}

//重算总敏捷
Character.prototype.updateTotalDex = function()
{
	this._totalDex = (this._basicDex + this._bonusDex) * (1 + this._pctBonusDex);
	this._totalDex = Util.roundTo(this._totalDex, GAME_CONST.NUM_DIGITS);
	this._toRecalTotalDex  = false;
}


//---------------------
//-- 体力计算 ---
//---------------------
//从自由点数中给基础体力配点
Character.prototype.addBasicVit = function(value)
{
	if(!value) value = 1;

	//超限
	if (value > this._freeAttribPoints) return;
	else
	{
		this._freeAttribPoints -= value;
		this.setBasicVit(this._basicVit + value);
	}
}

//设置基础vit
Character.prototype.setBasicVit = function(value)
{
	this._basicVit = value;
	this.invalidate();	
	
	//连锁
	this.toRecalTotalVit();
}

//准备重算总体力
Character.prototype.toRecalTotalVit = function()
{
	this._toRecalTotalVit = true;

	//连锁
	this.toRecalBasicLife();
}

//重算总体力
Character.prototype.updateTotalVit = function()
{
	this._totalVit = (this._basicVit + this._bonusVit) * (1 + this._pctBonusVit);
	this._totalVit = Util.roundTo(this._totalVit, GAME_CONST.NUM_DIGITS);
	this._toRecalTotalVit = false;
}

//---------------------
//-- 能量计算 ---
//---------------------
//从自由点数中给基础能量配点
Character.prototype.addBasicEng = function(value)
{
	if(!value) value = 1;

	//超限
	if (value > this._freeAttribPoints) return;
	else
	{
		this._freeAttribPoints -= value;
		this.setBasicEng(this._basicEng + value);
	}
}

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
	this.toRecalBasicMana();
}

//重算总能量
Character.prototype.updateTotalEng = function()
{
	this._totalEng = (this._basicEng + this._bonusEng) * (1 + this._pctBonusEng);
	this._totalEng = Util.roundTo(this._totalEng, GAME_CONST.NUM_DIGITS);
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
	this._basicLife = this._TData.INIT_LIFE + (this._lv - 1) * this._TData.LIFE_PER_LV + (this._totalVit -  this._TData.INIT_VIT) * this._TData.LIFE_PER_VIT;
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
	this._totalLife = (this._basicLife + this._bonusLife) * (1 + this._pctBonusLife);
	this._totalLife = Util.roundTo(this._totalLife, GAME_CONST.NUM_DIGITS);
	this._toRecalTotalLife = false;
}

//---------------------
//-- 魔法计算 ---
//---------------------
//准备重算基础魔法
Character.prototype.toRecalBasicMana = function()
{
	this._toRecalBaiscMana = true;

	//连锁
	this.toRecalTotalMana();
}

//重算基础魔法
Character.prototype.updateBasicMana = function()
{
	this._basicMana = this._TData.INIT_MANA + (this._lv - 1) * this._TData.MANA_PER_LV + (this._totalEng - this._TData.INIT_ENG) * this._TData.MANA_PER_ENG;
	this._toRecalBaiscMana = false;
}

//准备重算总魔法
Character.prototype.toRecalTotalMana = function()
{
	this._toRecalTotalMana = true;

	//连锁
}

//重算总魔法
Character.prototype.updateTotalMana = function()
{
	this._totalMana = (this._basicMana + this._bonusMana) *(1 + this._pctBonusMana);
	this._totalMana = Util.roundTo(this._totalMana, GAME_CONST.NUM_DIGITS);
	this._toRecalTotalMana = false;
}

/////////////////////////////////////////
//-- 命中 防御  --
/////////////////////////////////////////
//---------------------
//-- 命中 ---
//---------------------
//pacman 2013-10-27 准备重算基础准确度
Character.prototype.toRecalBasicAttackRating = function()
{
	this._toRecalBasicAttackRating = true;

	//连锁
	this.toRecalTotalAttackRating();
}

//pacman 2013-10-27 重算基础准确度
Character.prototype.updateBasicAttackRating = function()
{
	this._basicAttackRating = (this._totalDex * 5) - 35 + T_CHAR[this._class].ATTACK_RATING_FACTOR;

	this._toRecalBasicAttackRating = false;	
}


//pacman 2013-10-27 准备重算总准确度
Character.prototype.toRecalTotalAttackRating = function()
{
	this._toRecalTotalAttackRating = true;

	//连锁
}

//pacman 2013-10-27 重算总准确度
Character.prototype.updateTotalAttackRating = function()
{
	this._totalAttackRating = (this._basicAttackRating + this._bonusAttackRating) * (1 + this._pctBonusAttackRating);
	this._totalAttackRating = Util.roundTo(this._totalAttackRating, GAME_CONST.NUM_DIGITS);

	this._toRecalTotalAttackRating = false;
}

//---------------------
//-- 防御 ---
//---------------------
//pacman 2013-11-2 准备重算基础防御率
Character.prototype.toRecalBasicDefenseRating = function()
{
	this._toRecalBasicDefenseRating = true;

	//连锁
	this.toRecalTotalDefenseRating();
}

//pacman 2013-11-2 重算基础防御率
Character.prototype.updateBasicDefenseRating = function()
{
	this._basicDefenseRating = this._totalDex / 4;
	this._basicDefenseRating = Util.roundDownTo(this._basicDefenseRating, 0.01);

	this._toRecalBasicDefenseRating = false;
}

//pacman 2013-11-2 准备重算总防御率
Character.prototype.toRecalTotalDefenseRating = function()
{
	this._toRecalTotalDefenseRating = true;

	//连锁
}

//pacman 2013-11-2 重算总防御率
Character.prototype.updateTotalDefenseRating = function()
{
	this._totalDefenseRating = (this._basicDefenseRating + this._bonusDefenseRating) * (1 + this._pctBonusDefenseRating);
	this._totalDefenseRating = Util.roundTo(this._totalDefenseRating, 0.01);

	this._toRecalTotalDefenseRating = false;
}


////////////////////////////
//----装备---
////////////////////////////
//-- 武器及伤害输出 ---
//pacman 2013-10-25 检定是否可装备指定武器
//Character.prototype.ifCanWearWeapon = function(weaponR){}

//pacman 2013-10-26 装备手部道具
//这个函数的细节还没有想清楚，但是觉得结构上需要这么个函数
Character.prototype.wearHandItem = function(itemR, handSlot)
{

	//检查是否可以装备该道具

	//脱下旧装备，换上新装备

	this.setWeapon(itemR, handSlot);

}

//pacman 2013-10-25 设置武器	# IWearWeapon
Character.prototype.setWeapon = function(weaponR, handSlot)
{
	if (handSlot != EQUIPMENT_SLOT.LEFT_HAND && handSlot != EQUIPMENT_SLOT.RIGHT_HAND)
	{
		console.log("invalid handSlot");
		return;
	}

	this._equiptmentSlots[handSlot] = weaponR;
	this.invalidate();

	//连锁
	this.toRecalStatsBonusDamage();
	this.toRecalMinDamage();
	this.toRecalMaxDamage();
}

//pacman 2013-10-26 获得当前武器	# IWearWeapon
//我们先从简单的情况开始，假设玩家只持有一把武器
Character.prototype.getWeapon = function()
{
	//检查左右手获得当前持有的武器
	//先左后右
	var weaponR;
	var arrHandSlotID = [EQUIPMENT_SLOT.LEFT_HAND, EQUIPMENT_SLOT.RIGHT_HAND];
	for (var i = 0; i < arrHandSlotID.length; i ++)
	{
		weaponR = this._equiptmentSlots[arrHandSlotID[i]];
		if(weaponR && weaponR.getItemClass() == ITEM_CLASS.WEAPON )
		{
			break;;
		}
	}
	
	return weaponR;
}

//pacman 2013-10-26 准备重算属性加成伤害
Character.prototype.toRecalStatsBonusDamage = function()
{
	this._toRecalStatsBonusDamage = true;

	//连锁
	this.toRecalMinDamage();
	this.toRecalMaxDamage();
}

//pacman 2013-10-26 重算属性加成伤害
Character.prototype.updateStatsBonusDamage = function()
{
	var weaponR = this.getWeapon();

	//空手
	if (!weaponR || weaponR.getItemClass() != ITEM_CLASS.WEAPON)
	{
		this._pctStatsBonusDamage = 0;
	}
	else
	{
		//不同的武器类型有不同的计算公式
		switch(weaponR.getWeaponType())
		{
			case WEAPON_TYPE.AXE:
				this._pctStatsBonusDamage = Util.roundDownTo(this._totalStr / 100, GAME_CONST.NUM_DIGITS);	//先上舍入，精度0.01
				break;
		}
	}

	this._toRecalStatsBonusDamage = false;
}


//pacman 2013-10-26 准备重算最小伤害输出
Character.prototype.toRecalMinDamage = function()
{
	this._toRecalMinDamage = true;

	//连锁
}

//pacman 2013-10-26 重算最小伤害输出
Character.prototype.updateMinDamage = function()
{
	var weaponR = this.getWeapon();		//当前武器
	var weaponMinDamage;				//当前武器最小伤害
	if (weaponR && weaponR.getItemClass() == ITEM_CLASS.WEAPON)
	{
		weaponMinDamage = weaponR.getMinDamage();
	}
	else
	{
		weaponMinDamage = BARE_HAND_DAMAGE_MIN;
	}

	this._minDamage = (weaponMinDamage + this._bonusMinDamage) * (1 + this._pctStatsBonusDamage + this._pctBonusMinDamage); 
	this._minDamage = Util.roundTo(this._minDamage, GAME_CONST.NUM_DIGITS);		//pacman 2013-10-27 1.2竟然被算成了1.99999999，只好做一下处理

	this._toRecalMinDamage = false;
}


//pacman 2013-10-26 准备计算最大伤害输出
Character.prototype.toRecalMaxDamage = function()
{
	this._toRecalMaxDamage = true;

	//连锁
}

//pacman 2013-10-26 重算最大伤害输出
Character.prototype.updateMaxDamage = function()
{
	var weaponR = this.getWeapon();		//当前武器
	var weaponMaxDamage;				//当前武器最大伤害
	if (weaponR && weaponR.getItemClass() == ITEM_CLASS.WEAPON)
	{
		weaponMaxDamage = weaponR.getMaxDamage();
	}
	else
	{
		weaponMaxDamage = BARE_HAND_DAMAGE_MAX;
	}

	this._maxDamage = (weaponMaxDamage + this._bonusMaxDamage) * (1 + this._pctStatsBonusDamage + this._pctBonusMaxDamage); 
	this._maxDamage = Util.roundTo(this._maxDamage, GAME_CONST.NUM_DIGITS);		//pacman 2013-10-27 1.2竟然被算成了1.99999999，只好做一下处理

	this._toRecalMaxDamage = false;
}


//---------------------
//-- 总重算 ---
//---------------------
Character.prototype.update = function()
{
	//alert("update");
	//pacman 2013-10-20 必须按照先底层后高级的顺序逐步计算各种属性

	//总力量
	if (this._toRecalTotalStr)	this.updateTotalStr();
	
	//总敏捷
	if (this._toRecalTotalDex)	this.updateTotalDex();

	//总体力
	if (this._toRecalTotalVit)	this.updateTotalVit();
	
	//总能量
	if (this._toRecalTotalEng) this.updateTotalEng();

	//基本生命
	if (this._toRecalBasicLife)	this.updateBasicLife();
	
	//总生命
	if (this._toRecalTotalLife)	this.updateTotalLife();

	//基本魔法
	if (this._toRecalBaiscMana) this.updateBasicMana();

	//总魔法
	if (this._toRecalTotalMana) this.updateTotalMana();
	
	//-- 攻击准确度 ---
	//基础攻击准确度
	if (this._toRecalBasicAttackRating) this.updateBasicAttackRating();

	//总攻击准确度
	if (this._toRecalTotalAttackRating) this.updateTotalAttackRating();	

	//-- 防御率 ---
	//基础防御率
	if (this._toRecalBasicDefenseRating) this.updateBasicDefenseRating();

	//总防御率
	if (this._toRecalTotalDefenseRating) this.updateTotalDefenseRating();

	//-- 伤害输出 ---
	//属性加成伤害
	if (this._toRecalStatsBonusDamage) this.updateStatsBonusDamage();
	//最小伤害
	if (this._toRecalMinDamage)	this.updateMinDamage();
	//最大伤害
	if (this._toRecalMaxDamage) this.updateMaxDamage();

}


//---override TickInvalidateObject 相关方法---
//pacman 2013-10-27 javascript中只能彻底覆盖超类方法
//暂时还不知道怎么在override的时候调用超类方法
Character.prototype.invalidateHandler = function()
{
	this.update();
}

