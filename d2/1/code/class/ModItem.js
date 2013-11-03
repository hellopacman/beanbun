//include t_item.js

/*********************************
			武器类
***********************************/
//pacman 2013-10-26 
//构造
function WeaponR(weaponT)
{
	//成员变量声明
	this._TData = weaponT;		//模板数据
}

//pacman 2013-10-26 获取道具类型  #Super
WeaponR.prototype.getItemClass = function()
{
	return this._TData.ITEM_CLASS;
}

//pacman 2013-10-26 获取武器的最小伤害输出
WeaponR.prototype.getMinDamage = function()
{
	return this._TData.MIN_DAMAGE;
}

//pacman 2013-10-26 获取武器的最大伤害输出
WeaponR.prototype.getMaxDamage = function()
{
	return this._TData.MAX_DAMAGE;
}

//pacman 2013-10-26 获取武器类型
WeaponR.prototype.getWeaponType = function()
{
	return this._TData.WEAPON_TYPE;
}

