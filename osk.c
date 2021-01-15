#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspgu.h>
#include <string.h>
#include <psputility.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>

int drawOskDialog();
int oskDialog();
int oskDialogActive=-1;

static unsigned int __attribute__((aligned(16))) gulist[8192];
static int stateLast=-1;
static unsigned short intext[256];
static unsigned short outtext[256];
static unsigned short desc[256];
static SceUtilityOskData data;
static SceUtilityOskParams params;

int oskDialog()
{
	memset(&data,0,sizeof(data));
	data.language = PSP_UTILITY_OSK_LANGUAGE_DEFAULT;
	data.lines = 1;
	data.unk_24 = 1;
	data.inputtype = PSP_UTILITY_OSK_INPUTTYPE_ALL;	// allow all input types

	int i;
	char *what="Enter your message.";
	for(i=0;i<strlen(what)+1;i++) {
		desc[i]=what[i];	// poor man's unicode.
	}
	intext[0]=0;
	outtext[0]=0;

	data.desc= desc;
	data.intext=intext;
	data.outtext=outtext;
	data.outtextlength=256;
	data.outtextlimit=256;

        memset(&params, 0, sizeof(params));
        params.base.size = sizeof(params);
        sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &params.base.language);
        sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &params.base.buttonSwap);
        params.base.graphicsThread = 17;
        params.base.accessThread = 19;
        params.base.fontThread = 18;
        params.base.soundThread = 16;
        params.datacount = 1;
        params.data = &data;

        sceUtilityOskInitStart(&params);

	oskDialogActive=-1;

	return 1;
}

// returns -1 on quit, 0 on active, and 1 on success
int drawOskDialog()
{
	int done=0;

	switch(sceUtilityOskGetStatus()) {
	case PSP_UTILITY_DIALOG_NONE: printf("None\n"); break;
	case PSP_UTILITY_DIALOG_INIT: break;
	case PSP_UTILITY_DIALOG_VISIBLE: 
		sceUtilityOskUpdate(1);
		break;
	case PSP_UTILITY_DIALOG_QUIT:
		printf("OskDialog was quit.\n");
		sceUtilityOskShutdownStart(); 
		done=-1;
		break;	// cancelled??
	case PSP_UTILITY_DIALOG_FINISHED: 
		printf("OskDialog completed successfully.\n");
		sceUtilityOskShutdownStart(); 
		done=1;
		break;
	default: 
		printf("OskGetStatus: %08x\n",sceUtilityOskGetStatus());
		break;
	}
	
	return done;
}

extern SDL_Surface *screen;

void osk(char *buffer,int size)
{
	printf("Starting dialog\n");
	oskDialog();
	int i;
	for(i=0;i<size;i++) {
		intext[i]=buffer[i];
	}
	while(1) {
		sceGuStart(GU_DIRECT,gulist);
		sceGuClearColor(GU_RGBA(0,128,128,255));
		sceGuClear(GU_COLOR_BUFFER_BIT);
		sceGuFinish();
		sceGuSync(0,0);
		int rc=drawOskDialog();
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
	for(i=0;i<size;i++) {
		buffer[i]=outtext[i];
	}
}

