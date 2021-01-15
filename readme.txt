------------------------------------------------------------------------
r139 | hardhat | 2011-05-29 23:39:48 -0400 (Sun, 29 May 2011) | 6 lines

Added title screen.
Added compo screen.
Added icon.
Made text have drop shadow for better readability.
Fixed text for players in the lobby.

------------------------------------------------------------------------
r79 | hardhat | 2011-03-30 21:41:14 -0400 (Wed, 30 Mar 2011) | 4 lines

Added yields in the main thread, so that the network thread could make some
packets ready for me, on the PSP.
Tuned the error handling on Winsock.

------------------------------------------------------------------------
r78 | hardhat | 2011-03-30 12:07:23 -0400 (Wed, 30 Mar 2011) | 2 lines

Made 2D client work on Windows.

------------------------------------------------------------------------
r77 | hardhat | 2011-03-30 11:07:41 -0400 (Wed, 30 Mar 2011) | 7 lines

Added OSK for the PSP.
Changed help text for the PSP
Added FPS counter.
Tried TCP_NODELAY but I think it makes it worse somehow.
Win timer stops as soon as the winner leaves the game.
Fixed up shoulder button mappings.

------------------------------------------------------------------------
r76 | hardhat | 2011-03-30 00:55:52 -0400 (Wed, 30 Mar 2011) | 4 lines

Added scrolling when compiled for the PSP.
Worked on getting the quit message to get through to the server.
Cleans up networking on exit now.

------------------------------------------------------------------------
r75 | hardhat | 2011-03-29 21:19:14 -0400 (Tue, 29 Mar 2011) | 2 lines

Made it all work well on the PSP.

------------------------------------------------------------------------
r74 | hardhat | 2011-03-28 23:06:23 -0400 (Mon, 28 Mar 2011) | 3 lines

Fixed compile errors in network.c
Tweaked makefile for Mac installed SDL for PSP

------------------------------------------------------------------------
r73 | hardhat | 2011-03-28 17:50:17 -0400 (Mon, 28 Mar 2011) | 2 lines

Refined network.c, but didn't work on compiling yet.

------------------------------------------------------------------------
r72 | hardhat | 2011-03-28 17:37:49 -0400 (Mon, 28 Mar 2011) | 2 lines

Altered code to make the SDL 2D client perhaps work on the PSP screen.

------------------------------------------------------------------------
r71 | hardhat | 2011-03-27 23:43:31 -0400 (Sun, 27 Mar 2011) | 3 lines

Now parses parameters for top and games list correctly.
Make run less noisy on stdout.

------------------------------------------------------------------------
r68 | hardhat | 2011-03-27 19:13:35 -0400 (Sun, 27 Mar 2011) | 2 lines

Added backspace to edit your wall messages.

------------------------------------------------------------------------
r67 | hardhat | 2011-03-27 17:37:35 -0400 (Sun, 27 Mar 2011) | 2 lines

Changed default server address.

------------------------------------------------------------------------
r66 | hardhat | 2011-03-27 17:18:27 -0400 (Sun, 27 Mar 2011) | 2 lines

Updated for late joining of the game.

------------------------------------------------------------------------
r64 | hardhat | 2011-03-27 17:03:14 -0400 (Sun, 27 Mar 2011) | 2 lines

Added new font.  Fixed voting server crash.

------------------------------------------------------------------------
r62 | hardhat | 2011-03-27 15:57:21 -0400 (Sun, 27 Mar 2011) | 4 lines

Added help text for Lobby and Joined modes.
Added voting keybindings.
Added welcome message with delayed identify.

------------------------------------------------------------------------
r60 | hardhat | 2011-03-27 02:53:38 -0400 (Sun, 27 Mar 2011) | 2 lines

Made top and games list notices last longer.

------------------------------------------------------------------------
r59 | hardhat | 2011-03-27 02:44:59 -0400 (Sun, 27 Mar 2011) | 2 lines

Fixed double free bug in notices.

------------------------------------------------------------------------
r58 | hardhat | 2011-03-27 02:25:14 -0400 (Sun, 27 Mar 2011) | 2 lines

2d client now displays notices on screen.

------------------------------------------------------------------------
r52 | hardhat | 2011-03-26 22:15:10 -0400 (Sat, 26 Mar 2011) | 2 lines

Improved blocking visualization and symantics.

------------------------------------------------------------------------
r51 | hardhat | 2011-03-26 19:25:28 -0400 (Sat, 26 Mar 2011) | 4 lines

Added text overlays.
Added more accurate timers.
Refined game play.

------------------------------------------------------------------------
r49 | hardhat | 2011-03-26 17:22:33 -0400 (Sat, 26 Mar 2011) | 3 lines

Made socket error handling better
Added initial draw code.

------------------------------------------------------------------------
r48 | hardhat | 2011-03-26 15:41:07 -0400 (Sat, 26 Mar 2011) | 2 lines

First pass at implementing all server messages for psp ftw client2d.

------------------------------------------------------------------------
r47 | hardhat | 2011-03-26 14:31:49 -0400 (Sat, 26 Mar 2011) | 2 lines

First pass at a Makefile for client2d on OS X and Linux.

------------------------------------------------------------------------
r46 | hardhat | 2011-03-26 14:31:17 -0400 (Sat, 26 Mar 2011) | 4 lines

Added empty handlers for most aspects of the game.
Worked on implementing some handlers.
Made code compile for SDL on Mac.

------------------------------------------------------------------------
r38 | hardhat | 2011-03-20 13:26:54 -0400 (Sun, 20 Mar 2011) | 2 lines

Test client in 2D.

------------------------------------------------------------------------

