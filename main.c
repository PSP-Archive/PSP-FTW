/* By HardHat Copyright 2011
 * This code is released under the Revised BSD License.  See license.txt for
 * details.
 */
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#include <Winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#ifdef _PSP
#include <netinet/tcp.h>
#include <psptypes.h>
#include <pspkernel.h>
#include <psputility.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#endif
#include <string.h>
//#include <libiberty.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>

#ifdef _WIN32
SOCKET sock;
SOCKADDR_IN addr;
#else
int sock;
struct sockaddr_in addr;
#endif
int tcpPort=7110;

#ifdef _PSP
void startNetworking();
void stopNetworking();
int osk(char *buffer,int size);
#endif
void sendMessage(const char *message);

/* The screen surface */
SDL_Surface *screen = NULL;
TTF_Font *font=NULL;
SDL_Surface *sprite[256];
int spriteCount=16; //47;
const char *spriteName[]={
	"data/sprites/Albert.png",
	"data/sprites/Captain.png",
	"data/sprites/ChrisRock.png",
	"data/sprites/Darryl.png",
	"data/sprites/Doggy.png",
	"data/sprites/Faelon.png",
	"data/sprites/Farmer.png",
	"data/sprites/astronomer.png",
	"data/sprites/babies_mother.png",
	"data/sprites/bartender.png",
	"data/sprites/big_momma.png",
	"data/sprites/chef.png",
	"data/sprites/drunkard0.png",
	"data/sprites/drunkard1.png",
	"data/sprites/fat_old_guy.png",
	"data/sprites/geisha.png",
	"data/sprites/Guard.png",
	"data/sprites/Gunnar.png",
	"data/sprites/Kamwerere.png",
	"data/sprites/King.png",
	"data/sprites/Kitty.png",
	"data/sprites/Mate.png",
	"data/sprites/Mel.png",
	"data/sprites/Mom.png",
	"data/sprites/Rios.png",
	"data/sprites/SallyJones.png",
	"data/sprites/Seaside_informant.png",
	"data/sprites/Tiggy.png",
	"data/sprites/Tipper.png",
	"data/sprites/Trent.png",
	"data/sprites/guy_world.png",
	"data/sprites/lady.png",
	"data/sprites/mokkan.png",
	"data/sprites/prisoner.png",
	"data/sprites/ring_bearer.png",
	"data/sprites/sensei.png",
	"data/sprites/shyzu_female.png",
	"data/sprites/shyzu_male.png",
	"data/sprites/soldier.png",
	"data/sprites/toddler.png",
	"data/sprites/villager1.png",
	"data/sprites/villager2.png",
	"data/sprites/villager3.png",
	"data/sprites/villager4.png",
	"data/sprites/villager5.png",
	"data/sprites/villager6.png",
	"data/sprites/villager7.png",
};
SDL_Surface *hudImage;
SDL_Surface *statImage[4];
int frame=0;

enum GridType { GRID_NONE, GRID_BARRIER, GRID_WAYPOINT};

#define MAXPLAYER 16
struct Player {
	int active;
	int pid;
	char name[256];
	SDL_Surface *sprite;
	int x;
	int y;
	int serverX,serverY;
	int dir;
	int facing;
	int mana;
	int hit;
	int block;
	int waypoint;
	int blockTimer;
	int vote; // -1 for not voted, 0 for nay, 1 for aye
} player[MAXPLAYER];

#define MAXFIREBALL 16
struct Fireball {
	int active;
	int cid;
	int x;
	int y;
	int dir;
} fireball[MAXFIREBALL];

struct Item {
	int active;
	int cid;
	int x;
	int y;
} mana;

char wallMsg[256];
char incoming[1492];
int dirMask;
int grid[32*32];
const int gridWidth=32;
const int gridHeight=32;
int beginTimer;
int billTimer;
int voteTimer;
int voteType=0;
int voteNumber=3;
int votePid;
int winType;
int winNumber;
int winPid;
int winTimer;
int latestGid;
int latestGidTimer;
enum GameMode {MODE_LOGIN,MODE_LOBBY,MODE_JOINED,MODE_GAME,MODE_WINNER } gameMode;

char *typeName[]={
	"mana held",
	"fireball hits",
	"fireball blocks",
	"waypoints visited"
};

char *modeName[]={
	"login",
	"lobby",
	"joined",
	"game",
	"winner"
};

struct Notice { char *text; int timer; } noticeList[256];
int noticeListCount=0;
char *name="user2d";

void gameDisconnect()
{
	printf("Disconnecting...\n");
	sendMessage("~");	// log out.
	SDL_Delay(1000);
	printf("Disconnecting socket %d\n",sock);
	if(sock!=-1) close(sock);
	SDL_Delay(1000);
	sock=-1;
	printf("Disconnected\n");
}

int gameConnect(char *server)
{
#ifdef _WIN32
	WSADATA wsda;
	WSAStartup(MAKEWORD(1,1), &wsda);
	sock=socket(AF_INET, SOCK_STREAM, 0);

	memset( &addr,0,sizeof(addr) );
	addr.sin_family=AF_INET;
	addr.sin_port=htons(tcpPort);
	addr.sin_addr.s_addr=inet_addr(server);
	
#else
	memset(&addr,0,sizeof(addr));

	sock=socket(PF_INET, SOCK_STREAM, 0 ) ;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(tcpPort);
	addr.sin_addr.s_addr=inet_addr(server);
#endif
#ifdef _WIN32
	unsigned long argp=1;
	ioctlsocket(sock, FIONBIO, &argp);
#else
	fcntl(sock, F_SETFL, O_NONBLOCK);
#endif
	if( connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1 ) {
#ifdef _WIN32
		int err=WSAGetLastError();
		if(err=WSAEINPROGRESS) {
			printf("Connection in progress\n");
		} else {
		    printf("Could not connect to socket. %d\n",WSAGetLastError() );		    
		    return 0;
		}
#else
		if(errno==EINPROGRESS) {
			printf("Connection in progress\n");
		} else {
	    	printf("Could not connect to socket. %d\n",errno );
	    	return 0;
		}
#endif
	}
	printf("Started network: %d\n",(int)sock);
#ifdef _PSP
	int flag=1;
	int rc=setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,(char *)&flag,sizeof(flag));
	if(rc<0) printf("setsockopt returns %d\n",errno);
#endif
	//atexit(gameDisconnect);
	return sock;
}

void sendMessage(const char *message)
{
	char buffer[1494];
	sprintf(buffer,"%s\n",message);
	int rc=send(sock,buffer,strlen(buffer),0);
	if(rc==-1) {
#ifndef _WIN32
		if(errno==EAGAIN || errno==EWOULDBLOCK) {
			printf("Write: would block.   So really we need to retry message \"%s\" again.\n",message);
		} else if(errno==EBADF) {
			printf("Write: Bad file descriptor on socket\n");
			exit(0);
		} else if(errno==ECONNRESET) {
			printf("Write: Connection reset\n");
			exit(0);
		} else {
			printf("Write: socket error %d\n",errno);
		}
#else
		int err=WSAGetLastError();
		if(err==WSAENOTCONN || err==WSAENETRESET) {
			printf("Write: not connected or network reset\n");
			exit(0);	
		}
		if(err!=WSAEWOULDBLOCK) {
			printf("Write: socket error %d\n",err);
		}
#endif
	}
}

void getSize( const char *message, int *w, int *h)
{
	*w=0;
	*h=0;
    TTF_SizeText( font, message, w, h);
}

void drawText(int x,int y,const char *message)
{
	if(0) {
		SDL_Rect rect={x+1,y+1,0,0};
		SDL_Color color={0,0,0,255};
		SDL_Surface *surf=TTF_RenderText_Blended(font,message,color);
		SDL_BlitSurface(surf,NULL,screen,&rect);
		SDL_FreeSurface(surf);
	}
	SDL_Rect rect={x,y,0,0};
	SDL_Color color={50,50,0,255};
	SDL_Surface *surf=TTF_RenderText_Blended(font,message,color);
	SDL_BlitSurface(surf,NULL,screen,&rect);
	SDL_FreeSurface(surf);
}

struct Player *newPlayer(char *name,int pid)
{
	int i;
	for(i=0;i<MAXPLAYER;i++) {
		if(player[i].active==0) break;
	}
	if(i==MAXPLAYER) {
		printf("Out of player slots.\n");
		return 0;	// failed!  Out of players.
	}
	player[i].active=1;
	player[i].pid=pid;
	strcpy(player[i].name,name);
	printf("Added player %s (%d)\n",name,pid);
	player[i].sprite=sprite[rand()%spriteCount];
	player[i].x=0;
	player[i].y=0;
	player[i].dir=-1;
	player[i].facing=2;
	player[i].mana=0;
	player[i].hit=0;
	player[i].block=0;
	player[i].waypoint=0;
	player[i].blockTimer=0;
	player[i].vote=-1;
	return player+i;
}

struct Player *getPlayer(int pid)
{
	int i;
	for(i=0;i<MAXPLAYER;i++) {
		if(player[i].active==0) continue;
		if(player[i].pid==pid) {
			return player+i;
		}
	}
	printf("Player %d not found!\n",pid);
	return NULL;
}

struct Fireball *newFireball(int cid)
{
	int i;
	for(i=0;i<MAXFIREBALL;i++) {
		if(fireball[i].active==0) break;
	}
	if(i==MAXFIREBALL) {
		printf("Out of fireball slots!\n");
		return 0;	// out of fireballs!
	}
	fireball[i].active=1;
	fireball[i].cid=cid;
	fireball[i].x=0;
	fireball[i].y=0;
	fireball[i].dir=-1;
	return fireball+i;
}

struct Fireball *getFireball(int cid)
{
	int i;
	for(i=0;i<MAXFIREBALL;i++) {
		if(fireball[i].active==0) continue;
		if(fireball[i].cid==cid) {
			return fireball+i;
		}
	}
	printf("Fireball %d not found!\n",cid);
	return NULL;
}

void notice(const char *note,int duration)
{
	printf("NOTE: %s\n",note);
	if(noticeListCount>255) return;
	noticeList[noticeListCount].timer=duration;
	noticeList[noticeListCount].text=strdup(note);
	noticeListCount++;
}

void hWelcome(const char *msg) { // welcome servername
	notice(msg,10000);
	char buff[256];		// now sign in.
	sprintf(buff,"I%s",name);
	sendMessage(buff);
	sprintf(buff,"Welcome %s",name);
	notice(buff,10000);
}

void hJoin(const char *msg) { // join playername,pid
	char name[256];
	int pid=0;
	sscanf(msg,"%d,%32s",&pid,name);
	newPlayer(name,pid);
}

void hBegin(const char *msg) { // begin seconds
	beginTimer=atoi(msg)*1000;
	if(beginTimer==0) {
		gameMode=MODE_GAME;
	}
}

void hQuit(const char *msg) { // quit pid
	int pid=atoi(msg);
	printf("player %d quit\n",pid);
	struct Player *p=getPlayer(pid);
	if(p) p->active=0;
	if(p && p->pid==player[0].pid) {
		// actually left the game.
		int i;
		for(i=0;i<MAXPLAYER;i++) {
			player[i].active=0;
		}
		for(i=0;i<MAXFIREBALL;i++) {
			fireball[i].active=0;
		}
		mana.active=0;
		for(i=0;i<gridWidth*gridHeight;i++) {
			grid[i]=GRID_NONE;
		}
		gameMode=MODE_LOBBY;
	}
}

void hWinner(const char *msg) { // winner pid
	int pid=atoi(msg);
	char buff[256];
	
	struct Player *p=getPlayer(pid);
	strcpy(buff,"Winner ");
	if(p) strcat(buff,p->name);
	notice(buff,5000);
	winPid=pid;
	winTimer=30000;
	int i;
	for(i=0;i<MAXPLAYER;i++) player[i].dir=-1;	// stop animation
	gameMode=MODE_WINNER;
}

void hBill(const char *msg) { // bill pid -- who's turn it is to propose a new rule
	billTimer=30000;
	int i;
	for(i=0;i<MAXPLAYER;i++) {
		if(player[i].active==0) continue;
		player[i].vote=-1;
	}
}

void hVote(const char *msg) { // vote pid,type,number
	int pid=0,type=0,number=0;
	int count=sscanf(msg,"%d,%d,%d",&pid,&type,&number);
	if(count<3) {
		printf("Invalid vote command: '%s'\n",msg);
		return;
	}
	struct Player *p=getPlayer(pid);
	if(!p) {
		printf("Can't find vote player %d\n",pid);
	}
	billTimer=0;	// overridden.
	voteTimer=30000;
	voteType=type;
	voteNumber=number;
	votePid=pid;
}

void hResult(const char *msg) { // result pid,type,number,yea,nay
	int pid=0,type=0,number=0,yea=0,nay=0;
	int count=sscanf(msg,"%d,%d,%d,%d,%d",&pid,&type,&number,&yea,&nay);
	if(count<5) {
		printf("*** Invalid result message: '%s'",msg);
	}
	if(yea>nay) {
		winType=type;
		winNumber=number;
		char buff[256];
		sprintf(buff,"Rule in effect %d of %s (vote %d to %d)",number,typeName[type],yea,nay);
		notice(buff,3000);
	} else {
		char buff[256];
		sprintf(buff,"New rule vote failed %d to %d",yea,nay);
		notice(buff,3000);
	}
}

void hGames(const char *msg) { // games gid,seconds,mapname,playername,playername,playername,playername
	int gid=0,sec=0;
	char *line=strdup(msg);
	char *s=line;
	char *next=NULL;
	char mapname[256];
	char name[256][16];
	int i;
	//int count=sscanf(msg,"%d,%d,%s,%s,%s,%s,%s",&gid,&sec,mapname,name[0],name[1],name[2],name[3]);
	for(i=0;i<18;i++) {
		if(strchr(s,',')) {
			next=strchr(s,',');
			next[0]=0;
			next++;
		} else {
			next=0;
		}
		switch(i) {
		case 0: gid=atoi(s); break;
		case 1: sec=atoi(s); break;
		case 2: strcpy(mapname,s); break;
		default: strcpy(name[i-3],s); break;
		}
		s=next;
		if(!s) break;
	}
	int maxName=i-2;
	latestGid=gid;
	latestGidTimer=sec*1000;
	char buff[256];
	sprintf(buff,"Game on map '%s' with %d players begins in %d",mapname,maxName,sec);
	notice(buff,10000);
	free(line);
}

void hTop(const char *msg) { //top rank,playername,wins or top pid,playername,gid
	int pid=0;
	char playername[256]="";
	int gid=0;
	sscanf(msg,"%d,%d,%s",&gid,&pid,playername);
	char buff[256];
	if(gid>0)
		sprintf(buff,"Player '%s' playing in game %d",playername,gid);
	else
		sprintf(buff,"Player '%s' in lobby",playername);
	notice(buff,10000);
}

void hPlayer(const char *msg) { //player recognized
	int recognized=atoi(msg);
	if(!recognized) {
		printf("login failed!\n");
		notice("login failed.",5000);
		gameMode=MODE_LOGIN;
		return;
	}
	notice("player recognized",1000);
	gameMode=MODE_LOBBY;
}

void hWall(const char *msg) { //wall playername,message
	notice(msg,5000);
	printf("Wall: %s\n",msg);
}

void hGame(const char *msg) { //game joined,gid
	int joined=0;
	int gid=0;
	sscanf(msg,"%d,%d",&joined,&gid);
	if(joined) {
		gameMode=MODE_JOINED;
		beginTimer=1000;
	}
}

int dirToFacing[]={
	1,1,2,3,3,3,0,1
};

void hState(const char *msg) { //state pid,x,y,dir,mana,hits,blocks,visits
	int pid=0,x=0,y=0,dir=-1,mana=0,hits=0,blocks=0,visits=0;
	sscanf(msg,"%d,%d,%d,%d,%d,%d,%d,%d",&pid,&x,&y,&dir,&mana,&hits,&blocks,&visits);
	struct Player *p=getPlayer(pid);
	if(!p) {
		printf("Cannot find player %d for state update\n",pid);
		printf("Got message: '%s'\nscanned: %d,%d,%d,%d,%d,%d,%d,%d\n",msg,pid,x,y,dir,mana,hits,blocks,visits);
		return;
	}
	//printf("Player %d state\n",pid);
	p->serverX=x;
	p->serverY=y;
	p->dir=dir;
	if(dir>=0) p->facing=dirToFacing[dir];
	p->mana=mana;
	p->hit=hits;
	p->block=blocks;
	p->waypoint=visits;
}

void hFireball(const char *msg) { //fireball cid,x,y,direction
	int cid=0,x=0,y=0,dir=-1;
	sscanf(msg,"%d,%d,%d,%d",&cid,&x,&y,&dir);
	struct Fireball *f=getFireball(cid);
	if(!f) f=newFireball(cid);
	if(!f) {
		printf("Out of fireball slots!\n");
		return;
	}
	f->x=x;
	f->y=y;
	f->dir=dir;
	if(f->dir==-1) f->active=0;	// deactivated.
}

void hCaptured(const char *msg) { //captured cid,pid
	int cid=0,pid=0;
	sscanf(msg,"%d,%d",&cid,&pid);
	struct Player *p=getPlayer(pid);
	if(!p) {
		printf("*** cannot find mana capturing player %d\n",pid);
	}
	char buff[256];
	sprintf(buff,"Player %s captured some mana!",p->name);
	notice(buff,2000);
	mana.active=0;
}

void hMap(const char *msg) { //map 32x32grid
	int i;
	for(i=0;i<32*32;i++) {
		switch(msg[i]) {
		case 0: return;	// early end of the map.
		case '.': grid[i]=GRID_NONE; break;
		case 'W': grid[i]=GRID_WAYPOINT; break;
		default: grid[i]=GRID_BARRIER; break;
		}
	}
}

void hMana(const char *msg) { //mana cid,x,y
	int cid=0,x=0,y=0;
	sscanf(msg,"%d,%d,%d",&cid,&x,&y);
	mana.cid=cid;
	mana.x=x;
	mana.y=y;
	mana.active=1;
}

void hHit(const char *msg) { //hit cid,pid
	int cid=0,pid=0;
	sscanf(msg,"%d,%d",&cid,&pid);
	struct Player *p=getPlayer(pid);
	char buff[256];
	if(p) {
		sprintf(buff,"Player %s was hit with a fireball!",p->name);
	} else {
		sprintf(buff,"Unknown player %d was hit with a fireball",pid);
	}
	notice(buff,2000);
	struct Fireball *f=getFireball(cid);
	if(!f) return;
	f->active=0;	// deactivated.
}

void hBlocked(const char *msg) { //blocked cid,pid
	int cid=0,pid=0;
	sscanf(msg,"%d,%d",&cid,&pid);
	struct Player *p=getPlayer(pid);
	char buff[256];
	if(p) {
		sprintf(buff,"Player %s blocked a fireball!",p->name);
	} else {
		sprintf(buff,"Unknown player %d blocked a fireball",pid);
	}
	notice(buff,2000);
	struct Fireball *f=getFireball(cid);
	if(!f) return;
	f->active=0;	// deactivated.
}

void hBlock(const char *msg) { //blocked pid
	int cid=0,pid=0;
	sscanf(msg,"%d",&pid);
	struct Player *p=getPlayer(pid);
	char buff[256];
	if(p) {
		sprintf(buff,"Player %s blocking",p->name);
		p->blockTimer=2000;
	} else {
		sprintf(buff,"Unknown player %d blocking",pid);
	}
	notice(buff,2000);
	struct Fireball *f=getFireball(cid);
	if(!f) return;
	f->active=0;	// deactivated.
}

void hWaypoint(const char *msg) { //waypoint cid,pid
	int cid=0,pid=0;
	sscanf(msg,"%d,%d",&cid,&pid);
	struct Player *p=getPlayer(pid);
	char buff[256];
	if(p) {
		sprintf(buff,"Player %s touched a waypoint!",p->name);
		p->waypoint++;
	} else {
		sprintf(buff,"Unknown player %d touched a waypoint",pid);
	}
	notice(buff,2000);
}

struct Handler {
	char *command;
	void (*handler)(const char *);
} handlerTable[]={
	{"welcome",hWelcome}, // welcome servername
	{"join",hJoin}, // join playername,pid
	{"begin",hBegin}, // begin seconds
	{"quit",hQuit}, // quit pid
	{"winner",hWinner}, // winner pid
	{"bill",hBill}, // bill pid -- who's turn it is to propose a new rule
	{"vote",hVote}, // vote pid,type,number
	{"result",hResult}, // result pid,type,number,yea,nay
	{"games",hGames}, // games gid,seconds,mapname,playername,playername,playername,playername
	{"top",hTop}, //top rank,playername,wins
	{"player",hPlayer}, //player recognized
	{"wall",hWall}, //wall playename,message
	{"game",hGame}, //game joined,gid
	{"state",hState}, //state pid,x,y,mana,hits,blocks,visits
	{"fireball",hFireball}, //fireball cid,x,y,direction
	{"captured",hCaptured}, //captured cid,pid
	{"map",hMap}, //map 32x32grid
	{"mana",hMana}, //mana cid,x,y
	{"hit",hHit}, //hit cid,pid
	{"blocked",hBlocked}, //blocked cid,pid
	{"block",hBlock}, // block pid
	{"waypoint",hWaypoint}, //waypoint cid,pid	
};
const int handlerTableSize=sizeof(handlerTable)/sizeof(struct Handler);

void dispatchMessage(const char *message)
{
	char command[33];
	command[0]=0;
	sscanf(message,"%32s",command);
	//printf("Dispatching command '%s'\n",command);
	const char *body=message+strlen(command);
	if(body[0]==' ') body++;
	int i;
	for(i=0;i<handlerTableSize;i++) {
		if(strncmp(handlerTable[i].command,command,strlen(handlerTable[i].command))==0) {
			handlerTable[i].handler(body);
			return;
		}
	}
	printf("*** unhandled message '%s'\n",message);
}

int netTimes[11];

void dispatchNetwork()
{
#ifdef _PSP
	sceKernelDelayThread(1000);
#endif
	do {
		int len=strlen(incoming);
		int read=recv(sock,incoming+len,1490-len,0);
#ifndef _WIN32
		if(read<0 && (errno==ECONNRESET || errno==ENOTCONN)) {
			printf("Connection to server is lost.\n");
			exit(0);
		}
		if(read<0 && errno!=EAGAIN) {
			printf("server socket error %d\n",errno);
		}
#else
		int err=WSAGetLastError();
		if(read<0 && (err==WSAENETRESET)) {
			printf("Connection to server is lost.\n");
			exit(0);	
		}
		if(read<0 && err!=WSAEWOULDBLOCK) {
			printf("server socket error %d\n",errno);
		}
#endif
		if(read<=0) return;
#ifdef _PSP
		int i;
		for(i=0;i<9;i++) 
			netTimes[i]=netTimes[i+1];
		netTimes[10]=SDL_GetTicks();
#endif
		incoming[len+read]=0;
		incoming[len+read+1]=0;
		do {
			char *s=strchr(incoming,'\r');
			if(!s) s=strchr(incoming,'\n');
			if(!s) return;	// not a whole command yet.
			s[0]=0;
			if(strlen(incoming)>0) {
				// dispatch command.
				dispatchMessage(incoming);
				strcpy(incoming,s+1);
			}
		} while(strlen(incoming)>0);
	} while(1);
}

void wall()
{
	char msg[1492];
	sprintf(msg,"W%s",wallMsg);
	wallMsg[0]=0;
	sendMessage(msg);
}

void block()
{
	sendMessage("B");
}

void shoot()
{
	sendMessage("S");
}

void sendDir()
{
	char msg[64];
	int dir=-1;
	switch(dirMask) {
	case 1: dir=0; break;
	case 1+2: dir=1; break;
	case 2: dir=2; break;
	case 2+4: dir=3; break;
	case 4: dir=4; break;
	case 4+8: dir=5; break;
	case 8: dir=6; break;
	case 8+1: dir=7; break;
	}
	sprintf(msg,"M%d",dir);
	sendMessage(msg);
}

void dirDown(int i)
{
	dirMask|=i;
	sendDir();
}

void dirUp(int i)
{
	dirMask&=~i;
	sendDir();
}

void blitStat(SDL_Surface *surf,int value,int x,int y)
{
	SDL_Rect target={x+4,y,0,0};
	SDL_Rect src={(value<4?value:4)*15,0,15,16};
	SDL_BlitSurface(surf,&src,screen,&target);
	SDL_Rect target2={x+30,y,0,0};
	SDL_Rect src2={(value>4?value<8?value-4:4:0)*15,0,15,16};
	SDL_BlitSurface(surf,&src2,screen,&target2);
	
}

/* This function draws to the screen; replace this with your own code! */
void draw()
{
    Uint32 color;
    char text[255];
    int top=0;
    frame++;

#ifdef _PSP
    if(player[0].active) {
        top=player[0].y/2-272/2;
	if(top<0) top=0;
	if(top+272>480) top=480-272;
    }
#endif

    /* Create a black background */
    color=SDL_MapRGB(screen->format, 192, 255, 192);
    SDL_FillRect(screen, NULL, color);

	int i,j;
	for(j=0;j<gridHeight;j++) {
		for(i=0;i<gridWidth;i++) {
			SDL_Rect rect={i*30/2,j*30/2-top,30/2,30/2};
			if(grid[i+j*gridWidth]==GRID_NONE) continue;	// blank
			switch(grid[i+j*gridWidth]) {
			case GRID_NONE:
				color=SDL_MapRGB(screen->format,0,0,0); break;
			case GRID_BARRIER:
				color=SDL_MapRGB(screen->format,0,255,0); break;
			case GRID_WAYPOINT:
				color=SDL_MapRGB(screen->format,0,0,255); break;
			}
			SDL_FillRect(screen,&rect,color);
		}
	}
	for(i=0;i<MAXPLAYER;i++) {
		if(player[i].active==0) continue;
		if(player[i].blockTimer>0) {	// shield
			SDL_Rect rect={player[i].x/2-5-4,player[i].y/2-5-4-top,10+8,10+8};
			color=SDL_MapRGB(screen->format,128,128,128);
			SDL_FillRect(screen,&rect,color);
		}
		SDL_Rect rect={player[i].x/2-5,player[i].y/2-5-top,10,10};
		if(player[i].sprite) {
			int yy,xx;
			yy=player[i].facing*16+16;
			xx=((frame/8)%2)*16;
			if(player[i].dir==-1) {
				xx=player[i].facing*16;
				yy=0;
			}
			SDL_Rect src={xx,yy,16,16};
			rect.w=16;
			rect.h=16;
			SDL_BlitSurface(player[i].sprite,&src,screen,&rect);
		} else {
			color=SDL_MapRGB(screen->format,255,(i%4)*255/3,0);
			SDL_FillRect(screen,&rect,color);
		}

		drawText(i*90,screen->h-90,player[i].name);
		SDL_Rect infoRect={i*90,screen->h-75,0,0};
		SDL_BlitSurface(hudImage,NULL,screen,&infoRect);
		blitStat(statImage[0],player[i].hit,i*90,screen->h-75);
		blitStat(statImage[1],player[i].mana,i*90,screen->h-74+18);
		blitStat(statImage[2],player[i].block,i*90,screen->h-73+18+18);
		blitStat(statImage[3],player[i].waypoint,i*90,screen->h-72+18+18+18);
	}

	for(i=0;i<MAXFIREBALL;i++) {
		if(fireball[i].active==0) continue;
		SDL_Rect rect={fireball[i].x/2-5,fireball[i].y/2-5-top,10,10};
		color=SDL_MapRGB(screen->format,128,255,64);
		SDL_FillRect(screen,&rect,color);
	}
	
	if(mana.active) {
		SDL_Rect rect={mana.x/2+3,mana.y/2+3-top,10,10};
		color=SDL_MapRGB(screen->format,64,128,0);
		SDL_FillRect(screen,&rect,color);
	}

	if(beginTimer>0) {
		sprintf(text,"Begin in %ds",beginTimer/1000);
		int w,h;
		getSize(text,&w,&h);
		drawText(screen->w/2-w/2,screen->h/2-h/2,text);
	}
	if(billTimer>0) {
		sprintf(text,"Make a new rule (%ds)",billTimer/1000);
		int w,h;
		getSize(text,&w,&h);
		drawText(screen->w/2-w/2,h*3,text);
		sprintf(text,"Rule: %d of %s",voteNumber,typeName[voteType]);
		getSize(text,&w,&h);
		drawText(960/4-w/2,h*4,text);
	}
	if(voteTimer>0) {
		sprintf(text,"Vote on rule (%d seconds remaining)",voteTimer/1000);
		int w,h;
		getSize(text,&w,&h);
		drawText(screen->w/2-w/2,h,text);
		sprintf(text,"Rule: %d of %s",voteNumber,typeName[voteType]);
		getSize(text,&w,&h);
		drawText(screen->w/2-w/2,h*2,text);
	}
	if(winTimer>0) {
		char *name="Unknown";
		struct Player *p=getPlayer(winPid);
		if(p) name=p->name;
		sprintf(text,"Winner is %s. Game exits in %ds",name,winTimer/1000);
		if(!p) winTimer=0;
		int w,h;
		getSize(text,&w,&h);
		drawText(screen->w/2-w/2,screen->h/2+h/2,text);
	}
	if(gameMode==MODE_LOBBY) {
		const char *usage[]={
#ifndef _PSP
			"You are in the lobby.  For a list of users hit F10.",
			"To create a new game hit F12. To join a game hit F11.",
			"To see a list of active games hit F9.",
			"To quit hit ESC."
#else 
			"You are in the lobby.  For a list of users hit L trigger.",
			"To create a new game hit START. To join a game hit SELECT.",
			"To see a list of active games hit R trigger.",
			"To quit hit HOME."
#endif
		};
		for(i=0;i<4;i++) {
			drawText(0,15+15*i,usage[i]);
		}
	}
	if(gameMode==MODE_JOINED) {
		const char *keymap[]={
#ifndef _PSP
			"To talk, type your message, and send it with ENTER/RETURN",
			"Once the game begins, move with the arrow keys,",
			"Shoot fireballs with alt, block with tab.",
			"Make a rule with [, ], Page Up, Page Down and submit with END.",
			"Vote with Page Up (yes) or Page Down (no)."
#else
			"To talk, hit square and type your message.",
			"Once the game begins, move with the d-pad,",
			"Shoot fireballs with X, block with O.",
			"Make a rule with L trigger and R trigger and submit with triangle.",
			"Vote with L trigger (yes) or R trigger (no)."
#endif

		};
		for(i=0;i<5;i++) {
			drawText(0,15+15*i,keymap[i]);
		}
	}
	if(strlen(wallMsg)>0) drawText(100,0,wallMsg);
	if(gameMode!=MODE_GAME) drawText(0,0,modeName[gameMode]);
	for(i=0;i<noticeListCount;i++) {
		//if(noticeList[i].timer<=0) continue;
		int y=screen->h-95-15*i;
		int w,h;
		getSize(noticeList[i].text,&w,&h);
		int x=screen->w/2-w/2;
		drawText(x,y,noticeList[i].text);
	}
#ifdef _PSP
	static int oldTicks;
	int newTicks=SDL_GetTicks();
	int elapsed=newTicks-oldTicks;
	oldTicks=newTicks;
	float fps=1;
	if(elapsed>0) fps=1000.0f/elapsed;
	char fpsText[64];
	sprintf(fpsText,"fps %.2f (%d ms)",fps,elapsed);
	drawText(360,0,fpsText);
#endif

    /* Make sure everything is displayed on screen */
    SDL_Flip(screen);
    /* Don't run too fast */
#ifndef _PSP
    SDL_Delay(16);
#endif
}

void update(int elapsed)
{
	int i;
	for(i=0;i<MAXPLAYER;i++) {
		if(player[i].active==0) continue;
#ifdef SMOOTH_INTERPOLATION
		int frame;
		for(frame=0;frame<elapsed+5;frame+=11) {
			player[i].x=0.98f*player[i].x+0.02f*player[i].serverX;
			player[i].y=0.98f*player[i].y+0.02f*player[i].serverY;
		}
#else
		player[i].x=player[i].serverX;
		player[i].y=player[i].serverY;
#endif
		if(player[i].blockTimer>0) {
			player[i].blockTimer-=elapsed;
			if(player[i].blockTimer<0) player[i].blockTimer=0;
		}
	}
	if(beginTimer>0) {
		beginTimer-=elapsed;
		if(beginTimer<=0) beginTimer=0;
	}
	if(beginTimer==0 && gameMode==MODE_JOINED) gameMode=MODE_GAME;
	if(billTimer>0) {
		billTimer-=elapsed;
		if(billTimer<0) billTimer=0;
	}
	if(voteTimer>0) {
		voteTimer-=elapsed;
		if(voteTimer<0) voteTimer=0;
	}
	if(winTimer>0) {
		winTimer-=elapsed;
		if(winTimer<0) winTimer=0;
	}
	for(i=0;i<noticeListCount;i++) {
		noticeList[i].timer-=elapsed;
		if(noticeList[i].timer<=0) {
			int j;
			if(noticeList[i].text)
				free(noticeList[i].text);
			noticeList[i].text=0;
			for(j=i+1;j<noticeListCount;j++) {
				noticeList[j-1]=noticeList[j];
				noticeList[j].text=0;
				noticeList[j].timer=0;
			}
			noticeListCount--;
		}
	}
}

int main(int argc, char *argv[])
{
    int done;
    //char *name="testuser";
	//char *server="127.0.0.1";
	char *server="74.208.70.195";

#ifdef _PSP
	char nickname[128];
	sceUtilityGetSystemParamString(PSP_SYSTEMPARAM_ID_STRING_NICKNAME, nickname, 128);
	name=nickname;
#endif
	if(argc>1) {
		name=argv[1];
	}
	if(argc>2) {
		server=argv[2];
	}

    /* Initialize SDL */
    if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)<0) {
        printf ("Couldn't initialize SDL: %s\n", SDL_GetError ());
        exit(1);
    }
    atexit(SDL_Quit);

    /* Set 640x480 16-bits video mode */
#ifdef _PSP
    screen=SDL_SetVideoMode( 480,272,16,SDL_HWSURFACE|SDL_DOUBLEBUF);
#else
    screen=SDL_SetVideoMode( 960/2,960/2,16,SDL_SWSURFACE|SDL_DOUBLEBUF);
#endif
    if (screen == NULL) {
        printf("Couldn't set 960x960x16 video mode: %s\n",SDL_GetError());
        exit(2);
    }
    SDL_WM_SetCaption ("PSP FTW 2D", NULL);

	SDL_Joystick *joy=0;
	if(SDL_NumJoysticks()>0)
		joy=SDL_JoystickOpen(0);

#ifdef _PSP
	printf("Starting networking...\n");
	startNetworking();
#endif
	SDL_Surface *titleSurface=IMG_Load("data/title.png");
	if(titleSurface) {
		SDL_BlitSurface(titleSurface,NULL,screen,NULL);
		SDL_Flip(screen);
		SDL_Delay(1000);
		SDL_FreeSurface(titleSurface);
		titleSurface=0;
	}
	titleSurface=IMG_Load("data/compo.png");
	if(titleSurface) {
		SDL_BlitSurface(titleSurface,NULL,screen,NULL);
		SDL_Flip(screen);
		SDL_Delay(1000);
		SDL_FreeSurface(titleSurface);
		titleSurface=0;
	}

	TTF_Init();
	font=TTF_OpenFont("LiberationSans-Regular.ttf",15);
	if(!font) {
		printf("Can't find font Arial.ttf\n");
		exit(4);
	}
	printf("Connecting to server at %s:7110\n",server);
	int result=gameConnect(server);
	if(result==0) return 10;

	int i;
	for(i=0;i<spriteCount;i++) {
		sprite[i]=IMG_Load(spriteName[i]);
	}
	hudImage=IMG_Load("data/infodisp.png");
	statImage[0]=IMG_Load("data/barsfirepspftw.png");
	statImage[1]=IMG_Load("data/barsmanapspftw.png");
	statImage[2]=IMG_Load("data/barsblockspspftw.png");
	statImage[3]=IMG_Load("data/barswaypointspspftw.png");

	int newTicks,oldTicks;
	newTicks=SDL_GetTicks();
    done = 0;
    while (!done) {
        SDL_Event event;

        /* Check for events */
        while( SDL_PollEvent(&event))
        {
            switch( event.type) {
            case SDL_KEYDOWN:
				if(event.key.keysym.sym==SDLK_RIGHT) dirDown(1);
				if(event.key.keysym.sym==SDLK_DOWN) dirDown(2);
				if(event.key.keysym.sym==SDLK_LEFT) dirDown(4);
				if(event.key.keysym.sym==SDLK_UP) dirDown(8);
                break;
		case SDL_JOYBUTTONDOWN:
			if(event.jbutton.button==9) dirDown(1);	// right
			if(event.jbutton.button==6) dirDown(2);	// down
			if(event.jbutton.button==7) dirDown(4); // left
			if(event.jbutton.button==8) dirDown(8); // up
			break;
		case SDL_JOYBUTTONUP:
			switch( event.jbutton.button) {
				case 9: dirUp(1); break; // right
				case 6: dirUp(2); break; // down
				case 7: dirUp(4); break; // left
				case 8: dirUp(8); break; // up
				case 5:	// rtrigger
					if(billTimer) {
						voteType++;
						if(voteType>3) voteType=0;
					} else if(voteTimer) {
						sendMessage("V0");
					} else {
						sendMessage("L");
					}
					break;
				case 4:	// ltrigger
					if(billTimer) {
						voteNumber++;
						if(voteNumber>8) voteNumber=1;
					} else if(voteTimer) {
						sendMessage("V1");
					} else {
						sendMessage("T");
					}
					break;
				case 0:	// triangle
					if(billTimer) {
						char msg[256];
						sprintf(msg,"R%d,%d",voteType,voteNumber);
						sendMessage(msg);
					}
					break;
				case 2: shoot(); break;	// cross
				case 1: block(); break;	// circle
				case 3: 
#ifdef _PSP
					osk(wallMsg,256);
#endif
					wall(); 
					break;	// square -- should do OSK
				case 10: // select
					if(latestGid>0) {
						char msg[256];
						sprintf(msg,"J%d",latestGid);
						sendMessage(msg);
					}
					break;
				case 11: sendMessage("N"); break; // start
				/*
				case 3: // square
					if(strlen(wallMsg)>0) wallMsg[strlen(wallMsg)-1]=0;
					break;
				*/
				}
                break;

            case SDL_KEYUP:
				switch(	event.key.keysym.sym) {
				case SDLK_ESCAPE: done=1; break;
				case SDLK_RIGHT: dirUp(1); break;
				case SDLK_DOWN: dirUp(2); break;
				case SDLK_LEFT: dirUp(4); break;
				case SDLK_UP: dirUp(8); break;
				case '[':
					if(billTimer) {
						voteType--;
						if(voteType<0) voteType=3;
					}
					break;
				case ']':
					if(billTimer) {
						voteType++;
						if(voteType>3) voteType=0;
					}
					break;
				case SDLK_PAGEDOWN:
					if(billTimer) {
						voteNumber--;
						if(voteNumber<1) voteNumber=8;
					} else if(voteTimer) {
						sendMessage("V0");
					}
					break;
				case SDLK_PAGEUP:
					if(billTimer) {
						voteNumber++;
						if(voteNumber>8) voteNumber=1;
					} else if(voteTimer) {
						sendMessage("V1");
					}
					break;
				case SDLK_END:
					if(billTimer) {
						char msg[256];
						sprintf(msg,"R%d,%d",voteType,voteNumber);
						sendMessage(msg);
					}
					break;
				case SDLK_LALT: shoot(); break;
				case SDLK_TAB: block(); break;
				case SDLK_RETURN: wall(); break;
				case SDLK_F9: sendMessage("L"); break;
				case SDLK_F10: sendMessage("T"); break;
				case SDLK_F11:
					if(latestGid>0) {
						char msg[256];
						sprintf(msg,"J%d",latestGid);
						sendMessage(msg);
					}
					break;
				case SDLK_F12: sendMessage("N"); break;
				case SDLK_BACKSPACE:
					if(strlen(wallMsg)>0) wallMsg[strlen(wallMsg)-1]=0;
					break;
				default:
					if(	event.key.keysym.sym>=' ' && event.key.keysym.sym<256) {
						int len=strlen(wallMsg);
						if(len>254) wall();	// auto split.
						wallMsg[len]=event.key.keysym.sym;
						wallMsg[len+1]=0;
					}
				}
                break;
            case SDL_QUIT:
                done=1;
                break;
            default:
                break;
            }
        }
        /* Draw to screen */
        draw();
        dispatchNetwork();
		oldTicks=newTicks;
		newTicks=SDL_GetTicks();
        update(newTicks-oldTicks);
    }

#ifdef _PSP
	gameDisconnect();
	stopNetworking();
#endif

    return 0;
}
