#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<vector>
using namespace std;
/*
  ������ʱ���������2013��
  �ĸ����ʱ���������2502��
  ����������
  ��matlab��������(��ǰ��miditoolbox��ӵ�·����)

  nmat=readmidi('C:\Users\huang\Desktop\MIDI\�ҵ��ഺ���������Ȼ������ ED Hello Alone.mid');
  nmat(:,4)=midi2hz(pitch(nmat));

  midi��Ƶ·�������Լ�����,�滻��������
  ����nmat�ĵ����к��������,��סctrl��ѡ������ס�ж�����,����Ҫд��C++��һ���궨���λ��
  �½��ı��ĵ�,���ֺ�midi�ļ���һ������
  ��֮ǰ�����ݸ��ƽ�ȥ����
  ����ݴ���,�޸���������,�͵�һ���궨���λ��
  �޸������txt�ı���·��
  ���·��Ҳ�����Լ�����
  ctrl+F5
  ȥ����ı�������

  ��Ϊʱ��������������,��Щ����̫���Ŀ�����Ҫƴ�Ӻü���
  �������Ѿ�ʵ�����������,����̫����Զ�����ֺ�,�ļ������������ļ�������01��02��03�������ļ�����(֧�ֵ�01~99)
  ����֧��4̨��5̨��������
  ������CODE BY HLM������
         2021.1.25

�޸���ʷ 
v1.10
��Ӷ����ļ����й���
2021.1.23
������������������������
v1.11
���4̨��������������
2021.1.24
������������������������
v1.12
�޸�����΢С����
����������Ƭ�ν�����ʾ��2��
2021.1.25
������������������������
v1.20
����Զ����й��ܣ�����Ҫ���ֶ�������������
2021.2.26
������������������������
*/
#define MIDI "C:\\Users\\huang\\Desktop\\MIDI_IN\\.txt"
#define MIDI_OUT_FILE "C:\\Users\\huang\\Desktop\\MIDI_OUT\\.txt"
#define MULTIPLE_WRITER 1 //�����ļ����Ĭ�Ͽ���(1)
#define FOLD_CHEAK 1     //д1���MIDI�ı�;д0���������ϸ��Ϣ,�������MIDI�ı�,һ������
#define MIDI_SPEED 1  //����,Ĭ��1���٣�0.5~2�����Ƽ�,���������Ĺ�
#define MAX_NOTE5 2013   //��̨�����������
#define MAX_NOTE4 2502   //��̨�����������
#define MOTOR_NUMBER 5  //֧�ֵ������2~5,��Ҫ����д�������ְ�
#define MAX 20           //��һʱ�������������ɵ���������(�͵�������޹�),һ�㲻�޸�,��������������Ǿ͸Ĵ��
#define FREQUENCY 62200   
#define _CRT_SECURE_NO_WARNINGS
static int k;//�������
static int NOTE_NUMBER;//��������
struct timeline{   //ʱ��������
	int number;    //�������
	float begin;   //���俪ʼ
	float finish;  //�������
	float during;  //����ʱ��
	int note_number;//���ڸ��������������
	float note_fre[MAX];//�����䶼����ЩƵ�ʵ�����
};
struct note{     //��������
	float frequency;
	float begin;
	float during;  //��������ʱ��
	float finish;
};
struct motor{ //���ͨ��
	float Xmotor;
	float Ymotor;
	float Zmotor;
	float Amotor;
	float Mmotor;
};
timeline *sort(float *a,int n/*ʱ��������,�޳��ظ�ʱ��,����timeline��*/){  
	int i,j,pos;
	float t;    //ʱ��������
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
    int N=0;    //�޳��ظ���ʱ������
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
void sort_frequency(timeline *timing/*����Ѿ�����õ�����������*/){
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
void multiple_writer(timeline *timing,motor *motors,int number,int motor_number){ //��ʵ��������д�ķǳ��ǳ��ǳ���,�Ҿ͵�����vector����ϰ��;��ʵֻҪȥ��.txt������
	int len,i,j=0;
	char *txt_path=MIDI_OUT_FILE;
	vector<char> midiname;
	vector<char> file_path;
	len=strlen(MIDI_OUT_FILE);
	for(i=len-5;i>0;i--){//����.txt�����׺,��ǰѰ��
		if(txt_path[i]!='\\')
			midiname.push_back(txt_path[i]);
		else
			break;
	}
	for(i=0;i<len-midiname.size()-4;i++){//����·��
		if(txt_path[i]!='\\')
			file_path.push_back(txt_path[i]);
		if(txt_path[i]=='\\'){
			file_path.push_back(txt_path[i]);
			file_path.push_back('\\');
		}
	}
	reverse(midiname.begin(), midiname.end());//������midiname
	char *write_multiple_file=new char[file_path.size()+midiname.size()+8];
		for(i=0;i<file_path.size()+midiname.size();i++){ //·��+midi�ļ���,����û�к�׺��.txt
			if(i<file_path.size())
				write_multiple_file[j++]=file_path.at(i);
			if(i>=file_path.size())
				write_multiple_file[j++]=midiname.at(i-file_path.size());
		}
	write_multiple_file[j]='\0';
	int *interval_span=new int[number*2];//���������ֹ
 		for(int i=0;i<number*2;i++){      
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
	char *temp=new char[strlen(write_multiple_file)+8];//��һ��ԭʼ·�����ļ���
		for(int i=1,j=0;i<=number;i++,j+=2){
			char serial_number;
			strcpy(temp,write_multiple_file);
			serial_number=(i/10)+48;//ת�����ַ���
				strcat(temp,"_");
				strncat(temp,&serial_number,1);
			serial_number=i%10+48;
				strncat(temp,&serial_number,1);
				strcat(temp,".txt");
			fstream multiple_file(temp,ios::out);
				multiple_file<<"const MIDI NOTE_INFO[NOTE_NUMBER] PROGMEM = {"<<endl;
				switch(motor_number){
				case 5: for(int i=interval_span[j];i<interval_span[j+1];i++)	//���ĵ��洢��ʼ	
							multiple_file<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(motors[i].Xmotor)<<","<<int(motors[i].Ymotor)<<","<<int(motors[i].Zmotor)<<","<<int(motors[i].Amotor)<<","<<int(motors[i].Mmotor)<<","<<endl;							
						multiple_file<<"2000,100,100,100,100,100,\n};";
							multiple_file.close();
						break;
				case 4: for(int i=interval_span[j];i<interval_span[j+1];i++)	//���ĵ��洢��ʼ	
							multiple_file<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(motors[i].Xmotor)<<","<<int(motors[i].Ymotor)<<","<<int(motors[i].Zmotor)<<","<<int(motors[i].Amotor)<<","<<endl;							
						multiple_file<<"2000,100,100,100,100,100,\n};";
							multiple_file.close();
						break;
				}
		}
	delete []temp;
	delete write_multiple_file;
	delete []interval_span;
}
void distribution(timeline *timing/*������+���������˵�����*/){
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
										MOTOR[i].Mmotor=timing[i].note_fre[timing[i].note_number-1];
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
										MOTOR[i].Ymotor=FREQUENCY/timing[i].note_fre[timing[i].note_number-1];
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
			case 2: cout<<"����ʱ��"<<'\t'<<"X����ٶ�"<<'\t'<<"Y����ٶ�"<<endl;
					MIDI_OUT<<"const MIDI NOTE_INFO[NOTE_NUMBER] PROGMEM = {"<<endl;
						for(int i=0;i<k;i++){
							cout<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(fabs(MOTOR[i].Xmotor))<<","<<int(MOTOR[i].Ymotor)<<","<<endl;
							MIDI_OUT<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(fabs(MOTOR[i].Xmotor))<<","<<int(MOTOR[i].Ymotor)<<","<<endl;
						}
					MIDI_OUT<<"};";
					break;
			case 3: cout<<"����ʱ��"<<'\t'<<"X����ٶ�"<<'\t'<<"Y����ٶ�"<<'\t'<<"Z����ٶ�"<<endl;
					MIDI_OUT<<"const MIDI NOTE_INFO[NOTE_NUMBER] PROGMEM = {"<<endl;
						for(int i=0;i<k;i++){
							cout<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(MOTOR[i].Xmotor)<<","<<int(MOTOR[i].Ymotor)<<","<<int(MOTOR[i].Zmotor)<<","<<endl;
							MIDI_OUT<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(MOTOR[i].Xmotor)<<","<<int(MOTOR[i].Ymotor)<<","<<int(MOTOR[i].Zmotor)<<","<<endl;
						}
					MIDI_OUT<<"};";
					break;
			case 4: cout<<"����ʱ��"<<'\t'<<"X����ٶ�"<<'\t'<<"Y����ٶ�"<<'\t'<<"Z����ٶ�"<<'\t'<<"A����ٶ�"<<endl;
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
			case 5: cout<<"����ʱ��"<<'\t'<<"X����ٶ�"<<'\t'<<"Y����ٶ�"<<'\t'<<"Z����ٶ�"<<'\t'<<"A����ٶ�"<<'\t'<<"M���Ƶ��"<<endl;
					MIDI_OUT<<"const MIDI NOTE_INFO[NOTE_NUMBER] PROGMEM = {"<<endl;
						for(int i=0;i<(k<MAX_NOTE5?k:MAX_NOTE5);i++){
							cout<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(MOTOR[i].Xmotor)<<","<<int(MOTOR[i].Ymotor)<<","<<int(MOTOR[i].Zmotor)<<","<<int(MOTOR[i].Amotor)<<","<<int(MOTOR[i].Mmotor)<<","<<endl;
							MIDI_OUT<<fixed<<setprecision(4)<<timing[i].during*1000/MIDI_SPEED<<","<<int(MOTOR[i].Xmotor)<<","<<int(MOTOR[i].Ymotor)<<","<<int(MOTOR[i].Zmotor)<<","<<int(MOTOR[i].Amotor)<<","<<int(MOTOR[i].Mmotor)<<","<<endl;
						}
						MIDI_OUT<<"2000,100,100,100,100,100,"<<endl;
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
void interval_number(timeline *timing,note *notes/*ʱ�����������������Ͷ�����ЩƵ�ʵ�����*/){
	int i,j,m,n;
	int *s=new int[NOTE_NUMBER];
	int *t=new int[NOTE_NUMBER];
		for(i=0;i<k;i++)          //��ʼ��ȫ��������������Ϊ0
			timing[i].note_number=0;
		for(i=0;i<NOTE_NUMBER;i++){ //�ҵ���������Խ�������� n��������,m��β����
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
		for(i=0;i<NOTE_NUMBER;i++)      //��¼ÿ���������������
			for(j=s[i];j<=t[i];j++){
				timing[j].note_number++;
			}
	int count;
	    for(j=0;j<k;j++){   //��¼ÿ�����䶼����ЩƵ�ʵ�����
			count=0;
			for(i=0;i<NOTE_NUMBER;i++)
				if(s[i]<=j&&j<=t[i]){
					timing[j].note_fre[count]=notes[i].frequency;
					count++;
				}
		} 
		sort_frequency(timing);
#if FOLD_CHEAK  //��������
		distribution(timing);
#endif
		delete []s;
		delete []t;
}
void line_counter(char *midi_file/*ͳ��������Ҳ������������*/){
	fstream infile(midi_file,ios::in);
	int n=0;
	string temp;
	while(getline(infile,temp))
		n++;
	if(n==0){
		cout<<"�ļ���ȡʧ��\n";
		exit(1);
	}
	NOTE_NUMBER=n;
	infile.close();
}
void main(){
	int i=0,j=0;
	float total_time=0;
	line_counter(MIDI);//��ȡ����
	float *time=new float[NOTE_NUMBER*4];//��������,��һ�¾Ͳ�����
	note *note_INFO=new note[NOTE_NUMBER]; //��¼ÿһ����������
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
	timeline *TIME=sort(time,j);  //�ֺ�������������ÿ�������ʱ��,һ����k������
		for(int i=0;i<k;i++)
			total_time+=TIME[i].during;
		total_time/=MIDI_SPEED;
		int min=total_time/60;
		int sec=total_time-60*min;
		cout<<"������ʱ��:"<<total_time<<"��"<<endl;
		cout<<"           "<<min<<":"<<sec<<endl;
		cout<<"���������"<<k;
		switch(MOTOR_NUMBER){
			case 5: cout<<"    �����������������"<<MAX_NOTE5<<endl;break;
			case 4: cout<<"    �����������������"<<MAX_NOTE4<<endl;
			default: ;
		}
		interval_number(TIME,note_INFO);//ͳ�Ƴ�ÿ���������������������Щ����
#if !FOLD_CHEAK  //��������
	cout<<"�����"<<'\t'<<"��ʼ"<<'\t'<<"����"<<'\t'<<"��������"<<'\t'<<"����Ƶ��"<<endl;
		 for(i=0;i<k;i++){
			 cout<<fixed<<setprecision(4)<<TIME[i].number+1<<'\t'<<TIME[i].begin<<'\t'<<TIME[i].finish<<'\t'<<TIME[i].note_number<<'\t';
			 for(int j=0;j<TIME[i].note_number;j++)
				 cout<<TIME[i].note_fre[j]<<" ";
			 cout<<endl;
		 }
#endif
	delete []TIME;
	delete []time;
	delete []note_INFO;

}