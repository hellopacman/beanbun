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
``	场景中的对白显示框需命名为 _dialog，并且只能有一个
	场景中对白编辑框需需命名为 _dialog_edit，并且只能有一个
	2013-7-16 pacman 取消tc框，改为在dh框中直接编辑台词
	2013-7-25 pacman 为实现在时间轴上制作转场特效，需要对程序进行一些改动
	2013-7-28 又重新还原对白显示框，因为原先直接在显示框中编辑台词的做法，有时会造成台词播放时闪动
	2013-8-27 对白编辑框从dh改名为_dialog_edit
	*/
	public class AcgScene extends MovieClip
	{
		//-----静态常量-----
		static const DIALOGUE_LETTER_SPACE:Number = 0;		//台词文字间距
		static const CHAR_CODE_ENTER:Number = 13;			//台词文本分段符号
		static const LINES_DELIMITER:String = "|";			//台词文本分段符号
		static const DIALOG_START_SYMBOL:String = "『";		//对白开始提示符
		static const DIALOG_END_SYMBOL:String = "』";		//对白结束提示符
		static const TEXT_INTERVAL:Number = 100;					//文本阅读速度(输出下一文字的时间间隔: ms)
		
		//-----成员变量-----
		private var _lastFrame:int;			//上一帧序号
		private var _dialogueArr:Array;			//台词列表
		private var _curDialogueIndex:int;			//当前台词序号
		private var _curDialogueStr:String;			//当前台词分段内容
		private var _curDialogueLength:int;			//当前台词字符数量
		private var _isDialogueOutputing:Boolean;			//是否正在输出台词
		private var _curDialoguePos:int;					//当前台词播放到了第几个字符
		private var _dialogueTimer:Timer;					//控制台词显示速度的timer
		private var _isHold:Boolean = false;			//标记：是否要hold住游戏

		//------构造函数----
		public function AcgScene()
		{

			_lastFrame = 0;
			this.addEventListener(Event.ENTER_FRAME, onEnterFrame);
			_dialogueTimer = new Timer(TEXT_INTERVAL);
			_dialogueTimer.addEventListener(TimerEvent.TIMER, onLineTimer);
			//stage注册鼠标点击事件
			stage.addEventListener(MouseEvent.CLICK, onStageClicked);

		}
		
		//---setter&getter---curDialogueIndex
		private function set curDialogueIndex(value:int):void
		{
			_curDialogueIndex = value;
			_curDialogueStr = _dialogueArr[_curDialogueIndex];
			
			//对原始文本进行一些处理
			//pacman 2013-7-20 删除开头和末尾的回车符号
			var pattern:RegExp = new RegExp ( '^\r+' );
			_curDialogueStr = _curDialogueStr.replace ( pattern,'' );
			pattern = new RegExp ( '\r+$' );
			_curDialogueStr = _curDialogueStr.replace ( pattern,'' );
			
			//pacman 2013-7-17 在当前对白前后加上起始/结束标识符
			_curDialogueStr = DIALOG_START_SYMBOL + _curDialogueStr + DIALOG_END_SYMBOL;
			
			_curDialogueLength = _curDialogueStr.length;
		}
		private function get curDialogueIndex():int
		{
			return _curDialogueIndex;
		}

		//---事件处理：enterFrame---
		private function onEnterFrame(evt:Event):void
		{
			if (_lastFrame != this.currentFrame)		//开始播放新的一帧
			{
				
				if (_dialog_edit != null)		//如果舞台上有放置对白文本框
				{
					//获取舞台上编辑好的台词
					var txt = _dialog_edit.text;
					
						
					//切割对白分段
					_dialogueArr = txt.split(LINES_DELIMITER);
												
					//pacman 2013-7-21 设置文本框样式
					//对于【动态文本框】，在ide中设置字距不管用么？
          			var format:TextFormat = new TextFormat();
            		format.letterSpacing  = DIALOGUE_LETTER_SPACE;
					_dialog.defaultTextFormat = format;
	
					//开始输出台词
					startOutputLine();
					_lastFrame = this.currentFrame;
				}
				/*	pacman 2013-7-28 没有手动台词的帧也默认stop，不再自动nextFrame
				else if(!_isHold)
				{
					_lastFrame = this.currentFrame;		//pacman 2013-7-25 执行nextFrame貌似会跳过剩余帧脚本，所以要把这一行放在nextFrame前面
					nextFrame();
				}
				*/
			}
		}
		
		//----开始输出指定的第几段台词------
		//用帧时间真实跨度控制文本显示速度代码会复杂一下，我看可以用更简单直观的Timer
		//@ lineNo 从哪一段台词开始，默认从第一段开始
		private function startOutputLine(lineNo:int = 0)
		{
			curDialogueIndex = lineNo;
			_curDialoguePos = 0;
			_isDialogueOutputing = true;
			
			//隐藏对话中断符
			hideLineBreak();
						
			//启动定时器
			_dialogueTimer.reset();
			_dialogueTimer.start();			
		}
		
		//---时间处理---
		private function onLineTimer(evt:TimerEvent):void
		{
			//显示当前台词字符
			//pacman 2013-7-20 appendText会忽略对白字串中的换行符号，所以改用 +=
			//_dialog_edit.appendText(_curDialogueStr.charAt(_curDialoguePos));	
			_dialog.text += (_curDialogueStr.charAt(_curDialoguePos));
			
			//接下来是否已经播放到台词末尾
			_curDialoguePos ++;
			if(_curDialoguePos >= _curDialogueLength)	//到达末尾
			{
				finishOutput();
			}
		}
		
		//----结束输出台词----
		private function finishOutput():void
		{
			_dialogueTimer.stop();
			_isDialogueOutputing = false;
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
			if(!_isDialogueOutputing)
			{
				if (_curDialogueIndex < _dialogueArr.length - 1)		//如果还有下一段，准备显示它
				{
					_dialog.text = "";
					_curDialogueIndex ++;
					startOutputLine(_curDialogueIndex);
				}
				else		//没有下一段则判断是否进入下一帧
				{
					//隐藏对话中断符
					hideLineBreak();
					if(!_isHold)
					{
						//进入下一帧
						_dialog.text = "";		//清除对白
						nextFrame();
					}
					//trace("准备进入下一帧");	
				}
			}
			//如果某段台词还没显示完，直接显示完
			else
			{
				_dialog.text = _curDialogueStr;
				finishOutput();
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
			curDialogueIndex = 0;
			_curDialoguePos = 0;
		}
		*/
		

	}//end class
	
}//end package