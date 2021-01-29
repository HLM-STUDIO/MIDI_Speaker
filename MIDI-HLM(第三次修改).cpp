#include<iostream>
#include<iomanip>
#include<fstream>
#include<vector>
using namespace std;
/*
  五个电机时间区间最多2013个
  四个电机时间区间最多2502个
  操作方法：
  在matlab里面输入(提前将miditoolbox添加到路径中)

  nmat=readmidi('C:\Users\huang\Desktop\MIDI\我的青春恋爱物语果然有问题 ED Hello Alone.mid');
  nmat(:,4)=midi2hz(pitch(nmat));

  midi音频路径按你自己的来,替换掉就行了
  复制nmat的第四列和最后俩列,按住shift加选哈；记住有多少行,将来要写入C++第一个宏定义的位置
  新建文本文档,名字和midi文件名一样就行
  把之前的内容复制进去保存
  打开这份代码,修改音符数量,就第一个宏定义的位置
  修改输入的txt文本的路径
  输出路径也可以自己定义
  ctrl+F5
  去输出文本看看吧

  因为时间区间数量受限,有些音乐太长的可能需要拼接好几份
  本程序已经实现了这个功能,区间太多会自动帮你分好,文件名是你的输出文件名加上01、02、03这样的文件序列(支持到01~99)
  但仅支持4台和5台电机的情况
  ———CODE BY HLM———
         2021.1.25
*/
#define NOTE_NUMBER 4055 //音符数量
#define MIDI "C:\\Users\\huang\\Desktop\\MIDI_IN\\我的青春恋爱物语果然有问题。完 OP 芽ぐみの雨(Full ver.).txt"
#define MIDI_OUT_FILE "C:\\Users\\huang\\Desktop\\MIDI_OUT\\OP 芽ぐみの雨(Full ver.).txt"
#define MULTIPLE_WRITER 1 //序列文件输出默认关闭(0)
#define FOLD_CHEAK 1     //写1输出MIDI文本;写0输出区间详细信息,但不输出MIDI文本,一般检查用
#define MIDI_SPEED 1   //倍速,默认1倍速；0.5~2倍速推荐,音调不会变的哈
#define MAX_NOTE5 2013   //五台最大区间数量
#define MAX_NOTE4 2502   //四台最大区间数量
#define MOTOR_NUMBER 5   //支持电机数量2~5,不要尝试写其他数字啊
#define MAX 10           //单一时间区间最多可容纳的音符个数(和电机数量无关),一般不修改,如果遇到黑乐谱那就改大点
#define FREQUENCY 62200   
#define _CRT_SECURE_NO_WARNINGS
static int k;//区间个数
struct timeline{   //时间线区间
	int number;    //区间序号
	float begin;   //区间开始
	float finish;  //区间结束
	float during;  //区间时长
	int note_number;//落在该区间的音符数量
	float note_fre[MAX];//该区间都有哪些频率的音符
};
struct note{     //音符区间
	float frequency;
	float begin;
	float during;  //音符持续时间
	float finish;
};
struct motor{ //电机通道
	float Xmotor;
	float Ymotor;
	float Zmotor;
	float Amotor;
	float Mmotor;
};
timeline * sort(float *a,int n/*时间轴排序,剔除重复时间,存入timeline中*/){  
	int i,j,pos;
	float t;    //时间轴排序
        for(i=0;i<n-1;i++){
	        pos=i;
			for(j=i+1;j<n;j++)
				if(a[j]<a[pos])
					pos=j;
			if(pos!=i){
				t=a[pos];
				a[pos]=a[i];
				a[i]=t;
			}
        }
	float POS;
    int N=0;    //剔除重复的时间区间
		for(i=0;i<n-N;i++){
			POS=a[i];
			for (j=i+1;j<n-N;j++)
				if (a[j]==POS){
					for (k=j;k<n-N;k++)
						a[k]=a[k+1];
					N++;
					j--;
				}
		}
	k-=2;
	timeline *time=new timeline[k];
		for(i=0;i<k;i++){
			time[i].number=i;
			time[i].begin=a[i];
			time[i].finish=a[i+1];
			time[i].during=time[i].finish-time[i].begin;
		}
	return time;
	
}
void sort_frequency(timeline *timing/*针对已经分配好的音符来排序*/){
	int pos; 
	float t;
		for(int p=0;p<k;p++)
			for(int i=0;i<timing[p].note_number-1;i++){
				pos=i;
				for(int j=i+1; j<timing[p].note_number; j++)
					if(timing[p].note_fre[j]<timing[p].note_fre[pos])
						pos=j;
				if(pos!=i){
					t=timing[p].note_fre[pos];
					timing[p].note_fre[pos]=timing[p].note_fre[i];
					timing[p].note_fre[i]=t;
				}
			}
}
void multiple_writer(timeline *timing,motor *motors,int number,int motor_number){ //其实函数可以写的非常非常非常简单,我就当作是vector的练习了;其实只要去掉.txt就行了
	int len,i,j=0;
	char *txt_path=MIDI_OUT_FILE;
	vector<char> midiname;
	vector<char> file_path;
	len=strlen(MIDI_OUT_FILE);
	for(i=len-5;i>0;i--){//跳过.txt这个后缀,往前寻找
		if(txt_path[i]!='\\')
			midiname.push_back(txt_path[i]);
		else
			break;
	}
	for(i=0;i<len-midiname.size()-4;i++){//存入路径
		if(txt_path[i]!='\\')
			file_path.push_back(txt_path[i]);
		if(txt_path[i]=='\\'){
			file_path.push_back(txt_path[i]);
			file_path.push_back('\\');
		}
	}
	reverse(midiname.begin(), midiname.end());//逆序存放midiname
	char *write_multiple_file=new char[file_path.size()+midiname.size()+8];
		for(i=0;i<file_path.size()+midiname.size();i++){ //路径+midi文件名,但还没有后缀和.txt
			if(i<file_path.size())
				write_multiple_file[j++]=file_path.at(i);
			if(i>=file_path.size())
				write_multiple_file[j++]=midiname.at(i-file_path.size());
		}
	write_multiple_file[j]='\0';
	int *interval_span=new int[number*2];//存放区间起止
 		for(int i=0;i<number*2;i++){      //与上一个区间重叠10个区间,这样后期剪辑比较方便
			switch(motor_number){
				case 5: interval_span[i]=(i/2+1)*MAX_NOTE5;
							i++;
							if(i<number*2-1)
								interval_span[i]=((i+1)/2+1)*MAX_NOTE5-1;
							else
								interval_span[i]=k;
						break;
				case 4: interval_span[i]=(i/2+1)*MAX_NOTE4;
							i++;
							if(i<number*2-1)
								interval_span[i]=((i+1)/2+1)*MAX_NOTE4-1;
							else
								interval_span[i]=k;
						break;
			}
		}
	char *temp=new char[strlen(write_multiple_file)+8];//存一下原始路径加文件名
		for(int i=1,j=0;i<=number;i++,j+=2){
			char serial_number;
			strcpy(temp,write_multiple_file);
			serial_number=(i/10)+48;//转换成字符型
				strcat(temp,"_");
				strncat(temp,&serial_number,1);
			serial_number=i%10+48;
				strncat(temp,&serial_number,1);
				strcat(temp,".txt");
			fstream multiple_file(temp,ios::out);
				multiple_file<<"const MIDI NOTE_INFO[NOTE_NUMBER] PROGMEM = {"<<endl;
				switch(motor_number){
				case 5: for(int i=interval_span[j];i<interval_span[j+1];i++)	//分文档存储开始	
							multiple_file<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(motors[i].Xmotor)<<","<<int(motors[i].Ymotor)<<","<<int(motors[i].Zmotor)<<","<<int(motors[i].Amotor)<<","<<int(motors[i].Mmotor)<<","<<endl;							
						multiple_file<<"};";
							multiple_file.close();
						break;
				case 4: for(int i=interval_span[j];i<interval_span[j+1];i++)	//分文档存储开始	
							multiple_file<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(motors[i].Xmotor)<<","<<int(motors[i].Ymotor)<<","<<int(motors[i].Zmotor)<<","<<int(motors[i].Amotor)<<","<<endl;							
						multiple_file<<"};";
							multiple_file.close();
						break;
				}
		}
	delete []temp;
	delete write_multiple_file;
	delete []interval_span;
}
void distribution(timeline *timing/*分配电机+输出处理好了的数据*/){
	motor *MOTOR=new motor[k];
		for(int i=0;i<k;i++){
			if(timing[i].note_number==0){
				MOTOR[i].Xmotor=0;
				MOTOR[i].Ymotor=0;
				MOTOR[i].Zmotor=0;
				MOTOR[i].Amotor=0;
				MOTOR[i].Mmotor=0;
			}
			if(timing[i].note_number==MOTOR_NUMBER)
				switch(MOTOR_NUMBER){
					case 5: MOTOR[i].Mmotor=timing[i].note_fre[4];
					case 4: MOTOR[i].Amotor=FREQUENCY/timing[i].note_fre[3];
					case 3: MOTOR[i].Zmotor=FREQUENCY/timing[i].note_fre[2];
					case 2: MOTOR[i].Ymotor=FREQUENCY/timing[i].note_fre[1];
							MOTOR[i].Xmotor=FREQUENCY/timing[i].note_fre[0];
				}
			if(timing[i].note_number>MOTOR_NUMBER){
				switch(MOTOR_NUMBER){
					case 5: MOTOR[i].Mmotor=timing[i].note_fre[timing[i].note_number-5];
					case 4: MOTOR[i].Amotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-4];
					case 3: MOTOR[i].Zmotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-3];
					case 2: MOTOR[i].Ymotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-2];
							MOTOR[i].Xmotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-1];
				}
			}
			if(timing[i].note_number!=0&&timing[i].note_number<MOTOR_NUMBER){
				switch(MOTOR_NUMBER){
					case 2: MOTOR[i].Xmotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-1];
						    MOTOR[i].Ymotor=0;
							MOTOR[i].Zmotor=0;
							MOTOR[i].Amotor=0;
							MOTOR[i].Mmotor=0;
							break;
					case 3: switch(timing[i].note_number){
								case 2: MOTOR[i].Xmotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-2];
										MOTOR[i].Ymotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-1];
										MOTOR[i].Zmotor=0;
										MOTOR[i].Amotor=0;
										MOTOR[i].Mmotor=0;
										break;
								case 1: MOTOR[i].Xmotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-1];
										MOTOR[i].Ymotor=0;
										MOTOR[i].Zmotor=0;
										MOTOR[i].Amotor=0;
										MOTOR[i].Mmotor=0;
										break;
							}
							break;
					case 4: switch(timing[i].note_number){
								case 3: MOTOR[i].Xmotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-3];
										MOTOR[i].Ymotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-2];
										MOTOR[i].Zmotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-1];
										MOTOR[i].Amotor=0;
										MOTOR[i].Mmotor=0;
										break;
								case 2: MOTOR[i].Xmotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-2];
										MOTOR[i].Ymotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-1];
										MOTOR[i].Zmotor=0;
										MOTOR[i].Amotor=0;
										MOTOR[i].Mmotor=0;
										break;
							    case 1: MOTOR[i].Xmotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-1];
										MOTOR[i].Ymotor=0;
										MOTOR[i].Zmotor=0;
										MOTOR[i].Amotor=0;
										MOTOR[i].Mmotor=0;
										break;
							}
							break;
					case 5: switch(timing[i].note_number){
								case 4: MOTOR[i].Xmotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-4];
										MOTOR[i].Ymotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-3];
										MOTOR[i].Zmotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-2];
										MOTOR[i].Amotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-1];
										MOTOR[i].Mmotor=0;
										break;
								case 3: MOTOR[i].Xmotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-3];
										MOTOR[i].Ymotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-2];
										MOTOR[i].Zmotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-1];
										MOTOR[i].Amotor=0;
										MOTOR[i].Mmotor=0;
										break;
								case 2: MOTOR[i].Xmotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-2];
										MOTOR[i].Ymotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-1];
										MOTOR[i].Zmotor=0;
										MOTOR[i].Amotor=0;
										MOTOR[i].Mmotor=0;
										break;
								case 1: MOTOR[i].Xmotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-1];
										MOTOR[i].Ymotor=0;
										MOTOR[i].Zmotor=0;
										MOTOR[i].Amotor=0;
										MOTOR[i].Mmotor=0;
										break;
							}
							break;
				}
			}
		}
	fstream MIDI_OUT(MIDI_OUT_FILE,ios::out);
		switch(MOTOR_NUMBER){
			case 2: cout<<"持续时间"<<'\t'<<"X电机速度"<<'\t'<<"Y电机速度"<<endl;
					MIDI_OUT<<"const MIDI NOTE_INFO[NOTE_NUMBER] PROGMEM = {"<<endl;
						for(int i=0;i<k;i++){
							cout<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(fabs(MOTOR[i].Xmotor))<<","<<int(MOTOR[i].Ymotor)<<","<<endl;
							MIDI_OUT<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(fabs(MOTOR[i].Xmotor))<<","<<int(MOTOR[i].Ymotor)<<","<<endl;
						}
					MIDI_OUT<<"};";
					break;
			case 3: cout<<"持续时间"<<'\t'<<"X电机速度"<<'\t'<<"Y电机速度"<<'\t'<<"Z电机速度"<<endl;
					MIDI_OUT<<"const MIDI NOTE_INFO[NOTE_NUMBER] PROGMEM = {"<<endl;
						for(int i=0;i<k;i++){
							cout<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(MOTOR[i].Xmotor)<<","<<int(MOTOR[i].Ymotor)<<","<<int(MOTOR[i].Zmotor)<<","<<endl;
							MIDI_OUT<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(MOTOR[i].Xmotor)<<","<<int(MOTOR[i].Ymotor)<<","<<int(MOTOR[i].Zmotor)<<","<<endl;
						}
					MIDI_OUT<<"};";
					break;
			case 4: cout<<"持续时间"<<'\t'<<"X电机速度"<<'\t'<<"Y电机速度"<<'\t'<<"Z电机速度"<<'\t'<<"A电机速度"<<endl;
					MIDI_OUT<<"const MIDI NOTE_INFO[NOTE_NUMBER] PROGMEM = {"<<endl;
						for(int i=0;i<(k<MAX_NOTE4?k:MAX_NOTE4);i++){
							cout<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(MOTOR[i].Xmotor)<<","<<int(MOTOR[i].Ymotor)<<","<<int(MOTOR[i].Zmotor)<<","<<int(MOTOR[i].Amotor)<<","<<endl;
							MIDI_OUT<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(MOTOR[i].Xmotor)<<","<<int(MOTOR[i].Ymotor)<<","<<int(MOTOR[i].Zmotor)<<","<<int(MOTOR[i].Amotor)<<","<<endl;
						}
					MIDI_OUT<<"};";
#if MULTIPLE_WRITER
						if(MAX_NOTE4<k)
							 multiple_writer(timing,MOTOR,k/MAX_NOTE4,MOTOR_NUMBER);
#endif
					break;
			case 5: cout<<"持续时间"<<'\t'<<"X电机速度"<<'\t'<<"Y电机速度"<<'\t'<<"Z电机速度"<<'\t'<<"A电机速度"<<'\t'<<"M电机频率"<<endl;
					MIDI_OUT<<"const MIDI NOTE_INFO[NOTE_NUMBER] PROGMEM = {"<<endl;
						for(int i=0;i<(k<MAX_NOTE5?k:MAX_NOTE5);i++){
							cout<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(MOTOR[i].Xmotor)<<","<<int(MOTOR[i].Ymotor)<<","<<int(MOTOR[i].Zmotor)<<","<<int(MOTOR[i].Amotor)<<","<<int(MOTOR[i].Mmotor)<<","<<endl;
							MIDI_OUT<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(MOTOR[i].Xmotor)<<","<<int(MOTOR[i].Ymotor)<<","<<int(MOTOR[i].Zmotor)<<","<<int(MOTOR[i].Amotor)<<","<<int(MOTOR[i].Mmotor)<<","<<endl;
						}
					MIDI_OUT<<"};";
#if MULTIPLE_WRITER
						if(MAX_NOTE5<k)
							 multiple_writer(timing,MOTOR,k/MAX_NOTE5,MOTOR_NUMBER);
#endif
					break;
		}
	MIDI_OUT.close();
	delete []MOTOR;
}
void interval_number(timeline *timing,note *notes/*时间线区间音符个数和都有哪些频率的音符*/){
	int i,j,m,n,s[NOTE_NUMBER],t[NOTE_NUMBER];
		for(i=0;i<k;i++)          //初始化全部区间音符数量为0
			timing[i].note_number=0;
		for(i=0;i<NOTE_NUMBER;i++){ //找到该音符跨越的区间间隔 n是首区间,m是尾区间
			for(n=0;n<k;n++){
				if(notes[i].begin==timing[n].begin){
					s[i]=n;
					break;
				}
			}
			for(m=0;m<k;m++)
				if(notes[i].finish==timing[m].finish){
					t[i]=m;
					break;
				}
		}
		for(i=0;i<NOTE_NUMBER;i++)      //记录每个区间的音符数量
			for(j=s[i];j<=t[i];j++){
				timing[j].note_number++;
			}
	int count;
	    for(j=0;j<k;j++){   //记录每个区间都有那些频率的音符
			count=0;
			for(i=0;i<NOTE_NUMBER;i++)
				if(s[i]<=j&&j<=t[i]){
					timing[j].note_fre[count]=notes[i].frequency;
					count++;
				}
		} 
		sort_frequency(timing);
#if FOLD_CHEAK  //条件编译
		distribution(timing);
#endif
}
void main(){
	float time[NOTE_NUMBER*4];//工具数组,用一下就不用了
	note note_INFO[NOTE_NUMBER]; //记录每一个音符数据
	int i=0,j=0;
	fstream midi(MIDI,ios::in);
		for(;i<NOTE_NUMBER;i++){
			midi>>note_INFO[i].frequency>>note_INFO[i].begin>>note_INFO[i].during;
			note_INFO[i].finish=note_INFO[i].begin+note_INFO[i].during;
		}
	midi.close();
	i=0;
		while(i<NOTE_NUMBER){
			time[j]=note_INFO[i].begin;
			j++;
			time[j]=note_INFO[i].finish;
			j++;i++;
		}
	timeline *TIME=sort(time,j);  //分好了区间数量和每个区间的时间,一共有k个区间
		cout<<"区间个数："<<k;
		switch(MOTOR_NUMBER){
			case 5: cout<<"    可容纳最大区间数："<<MAX_NOTE5<<endl;break;
			case 4: cout<<"    可容纳最大区间数："<<MAX_NOTE4<<endl;
			default: ;
		}
		interval_number(TIME,note_INFO);//统计出每个区间的音符个数和有哪些音符
#if !FOLD_CHEAK  //条件编译
	cout<<"区间号"<<'\t'<<"开始"<<'\t'<<"结束"<<'\t'<<"音符数量"<<'\t'<<"音符频率"<<endl;
		 for(i=0;i<k;i++){
			 cout<<fixed<<setprecision(4)<<TIME[i].number+1<<'\t'<<TIME[i].begin<<'\t'<<TIME[i].finish<<'\t'<<TIME[i].note_number<<'\t';
			 for(int j=0;j<TIME[i].note_number;j++)
				 cout<<TIME[i].note_fre[j]<<" ";
			 cout<<endl;
		 }
#endif
	delete []TIME;
}