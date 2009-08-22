/*
 * Minimal implementation of PanoramiX/Xinerama
 */
/* $XFree86: xc/programs/Xserver/hw/darwin/quartz/pseudoramiX.h,v 1.1 2002/03/28 02:21:18 torrey Exp $ */

extern int noPseudoramiXExtension;

void PseudoramiXAddScreen(int x, int y, int w, int h);
void PseudoramiXExtensionInit(int argc, char *argv[]);
void PseudoramiXResetScreens (void);
