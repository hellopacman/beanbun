package
{
	import flash.display.MovieClip;
	import flash.events.*;
	import flash.events.Event;
	import flash.events.MouseEvent;
	import flash.utils.Timer;
	import flash.events.TimerEvent;
	import flash.sensors.Accelerometer;
	import flash.text.TextFormat;
	
	/*----------场景剪辑的基类---------
``	场景中的对白文本框需命名为 dh，并且只能有一个
	场景中对白输入框需需命名为 tc，并且只能有一个
	2013-7-16 pacman 取消tc框，改为在dh框中直接编辑台词
	*/
	public class AcgScene extends MovieClip
	{
		//-----静态常量-----
		static const DIALOG_LETTER_SPACE:Number = 0;		//台词文字间距
		static const CHAR_CODE_ENTER:Number = 13;			//台词文本分段符号
		static const LINES_DELIMITER:String = "|";			//台词文本分段符号
		static const DIALOG_START_SYMBOL:String = "『";		//对白开始标识符
		static const DIALOG_END_SYMBOL:String = "』";		//对白结束标识符
		static const TEXT_INTERVAL:Number = 100;					//文本阅读速度(输出下一文字的时间间隔: ms)
		
		//-----成员变量-----
		private var _lastFrame:int;			//上一帧序号
		private var _lineArr:Array;			//台词列表
		private var _curLine:int;			//当前台词序号
		private var _curLineStr:String;			//当前台词分段内容
		private var _curLineLength:int;			//当前台词字符数量
		private var _isLineOutputing:Boolean;			//是否正在输出台词
		private var _curLinePos:int;					//当前台词播放到了第几个字符
		private var _lineTimer:Timer;					//控制台词显示速度的timer
		private var _isHold:Boolean = false;			//标记：是否要hold住游戏

		//------构造函数----
		public function AcgScene()
		{
			
			
			_lastFrame = 0;
			this.addEventListener(Event.ENTER_FRAME, onEnterFrame);
			_lineTimer = new Timer(TEXT_INTERVAL);
			_lineTimer.addEventListener(TimerEvent.TIMER, onLineTimer);
			//stage注册鼠标点击事件
			stage.addEventListener(MouseEvent.CLICK, onStageClicked);

		}
		
		//---setter&getter---curLine
		private function set curLine(value:int):void
		{
			_curLine = value;
			_curLineStr = _lineArr[_curLine];
			
			//对原始文本进行一些处理
			//pacman 2013-7-20 删除开头和末尾的回车符号
    		//trace(_curLineStr.toString());
			var pattern:RegExp = new RegExp ( '^\r+' );
			//trace(_curLineStr.search(pattern));
			_curLineStr = _curLineStr.replace ( pattern,'' );
			pattern = new RegExp ( '\r+$' );
			//trace(_curLineStr.search(pattern));
			_curLineStr = _curLineStr.replace ( pattern,'' );
			
			//pacman 2013-7-17 在当前对白前后加上起始/结束标识符
			_curLineStr = DIALOG_START_SYMBOL + _curLineStr + DIALOG_END_SYMBOL;
			
						
			_curLineLength = _curLineStr.length;
		}
		private function get curLine():int
		{
			return _curLine;
		}
		
		
		//---事件处理：enterFrame---
		private function onEnterFrame(evt:Event):void
		{
			if (_lastFrame != this.currentFrame)		//开始播放新的一帧
			{
				if (dh != null)		//如果舞台上有放置对白文本框
				{
					//获取舞台上编辑好的台词
					var txt = dh.text;
					
					
					//重置对白框
					dh.text = "";		//清除文字
						
					//切割对白分段
					_lineArr = txt.split(LINES_DELIMITER);
												
					//pacman 2013-7-21 设置文本框样式
					//对于【动态文本框】，在ide中设置字距不管用么？
          			var format:TextFormat = new TextFormat();
            		format.letterSpacing  = DIALOG_LETTER_SPACE;
					dh.defaultTextFormat = format;

					//开始输出台词
					startOutputLine();
				}
				_lastFrame = this.currentFrame;
			}
		}
		
		//----开始输出指定的第几段台词------
		//用帧时间真实跨度控制文本显示速度代码会复杂一下，我看可以用更简单直观的Timer
		//@ lineNo 从哪一段台词开始，默认从第一段开始
		private function startOutputLine(lineNo:int = 0)
		{
			curLine = lineNo;
			_curLinePos = 0;
			_isLineOutputing = true;
			
			//隐藏对话中断符
			hideLineBreak();
						
			//启动定时器
			_lineTimer.reset();
			_lineTimer.start();			
		}
		
		//---时间处理---
		private function onLineTimer(evt:TimerEvent):void
		{
			//显示当前台词字符
			//pacman 2013-7-20 appendText会忽略对白字串中的换行符号，所以改用 +=
			//dh.appendText(_curLineStr.charAt(_curLinePos));	
			dh.text += (_curLineStr.charAt(_curLinePos));
			
			//接下来是否已经播放到台词末尾
			_curLinePos ++;
			if(_curLinePos >= _curLineLength)	//到达末尾
			{
				stopOutput();
			}
		}
		
		//----停止输出台词----
		private function stopOutput():void
		{
			_lineTimer.stop();
			_isLineOutputing = false;
			//显示停止符号
			showLineBreak();
						
			//这里日后可以细化一下，hold状态时最后一句台词结束时不显示停止符号
			//因为停止符号的意图是告知用户点击可以继续
		}

		//---事件处理：stage鼠标点击事件---
		//这个函数目前主要是处理点击屏幕对台词播放的控制
		//不适合做其他的控制，比如选择答案选项
		private function onStageClicked(evt:Event):void
		{
			//如果某段台词已经显示完
			if(!_isLineOutputing)
			{
				if (_curLine < _lineArr.length - 1)		//如果还有下一段，准备显示它
				{
					dh.text = "";
					_curLine ++;
					startOutputLine(_curLine);
				}
				else		//没有下一段则进入下一帧
				{
					//隐藏对话中断符
					hideLineBreak();
					if(!_isHold)
					{
						//进入下一帧
						nextFrame();
					}
					//trace("准备进入下一帧");	
				}
			}
			//如果某段台词还没显示完，直接显示完
			else
			{
				dh.text = _curLineStr;
				stopOutput();
			}
		}

		//显示对话停止符号
		private function showLineBreak():void
		{
			next.visible = true;
			next.play();
		}
		
		//隐藏对话停止符号
		private function hideLineBreak():void
		{
			next.visible = false;
			next.stop();
		}
		
		//hold住游戏
		public function hold():void
		{
			_isHold = true;			
		}
		
		//取消hold
		public function unhold():void
		{
			_isHold = false;			
		}
		
		/*
		//重置台词控制数据
		private function resetLineControl()
		{
			curLine = 0;
			_curLinePos = 0;
		}
		*/
		

	}//end class
	
}//end package