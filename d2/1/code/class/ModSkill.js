//include t_item.js

/*********************************
			技能实体 基类
***********************************/
//pacman 2013-11-27
//构造
//带参数的构造函数不适用于类继承结构
//pacman 2013-12-05 先凉在这里不管，先写几个技能的实现在来整理结构
function SkillRBase()
{
	this._isCoolDown;		//标志	正在cd
	this._cdTime;			//cd时间
	this._cdLeft;			//剩余cd
}








/*********************************
			攻击
***********************************/
/*
攻击技能是指用角色所装备的武器进行的普通近战物理攻击
其攻击输出，攻击速度(cd)，命中率等属性值取决于角色装备的武器和角色的属性值
*/


//pacman 2013-11-16 
//构造
function SklAttack()
{
	//----模板数据-----
	this._TData = null;		//模板数据

	//----相关数据-----
	this._char = null;		//相关角色
	this._weapon = null;	//相关武器

	//----cd-----
	this._isCoolDown = 0;		//标志	正在cd
	this._cdTime = 0;			//cd时间
	this._cdLeft = 0;			//剩余cd

	//----攻击输出----
	this._minDamage = 0;
	this._maxDAgame = 0;

	//-----命中率-----
	this._attackRate = 0;

}

//pacman 2013-12-12 
//初始化
SklAttack.prototype.init = function(char)
{
	if (!char) return;
	
	

}


/*********************************
			火球术
***********************************/
//未完待续
//pacman 2013-11-15  
//构造
function SklFireBall()
{
	this._skillT;		//模板数据
	this._isCoolDown;	//标志 正在cd
	this._cd;			//cd值
	this._cdLeft;		//剩余cd
	this._manaCost;		//耗魔
	this._castMaterialTID;		//施法材料TID
	this._castMaterialCost;		//施法材料数量

}

//pacman 2013-11-15 静态函数 使用技能
//@skillR  技能实体数据
//@user	    施法者
//@target	目标
SklFireBall.use = function(skillR, user, target)
{
	//检查技能状态
	var status = SklFireBall.checkStatus(skillR, user,target);

}

//pacman 2013-11-15 静态函数 检查技能使用状态
SklFireBall.checkStatus = function(skillR, user, target)
{
	//技能cd
	if (skillR._isCoolDown)
	{
		return ;
	}

}


/*********************************
			技能管理器
***********************************/
//pacman 2013-11-22
//构造
function SkillManager()
{

}

//pacman 2013-11-22 单例
SkillManager._instance = null;
SkillManager.instance = function()
{
	if(!SkillManager._instance)
	{
		SkillManager._instance = new SkillManager();
	}
	return SkillManager._instance;
}

//pacman 2013-11-22 为玩家添加技能
//character 获得技能的玩家
//SkillTID  要获得的技能的TID
//aquireSkill我写在SkillManager中而不是Character里，是考虑到也许对skillT的配置与生成需要更多的外部信息和数据
//另外也是因为不太想让Character过多的调用全局管理器
SkillManager.prototype.aquireSkill = function(character, SkillTID)
{
	//如果SkillTID没有对应的skillT对象
	var skillT = SkillManager.instance.getSkillTByTID(SkillTID);
	if (!skillT)
	{
		console.log("SkillManager.aquireSkill: no skillT for TID " + SkillTID);
		return;
	}
	else
	{
		character.aquireSkill(skillT);
	}

}