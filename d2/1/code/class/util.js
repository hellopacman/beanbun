
/*********************************
		工具函数
***********************************/
var _DEBUG = true;
var Util = new Object();

//pacman 2013-10-26 对数值进行指定位数取整
//@ digit 取整的位数 
//例：精确至0.01，digit取0.01即可
Util.roundDownTo = function(number, numDigits)
{
	return Math.floor(number / numDigits) * numDigits;
}

//pacman 2013-10-27 对数值进行指定位数四舍五入
//@ digit 取整的位数 
//例：精确至0.01，digit取0.01即可
Util.roundTo = function(number, numDigits)
{
	return Math.round(number / numDigits) * numDigits;
}

//pacman 2013-12-12 断言
Util.assert = function(condition, msg)
{
	if(_DEBUG == true && condition == false)
	{
		console.log(msg);
	}
}