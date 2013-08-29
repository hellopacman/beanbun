package  
{
	/*pacman 2013-8-29 工具函数集
	
	
	*/
	
	public class Util 
	{
		//------静态成员、方法---------
		//移除字串中的whiteSpace
		public static const REMOVE_WSPACE_MODE_START_AND_END:int = 0;		//模式： 移除字串首尾两端的whiteSpace
		public static function removeWhiteSpace(targetString:String, removeMode:int):String
		{
			switch (removeMode)
			{
				case REMOVE_WSPACE_MODE_START_AND_END:			//删除开头和末尾的回车符号					
					var pattern:RegExp = /^\s+/;
					targetString = targetString.replace ( pattern,'' );
			
					pattern = /\s+$/;
					targetString = targetString.replace ( pattern,'' );
			
					break;
				default:
					break;
				
			}

			return targetString;
		}
		
		
		
		
		//构造
		public function Util() 
		{
			// constructor code
		}

	}
	
}
