#include <stdlib.h>
#include <pspkernel.h>

#undef main
extern int SDL_main(int argc, char *argv[]);

PSP_MODULE_INFO("PSP FTW!", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER|THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-3192);

void gameDisconnect();

#if 1
int sdl_psp_exit_callback(int arg1, int arg2, void *common)
{
	gameDisconnect();
        exit(0);
        return 0;
}

int sdl_psp_callback_thread(SceSize args, void *argp)
{
        int cbid;
        cbid = sceKernelCreateCallback("Exit Callback", sdl_psp_exit_callback, NULL);
        sceKernelRegisterExitCallback(cbid);
        sceKernelSleepThreadCB();
        return 0;
}

int sdl_psp_setup_callbacks(void)
{
        int thid = 0;
        thid = sceKernelCreateThread("update_thread", sdl_psp_callback_thread, 0x11, 0xFA0, 0, 0);
        if(thid >= 0) sceKernelStartThread(thid, 0, 0);
        return thid;
}
#endif


#if 1
int main(int argc,char **argv)
{
	pspDebugScreenInit();
	sdl_psp_setup_callbacks();

	/* Register sceKernelExitGame() to be called when we exit */
	atexit(sceKernelExitGame);

	(void)SDL_main(argc, argv);
	return 0;
}
#endif

