/*********************************
		道具TID释义表
***********************************/
var i = 0;			//自增索引
var ETID = new Object();
ETID.Attack = i ++;		//近战武器攻击



/*********************************
		道具模板数据
***********************************/
//-- 技能模板表
var tid = 0;			//模板id
var T_SKILL_LIST = new Object();

//pacman 2013-11-6
//近战武器攻击	使用当前武器进行一次物理攻击
tid = ETID.Attack;
T_SKILL_LIST[tid] = new Object();
T_SKILL_LIST[tid].tid = tid; 					//技能TID
T_SKILL_LIST[tid].skillName = "Attack"; 		//技能名称
