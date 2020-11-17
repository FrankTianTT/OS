#include<iostream>
#include<fstream>
#include<cstring>
#include<string>
using namespace std;

typedef unsigned char u8;	//1字节
typedef unsigned short u16;	//2字节
typedef unsigned int u32;	//4字节

#pragma pack (1) /*指定按1字节对齐*/

struct F12BPB{
    u16 numBytesPerSector;
    u8 numSectorsPerCluster;
    u16 numReservedSectors;
    u8 numFATs;
    u16 numRootDirectoryEntries;
    u16 numTotalSectors;
    u8 mediaDescriptor;
    u16 numSectorsPerFAT16;
    char anyway[8];
    u32 numSectorsPerFAT32;
};

struct DirEntry {
	char fileName[11];
	u8 fileAttributes;		//文件属性
	char anyway[14];
	u16  fileFirstCluster;	//开始簇号
	u32  fileSize;
};

struct Node{
    string name;
    Node *next;
    Node *child;            //only used when node is a dir.
	string printPath;
	u32 fileSize;
    u16  fileFirstCluster;	//开始簇号
	bool isFile;
	bool isRootDir;
	int dirCount;           //如果是文件夹的话，统计子文件夹个数
	int fileCount;          //如果是文件夹的话，统计子文件个数
	char *content;
    long offset;
};

extern "C" {
	void asm_print(const char *, const int);
}


int NumBytesPerSector;
int NumSectorsPerCluster;
int NumReservedSectors;
int NumFATs;
int NumRootDirectoryEntries;
int NumTotalSectors;
int NumSectorsPerFAT;

void LoadingF12BPB(FILE* imgFile, F12BPB* BPBPointer);
void LoadingRootDir(FILE* imgFilePointer, Node* rootNodePointer, DirEntry* dirEntryPointer);
void dealInput(string input, Node* rootNodePointer);
void lsNormal(Node* nodePointer);
void lsL(Node* nodePointer);
Node* SearchDirNode(string dirName, Node* nodePointer);
Node* SearchFileNode(string fileName, Node* nodePointer);
void cpp_print(const char * s);

int main(){
	FILE* imgFile;
	imgFile = fopen("a.img", "rb");	//打开FAT12的映像文件
    F12BPB imgF12BPB;
    LoadingF12BPB(imgFile, &imgF12BPB);                    //加载BPB

    NumBytesPerSector = imgF12BPB.numBytesPerSector;
    NumSectorsPerCluster = imgF12BPB.numSectorsPerCluster;
    NumReservedSectors = imgF12BPB.numReservedSectors;
    NumFATs = imgF12BPB.numFATs;
    NumRootDirectoryEntries = imgF12BPB.numRootDirectoryEntries;
    NumTotalSectors = imgF12BPB.numTotalSectors;
    if (imgF12BPB.numSectorsPerFAT16 != 0){
        NumSectorsPerFAT = imgF12BPB.numSectorsPerFAT16;
    }
    else{
        NumSectorsPerFAT = imgF12BPB.numSectorsPerFAT32;
    }

    DirEntry dirEntry;
    Node rootNode;
    LoadingRootDir(imgFile, &rootNode, &dirEntry);

    while(true){
        cpp_print("> ");
        string input;
        getline(cin, input);
        if(input == "exit") break;
        dealInput(input, &rootNode);
    }
    return 0;
}

void LoadingF12BPB(FILE* imgFilePointer, F12BPB* BPBPointer){
    fseek(imgFilePointer, 11, SEEK_SET);
    fread(BPBPointer, 1, 25, imgFilePointer);
}

bool checkIsName(char* dirName){
    for(int i=0;i<11;i++){
        char c = dirName[i];
        if((c>='0' && c<='9') || (c>='a' && c<='z') || (c>='A' && c<='Z') || c == ' '){
            continue;
        }
        else{
            return false;
        }
    }
    return true;
}
string dealFileName(char *dirName){
    string re = "";
    for(int i=0;i<8;i++){
        char c = dirName[i];
        if(c==' ') break;
        re += c;
    }
    if(dirName[8] != ' ') re += '.';
    for(int i=8;i<11;i++){
        char c = dirName[i];
        if(c==' ') break;
        re += c;
    }
    return re;
}

void LoadingFile(FILE* imgFilePointer, Node* fileNodePointer, string rootDirName){
    long beforeOffset = ftell(imgFilePointer);          //得到函数掉调用前的文件指针位置，在函数结束后重置
    long dataBaseOffset = NumBytesPerSector * (NumReservedSectors + NumFATs * NumSectorsPerFAT + (NumRootDirectoryEntries * 32 + NumBytesPerSector -1)/NumBytesPerSector);
    long thisDirOffset = dataBaseOffset + (fileNodePointer->fileFirstCluster - 2) * NumBytesPerSector * NumSectorsPerCluster;
    fseek(imgFilePointer, thisDirOffset, SEEK_SET); 
    fileNodePointer->content = new char[fileNodePointer->fileSize];
    fread(fileNodePointer->content, 1, fileNodePointer->fileSize, imgFilePointer);
    fileNodePointer->printPath = rootDirName + fileNodePointer->name;
    fseek(imgFilePointer, beforeOffset, SEEK_SET); 
    return;
}

void LoadingDir(FILE* imgFilePointer, Node* dirNodePointer, DirEntry* dirEntryPointer, string rootDirName){
    long beforeOffset = ftell(imgFilePointer);          //得到函数掉调用前的文件指针位置，在函数结束后重置
    long dataBaseOffset = NumBytesPerSector * (NumReservedSectors + NumFATs * NumSectorsPerFAT + (NumRootDirectoryEntries * 32 + NumBytesPerSector -1)/NumBytesPerSector);
    long thisDirOffset = dataBaseOffset + (dirNodePointer->fileFirstCluster - 2) * NumBytesPerSector * NumSectorsPerCluster;
    fseek(imgFilePointer, thisDirOffset, SEEK_SET); 
    long maxDirInCluster = NumBytesPerSector * NumSectorsPerCluster / 32;
    Node * nowNotePointer = dirNodePointer;            //当前指针指向根目录节点

    dirNodePointer->printPath = rootDirName + dirNodePointer->name + "/";
    int childDirCount = 0;
    int childFileCount = 0;
    for(int i = 0; i < maxDirInCluster; i++){
        fread(dirEntryPointer, 1, 32, imgFilePointer);
        if (!checkIsName(dirEntryPointer->fileName)) continue;  //检查是否是正常的文件名/文件夹名
        if (nowNotePointer == dirNodePointer){                 //如果当前节点是根结点，接下来的文件应该是节点的child
            nowNotePointer->child = new Node;                   //如果不是，接下来的文件应该是节点的next
            nowNotePointer = nowNotePointer->child;
        }
        else{
            nowNotePointer->next = new Node;
            nowNotePointer = nowNotePointer->next;
        }

        nowNotePointer->name = dealFileName(dirEntryPointer->fileName);
        nowNotePointer->fileFirstCluster = dirEntryPointer->fileFirstCluster;
        nowNotePointer->offset = thisDirOffset + i * 32;
        if(dirEntryPointer->fileAttributes == 16){              //通过属性判断文件类型，16是文件夹，32是文件
            nowNotePointer->isFile=false;
            LoadingDir(imgFilePointer, nowNotePointer, dirEntryPointer, dirNodePointer->printPath);
            childDirCount ++;
        }
        else{
            nowNotePointer->isFile=true;
            nowNotePointer->fileSize = dirEntryPointer->fileSize;
            LoadingFile(imgFilePointer, nowNotePointer, dirNodePointer->printPath);
            childFileCount ++;
        }
    }
    dirNodePointer->dirCount = childDirCount;
    dirNodePointer->fileCount = childFileCount;
    dirNodePointer->isRootDir = false;
    fseek(imgFilePointer, beforeOffset, SEEK_SET); 
    return;
}

void LoadingRootDir(FILE* imgFilePointer, Node* rootNodePointer, DirEntry* dirEntryPointer){
    int rootDirOffset = NumBytesPerSector * (NumReservedSectors + NumFATs * NumSectorsPerFAT);
    fseek(imgFilePointer, rootDirOffset, SEEK_SET);     //将指针指向偏移量，也就是根目录起始的位置
    Node * nowNotePointer = rootNodePointer;            //当前指针指向根目录节点

    int rootDirCount = 0;
    int rootFileCount = 0;
    rootNodePointer->printPath = "/";
    for(int i = 0; i < NumRootDirectoryEntries; i++){
        fread(dirEntryPointer, 1, 32, imgFilePointer);
        if (!checkIsName(dirEntryPointer->fileName)) continue;  //检查是否是正常的文件名/文件夹名
        if (nowNotePointer == rootNodePointer){                 //如果当前节点是根结点，接下来的文件应该是节点的child
            nowNotePointer->child = new Node;                   //如果不是，接下来的文件应该是节点的next
            nowNotePointer = nowNotePointer->child;
        }
        else{
            nowNotePointer->next = new Node;
            nowNotePointer = nowNotePointer->next;
        }
        nowNotePointer->name = dealFileName(dirEntryPointer->fileName);
        nowNotePointer->fileFirstCluster = dirEntryPointer->fileFirstCluster;
        nowNotePointer->offset = rootDirOffset + i * 32;
        if(dirEntryPointer->fileAttributes == 16){              //通过属性判断文件类型，16是文件夹，32是文件
            nowNotePointer->isFile=false;
            LoadingDir(imgFilePointer, nowNotePointer, dirEntryPointer, rootNodePointer->printPath);
            rootDirCount ++;
        }
        else{
            nowNotePointer->isFile=true;
            nowNotePointer->fileSize = dirEntryPointer->fileSize;
            LoadingFile(imgFilePointer, nowNotePointer, rootNodePointer->printPath);
            rootFileCount ++;
        }
    }
    rootNodePointer->dirCount = rootDirCount;
    rootNodePointer->fileCount = rootFileCount;
    rootNodePointer->isRootDir = true;
}

void dealInput(string input, Node* rootNodePointer){                       //有限状态机
// 有限状态机的状态有：
//0: 期待读入一条指令
//1: 期待读入一个文件                       //由cat指令导入
//2: 可以读入一个文件夹（也可以不读入)         //由ls指令导入
//3: 已经读完了一个文件夹                    //由ls指令导入，并不再读入文件夹
//4: 已经读完了一个文件                     //由cat指令导入，并不再读入文件
    input = input + "  ";
    int dirNameCharPointer = 0;
    int inputCharPointer = 0;
    char* dirName = new char[input.length()];
    int state = 0;                                  //有限状态机的状态
    bool ifParamL = false;
    while(inputCharPointer < input.length()){
        if(input[inputCharPointer] == ' '){
        }
        else if(input[inputCharPointer] == 'l'){         //期待's'
            if(state != 0){
                cpp_print("指令重复！\n");
                return;
            }
            inputCharPointer ++;
            if(input[inputCharPointer] == 's'){          
                state = 2;
            }
            else{
                cpp_print("无法识别的指令，你是想输入‘ls’吗？\n");
                return;
            }
        }
        else if(input[inputCharPointer] == 'c'){         //期待'a'
            if(state != 0){
                cpp_print("指令重复！\n");
                return;
            }
            inputCharPointer ++;
            if(input[inputCharPointer] == 'a'){          //期待't'
                inputCharPointer ++;
                if(input[inputCharPointer] == 't'){
                    state = 1;
                }
                else{
                    cpp_print("无法识别的指令，你是想输入‘cat’吗？\n");
                    return;
                }
            }
            else{
                cpp_print("无法识别的指令，你是想输入‘cat’吗？\n");
                return;
            }
        }
        else if(input[inputCharPointer] == '-'){
            if(state == 1){
                cpp_print("cat指令不支持参数！\n");
                return;
            }
            else if(state == 0){
                cpp_print("请先输入一条指令！\n");
                return;
            }
            else{
                inputCharPointer ++;
                if(input[inputCharPointer] == 'l'){
                    inputCharPointer ++;
                    while(true){
                        if(input[inputCharPointer] == 'l'){
                            inputCharPointer ++;
                            continue;
                        }
                        else if (input[inputCharPointer] == ' ' || input[inputCharPointer] == '\n'){
                            ifParamL = true;
                            break;
                        }
                        else{
                            cpp_print("无效的参数！\n");
                            return;
                        }
                    }
                }
                else{
                    cpp_print("只支持-l参数！\n");
                    return;
                }
            }
        }
        else{
            if (state == 0){
                cpp_print("不存在的指令\n");
                return;
            }
            if (state == 3){
                cpp_print("不再接受一个文件夹！\n");
                return;
            }
            while(true){
                char c = input[inputCharPointer];
                if((c>='0' && c<='9') || (c>='a' && c<='z') || (c>='A' && c<='Z') || c == '/' || c == '.'){
                    dirName[dirNameCharPointer] = c;
                    dirNameCharPointer ++;
                    inputCharPointer ++;
                }
                else if (c == ' ' || c == '\n'){
                    dirName[dirNameCharPointer] = '\0';
                    break;
                }
                else{
                    cpp_print("无效的路径名！\n");
                    return;
                }
            }
            if (state==2)state = 3;
            if (state==1)state = 4;
        }
        inputCharPointer ++;
    }

    if (state == 0) return;
    else if(state == 1){
        cpp_print("请输入文件名！\n");
        return;
    }
    else if(state == 2){
        if(ifParamL) lsL(rootNodePointer);
        else lsNormal(rootNodePointer);
    }
    else if(state == 3){
        Node* lsNode = SearchDirNode(dirName, rootNodePointer);
        if(lsNode == NULL){
            cpp_print("您输入的目录不存在！\n");
            return;
        }
        if(ifParamL) lsL(lsNode);
        else lsNormal(lsNode);
    }
    else if(state == 4){
        Node* catNode = SearchFileNode(dirName, rootNodePointer);
        if(catNode == NULL){
            cpp_print("您输入的文件不存在！\n");
            return;
        }
        cpp_print(catNode->content);
    }
    return;
}

void lsNormal(Node* nodePointer){
    string strPrint = "";
    strPrint = nodePointer->printPath + ":\n";
    cpp_print(strPrint.c_str());
    Node* nowNodePointer = nodePointer->child;
    if(!nodePointer->isRootDir){
        strPrint = "\033[31m.  ..\033[0m";
        cpp_print(strPrint.c_str());
    }
    else{
        if(nowNodePointer->isFile){
            strPrint = "\033[0m"+nowNodePointer->name;
            cpp_print(strPrint.c_str());
        }
        else{
            strPrint = "\033[31m"+nowNodePointer->name+"\033[0m";
            cpp_print(strPrint.c_str());
        }
        nowNodePointer = nowNodePointer->next;
    }

    while(nowNodePointer!=NULL){
        if(nowNodePointer->isFile){
            strPrint = "\033[0m"+nowNodePointer->name;
            cpp_print(strPrint.c_str());
        }
        else{
            strPrint = "\033[31m"+nowNodePointer->name+"\033[0m";
            cpp_print(strPrint.c_str());
        }
        nowNodePointer = nowNodePointer->next;
    }
    cpp_print("\n");
    nowNodePointer = nodePointer->child;
    while(nowNodePointer!=NULL){
        if(!nowNodePointer->isFile){
            lsNormal(nowNodePointer);
        }
        nowNodePointer = nowNodePointer->next;
    }
}

void lsL(Node* nodePointer){
    string strPrint = "";
    strPrint = nodePointer->printPath + " " + to_string(nodePointer->dirCount) + " " + to_string(nodePointer->fileCount) + ":\n";
    cpp_print(strPrint.c_str());
    Node* nowNodePointer = nodePointer->child;
    if(!nodePointer->isRootDir){
        strPrint = "\033[31m.\n..\033[0m\n";
        cpp_print(strPrint.c_str());
    }
    while(nowNodePointer!=NULL){
        if(nowNodePointer->isFile){
            strPrint = "\033[0m" + nowNodePointer->name + " " + to_string(nowNodePointer->fileSize) + "\n";
            cpp_print(strPrint.c_str());
        }
        else{
            strPrint = "\033[31m" + nowNodePointer->name + "\033[0m " + to_string(nowNodePointer->dirCount) + " " + to_string(nowNodePointer->fileCount) + "\n";
            cpp_print(strPrint.c_str());
        }
        nowNodePointer = nowNodePointer->next;
    }
    cpp_print("\n");
    nowNodePointer = nodePointer->child;
    while(nowNodePointer!=NULL){
        if(!nowNodePointer->isFile){
            lsL(nowNodePointer);
        }
        nowNodePointer = nowNodePointer->next;
    }
}

Node* SearchDirNode(string dirName, Node* nodePointer){
    if(dirName[dirName.length()-1]!='/') dirName += '/';
    if(dirName[0]!='/') dirName = '/' + dirName;
    if(nodePointer->printPath == dirName) return nodePointer;
    Node* nowNodePointer = nodePointer->child;
    Node *re;
    while(nowNodePointer!=NULL){
        if(!nowNodePointer->isFile){
            re = SearchDirNode(dirName, nowNodePointer);
            if(re != NULL) return re;
        }
        nowNodePointer = nowNodePointer->next;
    }
    return NULL;
}

Node* SearchFileNode(string fileName, Node* nodePointer){
    if(fileName[0]!='/') fileName = '/' + fileName;
    Node* nowNodePointer = nodePointer->child;
    Node *re;
    while(nowNodePointer!=NULL){
        if(!nowNodePointer->isFile){
            re = SearchFileNode(fileName, nowNodePointer);
            if(re != NULL) return re;
        }
        else{
            if(nowNodePointer->printPath == fileName){
                return nowNodePointer;
            }
        }
        nowNodePointer = nowNodePointer->next;
    }
    return NULL;
}

void cpp_print(const char * s){
    asm_print(s, strlen(s));
}