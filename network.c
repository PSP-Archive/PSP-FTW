#include <pspgu.h>
#include <pspgum.h>
#include <pspsdk.h>
#include <psputility.h>
#include <pspdisplay.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <psphttp.h>
#include <pspopenpsid.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>

void notice( const char *msg,int duration);

int drawNetDialog();
int netDialog();
int netDialogActive=-1;
int httpTemplate;
void stopNetworking();

void loadNetworkingLibs()
{
	int rc=sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	if(rc<0) printf("net common didn't load.\n");
	rc=sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
	if(rc<0) printf("inet didn't load.\n");
}
	

int goOnline()
{
	sceNetInit(128*1024, 42, 4*1024, 42, 4*1024);
	sceNetInetInit();
	sceNetApctlInit(0x8000, 48);
	sceNetResolverInit();
	if(!netDialog()) {
		notice("Could not access networking dialog!",30000);
		stopNetworking();
		return 1;
	}
	//httpInit();
	return 0;
}

static unsigned int __attribute__((aligned(16))) gulist[8192];
static int stateLast=-1;
#ifdef _PSP
pspUtilityNetconfData data;
#endif
int netDialog()
{
#if _PSP_FW_VERSION >= 200
	memset(&data,0,sizeof(data));
	data.base.size = sizeof(data);
	data.base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
	data.base.buttonSwap = PSP_UTILITY_ACCEPT_CROSS;
	data.base.graphicsThread = 17;
	data.base.accessThread = 19;
	data.base.fontThread = 18;
	data.base.soundThread = 16;
	data.action = PSP_NETCONF_ACTION_CONNECTAP;

//#if  _PSP_FW_VERSION < 200
//		data.base.size=sizeof(pspUtilityNetconfData)-12;
//#endif
	netDialogActive=-1;
	int result=sceUtilityNetconfInitStart(&data);
	printf("sceUtilityNetconfInitStart: %08x\n",result);
	if(result<0) {
		data.base.size=sizeof(pspUtilityNetconfData)-12;
		result=sceUtilityNetconfInitStart(&data);
		printf("sceUtilityNetconfInitStart again: %08x\n",result);
		if(result<0) return 0;
	}
	netDialogActive=0;
#else
	netDialogActive=0;
	/* Connect using the first profile */
	int config=1;
	printf("Making connection to apctl\n");
#ifdef _PSP
	int err = sceNetApctlConnect(config);
	stateLast=-1;
	if(err!=0) {
	        printf("PSP FTW: sceNetApctlConnect returns %08X\n", err);
	        return -1;
	}
#endif
	printf("PSP FTW: Connecting...\n");
#endif

	return 1;
}

// returns -1 on quit, 0 on active, and 1 on success
int drawNetDialog()
{
	int done=0;

	switch(sceUtilityNetconfGetStatus()) {
	case PSP_UTILITY_DIALOG_NONE: printf("None\n"); break;
	case PSP_UTILITY_DIALOG_INIT: break;
	case PSP_UTILITY_DIALOG_VISIBLE: 
		sceUtilityNetconfUpdate(1);
		break;
	case PSP_UTILITY_DIALOG_QUIT:
		printf("NetDialog was quit.\n");
		sceUtilityNetconfShutdownStart(); 
		done=-1;
		break;	// cancelled??
	case PSP_UTILITY_DIALOG_FINISHED: 
		printf("NetDialog completed successfully.\n");
		sceUtilityNetconfShutdownStart(); 
		done=1;
		break;
	default: 
		printf("NetconfGetStatus: %08x\n",sceUtilityNetconfGetStatus());
		break;
	}
	
	return done;
}

char *urlEncode(const char *msg)
{
	static char out[512];
	
	int i;
	int o=0;
	for(i=0;i<strlen(msg);i++) {
		if(msg[i]==' ') out[o++]='+';
		else if(msg[i]>126 || msg[i]=='+' || msg[i]=='=' || msg[i]=='&' || msg[i]=='?' || msg[i]<32) { sprintf(out+o,"%%%02x",msg[i]); o+=3; }
		else out[o++]=msg[i];
	}
	out[o]=0;
	
	return out;
}

extern SDL_Surface *screen;
extern int done;

void startNetworking()
{
	printf("Loading libs.\n");
	loadNetworkingLibs();
	printf("Initing networking\n");
	goOnline();

	printf("Starting dialog\n");
	while(1) {
#if 0
		sceGuStart(GU_DIRECT,gulist);
		sceGuClearColor(GU_RGBA(0,128,128,255));
		sceGuClear(GU_COLOR_BUFFER_BIT);
		sceGuFinish();
		sceGuSync(0,0);
#else
		Uint32 color=SDL_MapRGB(screen->format,0,128,128);
		SDL_FillRect(screen, NULL, color);
#endif
		int rc=drawNetDialog();
		if(rc<0) {
			netDialog();
			rc=0;
		}
		if(rc!=0) break;
#if 0
		int page=(int)sceGuSwapBuffers();
		pspDebugScreenSetOffset(page);
		sceDisplayWaitVblankStart();
#else
		SDL_Flip(screen);
		sceDisplayWaitVblankStart();
#endif
	}
	atexit(stopNetworking);
}

void stopNetworking()
{
	// De-initialize networking
	//httpTerm();
	sceNetResolverTerm();
	sceNetApctlTerm();
	sceNetInetTerm();
	sceNetTerm();
}

