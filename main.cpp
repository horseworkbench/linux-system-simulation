#include "stdafx.h"
#include "head.h"
char		choice;
vector<string>vc_of_str;
string  s1, s2;
int		inum_cur;		// 当前目录
char		temp[2 * BLKSIZE];	// 缓冲区
User		user;		// 当前的用户
char		bitmap[BLKNUM];	// 位图数组
Inode	inode_array[INODENUM];	// i节点数组
File_table file_array[FILENUM];	// 打开文件表数组
char	image_name[10] = "hd.dat";	// 文件系统名称
FILE		*fp;					// 打开文件指针

// "help", "cd", "dir", "mkdir", "creat", "open","read", "write", "close", "delete", "logout", "clear", "format","quit","rd"
const string Commands[] = { "man", "cd", "ls", "mkdir", "creat", "open","read", "write", "close", "rmfile", "sudo", "clear", "format","df","rmdir" };

// bin/xx 给出进入bin即可
int readby() {	//根据当前目录和第二个参数确定转过去的目录
	int result_cur = 0; string s = s2;
	if (s.find('/') != -1) {
		s = s.substr(0, s.find_last_of('/') + 1);
	}
	else {
		s = "";
	}
	int temp_cur = inum_cur;
	vector<string>v;
	while (s.find('/') != -1) {
		v.push_back(s.substr(0, s.find_first_of('/')));
		s = s.substr(s.find_first_of('/') + 1);
	}
	if (v.size() == 0) {
		return inum_cur;
	}if (v[0].length() == 0) {
		temp_cur = 0;
	}
	else if (v[0] == "..") {
		if (inode_array[inum_cur].inum > 0) {
			temp_cur = inode_array[inum_cur].iparent;
		}
	}
	else {
		int i;
		for (i = 0; i < INODENUM; i++) {
			if ((inode_array[i].inum > 0) &&
				(inode_array[i].iparent == inum_cur) &&
				(inode_array[i].type == 'd') &&
				inode_array[i].file_name == v[0]) {
				break;
			}
		}
		if (i == INODENUM) {
			return -1;
		}
		else {
			temp_cur = i;
		}
	}
	int i;
	for (unsigned int count = 1; count < v.size(); count++) {
		for (i = 0; i < INODENUM; i++) {
			if ((inode_array[i].inum > 0) &&//是否为空
				(inode_array[i].iparent == temp_cur) &&
				(inode_array[i].type == 'd') &&
				inode_array[i].file_name == v[count]) {
				break;
			}
		}
		if (i == INODENUM) {
			return -1;
		}
		else {
			temp_cur = i;
		}
	}
	result_cur = temp_cur;
	return result_cur;
}
									//创建映像hd，并将所有用户和文件清除
void format(void)
{
	FILE  *fp;
	int   i;
	Inode inode;
	printf("Will be to format filesystem...\n");
	printf("WARNING:ALL DATA ON THIS FILESYSTEM WILL BE LOST!\n");
	printf("Proceed with Format(Y/N)?");
	scanf("%c", &choice);
	gets_s(temp);
	if ((choice == 'y') || (choice == 'Y'))
	{
		if ((fp = fopen(image_name, "w+b")) == NULL)
		{
			printf("Can't create file %s\n", image_name);
			exit(-1);
		}
		for (i = 0; i < BLKSIZE; i++)
			fputc('0', fp);
		inode.inum = 0;
		strcpy(inode.file_name, "/");
		inode.type = 'd';
		strcpy(inode.user_name , "all");
		inode.iparent = 0;
		inode.length = 0;
		inode.address[0] = -1;
		inode.address[1] = -1;
		fwrite(&inode, sizeof(Inode), 1, fp);
		inode.inum = -1;
		for (i = 0; i < 31; i++)
			fwrite(&inode, sizeof(Inode), 1, fp);
		for (i = 0; i < BLKNUM*BLKSIZE; i++)
			fputc('\0', fp);
		fclose(fp);
		// 打开文件user.txt
		if ((fp = fopen("user.txt", "w+")) == NULL)
		{
			printf("Can't create file %s\n", "user.txt");
			exit(-1);
		}
		fclose(fp);
		printf("Filesystem created successful.Please first login!\n");
	}
	return;
}
// 功能: 用户登陆，如果是新用户则创建用户

void login(void)
{
	char *p;
	int  flag;
	char user_name[10];
	char password[10];
	char file_name[10] = "user.txt";
	do
	{
		printf("login:");
		gets_s(user_name);
		printf("password:");
		p = password;
		while (*p = _getch())
		{
			if (*p == 0x0d) //当输入回车键时，0x0d为回车键的ASCII码
			{
				*p = '\0'; //将输入的回车键转换成空格
				break;
			}
			printf("*");   //将输入的密码以"*"号显示
			p++;
		}
		flag = 0;
		if ((fp = fopen(file_name, "r+")) == NULL)
		{
			printf("\nCan't open file %s.\n", file_name);
			printf("This filesystem not exist, it will be create!\n");
			format();
			login();
		}
		while (!feof(fp))
		{
			fread(&user, sizeof(char)*20, 1, fp);
			// 已经存在的用户, 且密码正确
			if (user.user_name== user_name &&
				!strcmp(user.password, password))
			{
				fclose(fp);
				printf("\n");
				return;
			}
			// 已经存在的用户, 但密码错误
			else if (user.user_name== user_name)
			{
				printf("\nThis user is exist, but password is incorrect.\n");
				flag = 1;
				fclose(fp);
				break;
			}
		}
		if (flag == 0) break;
	} while (flag);
	// 创建新用户
	if (flag == 0)
	{
		printf("\nDo you want to creat a new user?(y/n):");
		scanf("%c", &choice);
		gets_s(temp);
		if ((choice == 'y') || (choice == 'Y'))
		{
			user.user_name= user_name;
			strcpy(user.password, password);
			fwrite(&user, sizeof(char)*20, 1, fp);
			fclose(fp);
			return;
		}
		if ((choice == 'n') || (choice == 'N'))
			login();
	}
}
// 功能: 将所有i节点读入内存

void init(void)
{
	int   i;
	if ((fp = fopen(image_name, "r+b")) == NULL)
	{
		printf("Can't open file %s.\n", image_name);
		exit(-1);
	}
	// 读入位图
	for (i = 0; i < BLKNUM; i++)
		bitmap[i] = fgetc(fp);
	// 显示位图
	// 读入i节点信息
	for (i = 0; i < INODENUM; i++)
		fread(&inode_array[i], sizeof(Inode), 1, fp);
	// 显示i节点
	// 当前目录为根目录
	inum_cur = 0;
	// 初始化打开文件表
	for (i = 0; i < FILENUM; i++)
		file_array[i].inum = -1;
}

void StrListForCom() {
	vc_of_str.clear();
	vc_of_str.push_back("cd");
	vc_of_str.push_back("mkdir");
	vc_of_str.push_back("ls");
	vc_of_str.push_back("vi");
	vc_of_str.push_back("sudo");
	vc_of_str.push_back("rmfile");
	vc_of_str.push_back("touch");
	vc_of_str.push_back("man");
	vc_of_str.push_back("cat");
	vc_of_str.push_back("clear");
	vc_of_str.push_back("rmdir");
	vc_of_str.push_back("df");
}

void StrListForAdd() {
	vc_of_str.clear();
	int temp_cur;
	temp_cur = readby();
	for (int i = 0; i < INODENUM; i++) {
		if ((inode_array[i].inum > 0) &&
			(inode_array[i].iparent == temp_cur)) {
			if (inode_array[i].type == 'd' && inode_array[i].user_name==user.user_name)
			{
				string temp = inode_array[i].file_name;
				temp += '/';
				vc_of_str.push_back(temp);
			}
			if (inode_array[i].type == '-' && inode_array[i].user_name == user.user_name)
			{
				vc_of_str.push_back(inode_array[i].file_name);
			}
		}

	}
}

// 结果: 0-14为系统命令, 15为命令错误
int analyse()
{
	string s = ""; s1 = ""; s2 = "";
	int tabcount = 0;
	int res = 0;
	while (1) {
		s1 = s.substr(0, s.find_first_of(' '));
		if(s.find(' ')==-1)s2 = "";
		else { 
			s2 = s.substr(s.find_first_of(' ') + 1);
			while (s2.length()>0 && s2[s2.length() - 1] == ' ') {
				s2 = s2.substr(0, s2.length() - 1);
			}
			while (s2.length()>0&&s2[0] == ' ') {
				s2 = s2.substr(1);
			} 
		}
		int ch = _getch();
		if (ch == 8) {				//退格
			if (!s.empty()) {
				printf("%c", 8);
				printf(" ");
				printf("%c", 8);
				s.pop_back();
			}
		}
		else if (ch == 13) {		//回车
			for (res = 0; res < 15; res++) {
				if (s1 == Commands[res])break;
			}
			break;
		}
		else if (ch == 9) {			//tab
			int count = 0; vector<int>v;
			string tstr;
			if (s.find(' ') != -1) {
				tstr = s.substr(s.find_last_of(' ') + 1);
				if (tstr.find('/') != -1) {
					tstr = tstr.substr(tstr.find_last_of('/') + 1);
				}
				StrListForAdd();
			}
			else {
				tstr = s;
				StrListForCom();
			}
			for (unsigned int i = 0; i < vc_of_str.size(); i++) {
				if (vc_of_str[i].length() >= tstr.length() && vc_of_str[i].substr(0, tstr.length()) == tstr) {
					count++; v.push_back(i);
				}
			}
			//cout << "count:" << count<<endl;
			//cout << "tstr:" << tstr<<endl;
			if (count < 1) {
				if (s.find(' ') == -1) {
					s.push_back(' ');
					printf(" ");
				}
				tabcount = -1;
			}
			if (count == 1) {
				for (unsigned int i = tstr.length(); i < vc_of_str[v[0]].length(); i++) {
					s.push_back(vc_of_str[v[0]][i]);
					printf("%c", vc_of_str[v[0]][i]);
				}
				if (s.find(' ') == -1) {
					s.push_back(' ');
					printf(" ");
				}
				tabcount = -1;
			}
			if (count > 1 && tabcount) {
				cout << "\n";
				cout << vc_of_str[v[0]];
				for (unsigned int i = 1; i < v.size(); i++) {
					cout << "    " << vc_of_str[v[i]];
				}
				cout << endl;
				pathset();
				cout << s;
				tabcount = -1;
			}

		}
		else if (ch == ' ') {
			printf("%c", ch);
			s.push_back(ch);
		}
		else {
			printf("%c", ch);
			s.push_back(ch);
		}
		//用于处理按两次tab
		if (ch == 9) {
			tabcount++;
		}
		else {
			tabcount = 0;
		}
	}
	printf("\n");
	return res;
}

// 功能: 将num号i节点保存到hd.dat
void save_inode(int num)
{
	if ((fp = fopen(image_name, "r+b")) == NULL)
	{
		printf("Can't open file %s\n", image_name);
		exit(-1);
	}
	fseek(fp, 512 + num * sizeof(Inode), SEEK_SET);
	fwrite(&inode_array[num], sizeof(Inode), 1, fp);
	fclose(fp);
}

// 功能: 申请一个数据块
int get_blknum(void)
{
	int i;
	for (i = 0; i < BLKNUM; i++)
		if (bitmap[i] == '0') break;
	// 未找到空闲数据块
	if (i == BLKNUM)
	{
		printf("Data area is full.\n");
		exit(-1);
	}
	bitmap[i] = '1';
	if ((fp = fopen(image_name, "r+b")) == NULL)
	{
		printf("Can't open file %s\n", image_name);
		exit(-1);
	}
	fseek(fp, i, SEEK_SET);
	fputc('1', fp);
	fclose(fp);
	return i;
}

// 功能: 将i节点号为num的文件读入temp 
void read_blk(int num)
{
	int  i, len;
	char ch;
	int  add0, add1;
	len = inode_array[num].length;
	add0 = inode_array[num].address[0];
	if (len > 512)
		add1 = inode_array[num].address[1];
	if ((fp = fopen(image_name, "r+b")) == NULL)
	{
		printf("Can't open file %s.\n", image_name);
		exit(-1);
	}
	fseek(fp, 1536 + add0 * BLKSIZE, SEEK_SET);
	ch = fgetc(fp);
	for (i = 0; (i < len) && (ch != '\0') && (i < 512); i++)
	{
		temp[i] = ch;
		ch = fgetc(fp);
	}
	if (i >= 512)
	{
		fseek(fp, 1536 + add1 * BLKSIZE, SEEK_SET);
		ch = fgetc(fp);
		for (; (i < len) && (ch != '\0'); i++)
		{
			temp[i] = ch;
			ch = fgetc(fp);
		}
	}
	temp[i] = '\0';
	fclose(fp);
}

// 功能: 将temp的内容输入hd的数据区
void write_blk(int num)
{
	int  i, len;
	int  add0, add1;
	add0 = inode_array[num].address[0];
	len = inode_array[num].length;
	if ((fp = fopen(image_name, "r+b")) == NULL)
	{
		printf("Can't open file %s.\n", image_name);
		exit(-1);
	}
	fseek(fp, 1536 + add0 * BLKSIZE, SEEK_SET);
	for (i = 0; (i<len) && (temp[i] != '\0') && (i < 512); i++)
		fputc(temp[i], fp);
	if (i == 512)
	{
		add1 = inode_array[num].address[1];
		fseek(fp, 1536 + add1 * BLKSIZE, SEEK_SET);
		for (; (i < len) && (temp[i] != '\0'); i++)
			fputc(temp[i], fp);
	}
	fputc('\0', fp);
	fclose(fp);
}

// 功能: 释放文件块号为num的文件占用的空间
void release_blk(int num)
{
	FILE *fp;
	if ((fp = fopen(image_name, "r+b")) == NULL)
	{
		printf("Can't open file %s\n", image_name);
		exit(-1);
	}
	bitmap[num] = '0';
	fseek(fp, num, SEEK_SET);
	fputc('0', fp);
	fclose(fp);
}

// 功能: 显示帮助命令
void help(void)
{
	printf("command: \n\
help   ---  show help menu \n\
clear  ---  clear the screen \n\
cd     ---  change directory \n\
mkdir  ---  make directory   \n\
creat  ---  create a new file \n\
open   ---  open a exist file \n\
read   ---  read a file \n\
write  ---  write something to a file \n\
close  ---  close a file \n\
delete ---  delete a exist file \n\
format ---  format a exist filesystem \n\
logout ---  exit user \n\
rd     ---  delete a directory \n\
quit   ---  exit this system\n");
}

//设置文件路径
void pathset()
{
	string s;
	if (inode_array[inum_cur].inum == 0)s = "";
	else {
		int temp = inum_cur;
		while (inode_array[temp].inum != 0) {
			s = inode_array[temp].file_name + s;
			s = '/' + s;
			temp = inode_array[temp].iparent;
		}
	}
	cout << user.user_name << "@" << "4423" << ":~" << s << "# ";
}


// 功能: 切换目录(cd .. 或者 cd dir1)
void cd()
{
	int temp_cur;
	if (s2.length() == 0) {
		temp_cur = 0;
	}
	else {
		if (s2[s2.length() - 1] != '/')s2 += '/';
		temp_cur = readby();
	}
	if (temp_cur != -1) {
		inum_cur = temp_cur;
	}
	else {
		cout << "No Such Directory" << endl;
	}
}


// 功能: 显示当前目录下的子目录和文件(dir)
void dir(void)
{
	int temp_cur;
	int i = 0;
	if (s2.length() == 0){
		temp_cur = inum_cur;
		}
	else {
		temp_cur = readby();
		if (temp_cur == -1) {
			cout << "No Such Directory" << endl;
			return;
		}
	}
	if (temp_cur != -1 && inode_array[temp_cur].type == 'd')
		for (i = 0; i < INODENUM; i++)
		{
			if ((inode_array[i].inum > 0) &&
				(inode_array[i].iparent == temp_cur))
			{
				if (inode_array[i].type == 'd' && inode_array[i].user_name == user.user_name)
				{
					printf("%-20s<DIR>\n", inode_array[i].file_name);
				}
				if (inode_array[i].type == '-' && inode_array[i].user_name == user.user_name)
				{
					printf("%-20s%12d bytes\n", inode_array[i].file_name, inode_array[i].length);
				}
			}
		}
}

// 功能: 删除目录树(rd dir1)
void rd()
{
	if (s2[s2.length() - 1] != '/')s2 += '/';
	int temp_cur=readby();

	return;
}

// 功能: 在当前目录下创建子目录(mkdir dir1)
void mkdir(void)
{
	int i;
	// 遍历i节点数组, 查找未用的i节点
	for (i = 0; i < INODENUM; i++) {
		if (inode_array[i].iparent == inum_cur && inode_array[i].type == 'd'
			&&inode_array[i].file_name == s2 && inode_array[i].inum > 0 
			&& inode_array[i].user_name == user.user_name) {
			break;
		}
	}
	if (i != INODENUM) {
		printf("There is directory having same name.\n");
		return;
	}
	for (i = 0; i < INODENUM; i++)
		if (inode_array[i].inum < 0) break;
	if (i == INODENUM)
	{
		printf("Inode is full.\n");
		exit(-1);
	}
	inode_array[i].inum = i;
	strcpy(inode_array[i].file_name, s2.data());
	inode_array[i].type = 'd';
	strcpy(inode_array[i].user_name, user.user_name.data());
	inode_array[i].iparent = inum_cur;
	inode_array[i].length = 0;
	save_inode(i);
}

// 功能: 在当前目录下创建文件(creat file1)
void creat(void)
{
	int i;
	cout <<"s2"<< s2 << endl;
	for (i = 0; i < INODENUM; i++)
	{
		if ((inode_array[i].inum > 0) &&
			(inode_array[i].type == '-') &&
			s2==inode_array[i].file_name)
		{
			printf("This file is exsit.\n");
			return;
		}
	}
	for (i = 0; i < INODENUM; i++)
		if (inode_array[i].inum < 0) break;
	if (i == INODENUM)
	{
		printf("Inode is full.\n");
		exit(-1);
	}
	inode_array[i].inum = i;
	strcpy( inode_array[i].file_name,s2.data());
	inode_array[i].type = '-';
	strcpy(inode_array[i].user_name, user.user_name.data());
	inode_array[i].iparent = inum_cur;
	inode_array[i].length = 0;
	save_inode(i);
}

// 功能: 打开当前目录下的文件(open file1)
void open()
{
	int i, inum, mode, filenum;
	for (i = 0; i < INODENUM; i++)
		if ((inode_array[i].inum > 0) &&
			(inode_array[i].type == '-') &&
			s2 == inode_array[i].file_name)
			break;
	if (i == INODENUM)
	{
		printf("The file you want to open doesn't exsited.\n");
		return;
	}
	inum = i;
	if (inode_array[i].user_name == user.user_name)
	{
		printf("This file is not your !\n");
		return;
	}
	printf("Please input open mode:(1: read, 2: write, 3: read and write):");
	scanf("%d", &mode);
	gets_s(temp);
	if ((mode < 1) || (mode > 3))
	{
		printf("Open mode is wrong.\n");
		return;
	}
	for (i = 0; i < FILENUM; i++)
		if (file_array[i].inum < 0) break;
	if (i == FILENUM)
	{
		printf("The file table is full, please close some file.\n");
		return;
	}
	filenum = i;
	file_array[filenum].inum = inum;
	strcpy(file_array[filenum].file_name, inode_array[inum].file_name);
	file_array[filenum].mode = mode;
	file_array[filenum].offset = 0;
	printf("Open file %s by ", file_array[filenum].file_name);
	if (mode == 1) printf("read only.\n");
	else if (mode == 2) printf("write only.\n");
	else printf("read and write.\n");
}

// 功能: 从文件中读出字符(read file1)
void read()
{
	int i, start, num, inum;
	for (i = 0; i < FILENUM; i++)
		if ((file_array[i].inum > 0) &&
			s2 == file_array[i].file_name)
			break;
	if (i == FILENUM)
	{
		cout << "Open " << s2 << " first.\n";
		return;
	}
	else if (file_array[i].mode == 2)
	{
		cout<<"Can't read "<<s2<<".\n";
		return;
	}
	inum = file_array[i].inum;
	cout<<"The length of "<<s2<<":"<< inode_array[inum].length<<".\n";
	if (inode_array[inum].length > 0)
	{
		printf("The start position:");
		scanf("%d", &start);
		gets_s(temp);
		if ((start<0) || (start >= inode_array[inum].length))
		{
			printf("Start position is wrong.\n");
			return;
		}
		printf("The bytes you want to read:");
		scanf("%d", &num);
		gets_s(temp);
		if (num <= 0)
		{
			printf("The num you want to read is wrong.\n");
			return;
		}
		read_blk(inum);
		for (i = 0; (i < num) && (temp[i] != '\0'); i++)
			printf("%c", temp[start + i]);
		printf("\n");
	}
}

// 功能: 向文件中写入字符(write file1)
void write()
{
	int i, inum, length;
	for (i = 0; i < FILENUM; i++)
		if ((file_array[i].inum>0) &&
			s2==file_array[i].file_name) break;
	if (i == FILENUM)
	{
		cout << "Open " << s2 << " first.\n";
		return;
	}
	else if (file_array[i].mode == 1)
	{
		cout << "Can't write " << s2 << ".\n";
		return;
	}
	inum = file_array[i].inum;
	printf("The length of %s:%d\n", inode_array[inum].file_name, inode_array[inum].length);
	if (inode_array[inum].length == 0)
	{
		printf("The length you want to write(0-1024):");
		scanf("%d", &length);
		gets_s(temp);
		if ((length < 0) && (length >1024))
		{
			printf("Input wrong.\n");
			return;
		}
		inode_array[inum].length = length;
		inode_array[inum].address[0] = get_blknum();
		if (length > 512)
			inode_array[inum].address[1] = get_blknum();
		save_inode(inum);
		printf("Input the data(Enter to end):\n");
		gets_s(temp);
		write_blk(inum);
	}
	else
		printf("This file can't be written.\n");
}

// 功能: 关闭已经打开的文件(close file1)
void close(void)
{
	int i;
	for (i = 0; i < FILENUM; i++)
		if ((file_array[i].inum > 0) &&
			s2==file_array[i].file_name) break;
	if (i == FILENUM)
	{
		printf("This file doesn't be opened.\n");
		return;
	}
	else
	{
		file_array[i].inum = -1;
		cout << "Close " << s2 << " successful!\n";
	}
}

//删除目录树
void delet(int innum)
{

	inode_array[innum].inum = -1;
	if (inode_array[innum].length >= 0)
	{
		release_blk(inode_array[innum].address[0]);
		if (inode_array[innum].length >= 512)
			release_blk(inode_array[innum].address[1]);
	}
	save_inode(innum);
}

// 功能: 删除文件(delete file1)
void del(void)
{
	int i;
	for (i = 0; i < INODENUM; i++)
		if ((inode_array[i].inum > 0) &&
			(inode_array[i].type == '-') &&
			s2 == inode_array[i].file_name) break;
	if (i == INODENUM)
	{
		printf("This file doesn't exist.\n");
		return;
	}
	if (inode_array[i].user_name == user.user_name)
	{
		printf("This file is not your !\n");
		return;
	}
	/*inode_array[i].inum = -1;
	if(inode_array[i].length > 0)
	{
	release_blk(inode_array[i].address[0]);
	if(inode_array[i].length > 512)
	release_blk(inode_array[i].address[1]);
	}
	save_inode(i);*/
	delet(i);
}
// 功能: 退出当前用户(logout)

void logout()
{
	login();
}


// 功能: 退出文件系统(quit)
void quit()
{
	char choice;
	printf("Do you want to exist(y/n):");
	scanf("%c", &choice);
	gets_s(temp);
	if ((choice == 'y') || (choice == 'Y'))
		exit(0);
}

// 功能: 显示错误
void errcmd()
{
	printf("Command Error!!!\n");
}

//清空内存中存在的用户名
void free_user()
{
	int i;
	for (i = 0; i<10; i++)
		user.user_name[i] = '\0';
}
// 功能: 循环执行用户输入的命令, 直到logout
// "help", "cd", "dir", "mkdir", "creat", "open","read", "write", "close", "delete", "logout", "clear", "format","quit","rd"

void command(void)
{
	system("cls");
	do
	{
		pathset();
		switch (analyse())
		{
		case 0:
			help();
			break;
		case 1:
			cd();
			break;
		case 2:
			dir();
			break;
		case 3:
			mkdir();
			break;
		case 4:
			creat();
			break;
		case 5:
			open();
			break;
		case 6:
			read();
			break;
		case 7:
			write();
			break;
		case 8:
			close();
			break;
		case 9:
			del();
			break;
		case 10:
			logout();
			break;
		case 11:
			system("cls");
			break;
		case 12:
			format();
			init();
			free_user();
			login();
			break;
		case 13:
			quit();
			break;
		case 14:
			rd();
			break;
		case 15:
			errcmd();
			break;
		default:
			break;
		}
	} while (1);
}

// 主函数
int main(void)
{
	login();
	init();
	command();
	return 0;
}

