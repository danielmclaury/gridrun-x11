#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define UNDERWAY	0
#define WON		1
#define LOST		2
#define CHEATED		3

int main(int argc, char **argv) {
  Display *dpy;
  Window w;
  Pixmap p;
  Colormap c;
  GC gc;
  XColor red, green, blue, yellow;
  int black, white;
  XEvent e;
  KeySym ks;

  char turn_s[4];
  int computers, has_obj;
  int px, py, *cx, *cy, ox, oy;
  int status, turn, i, cheats, invincible;

  if(argc > 1) {
    sscanf(argv[1], "%d", &computers);
    if(computers < 1) computers = 1;
    if(computers > 48) computers = 12;
  } else {
    computers = 4;
  }

  srand(time(0));
  dpy = XOpenDisplay(0);

  white = WhitePixel(dpy, DefaultScreen(dpy));
  black = BlackPixel(dpy, DefaultScreen(dpy));

  w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 600, 600, 0, 0,
	black);
  XMapWindow(dpy, w);
  XFlush(dpy);

  gc = XCreateGC(dpy, w, 0, 0);

  c = DefaultColormap(dpy, DefaultScreen(dpy));
  XParseColor(dpy, c, "red", &red);
  XParseColor(dpy, c, "green", &green);
  XParseColor(dpy, c, "blue", &blue);
  XParseColor(dpy, c, "yellow", &yellow);
  XAllocColor(dpy, c, &red);
  XAllocColor(dpy, c, &blue);
  XAllocColor(dpy, c, &green);
  XAllocColor(dpy, c, &yellow);
  XFlush(dpy);

  p = XCreatePixmap(dpy, w, 600, 600, DefaultDepth(dpy, DefaultScreen(dpy)));

  XSelectInput(dpy, w, KeyPressMask | ExposureMask);

  XmbSetWMProperties(dpy, w, "GridRun by Daniel McLaury", "", 0, 1, 0, 0, 0);
  XFlush(dpy);

  cx = (int *) malloc(computers * sizeof(int));
  cy = (int *) malloc(computers * sizeof(int));

  for(i = 0; i < computers; i++) {
    cx[i] = i * 24 / computers;
    cy[i] = (rand() % 3) * 2;
  }

  px = 0; py = 24;
  ox = oy = 12;
  has_obj = 0;
  turn = 0; status = UNDERWAY;
  cheats = invincible = 0;

  while(status == UNDERWAY) {
    turn++;
    if((turn >= 75) && has_obj) status = WON;

    // Draw background on double buffer
    XSetForeground(dpy, gc, black);
    XFillRectangle(dpy, p, gc, 0, 0, 600, 600);
    XSetForeground(dpy, gc, green.pixel);
    XFlush(dpy);
    for(i = 0; i < 12; i++) {
      XDrawLine(dpy, p, gc, 0, i*50, 600, i*50);
      XDrawLine(dpy, p, gc, i*50, 0, i*50, 600);
      XFlush(dpy);
    }

    // Put the turn in the upper left corner
    sprintf(turn_s, "%c%d", (has_obj ? '*' : ' '), turn);
    XSetForeground(dpy, gc, white);
    XDrawImageString(dpy, p, gc, 20, 20, turn_s, strlen(turn_s));
    XFlush(dpy);

    // Draw the object to get
    if(! has_obj) {
            XSetForeground(dpy, gc, yellow.pixel);
            XFillRectangle(dpy, p, gc, 25 * ox - 10, 25 * oy - 10, 20, 20);
            XFlush(dpy);
    }

    // Draw the player
    XSetForeground(dpy, gc, blue.pixel);
    if(invincible) XSetForeground(dpy, gc, white);
    XFillRectangle(dpy, p, gc, px * 25 - 10, py * 25 - 10, 20, 20);
    XFlush(dpy);

    // Draw the computers
    XSetForeground(dpy, gc, red.pixel);
    for(i = 0; i < computers; i++)
      XFillRectangle(dpy, p, gc, cx[i] * 25 - 10, cy[i] * 25 - 10, 20, 20);
    XFlush(dpy);

    // Show the double buffer
    XCopyArea(dpy, p, w, gc, 0, 0, 600, 600, 0, 0);
    XFlush(dpy);

    // Get and process input
    e.type = 0;
    while(e.type != KeyPress) {
	if(e.type == Expose) XCopyArea(dpy, p, w, gc, 0, 0, 600, 600, 0, 0);
	XNextEvent(dpy, &e);
    }
    ks = XKeycodeToKeysym(dpy, e.xkey.keycode, 0);
    switch(ks) {
	case XK_Up:
	case XK_KP_Up:
	  if((py > 0) && (px % 2 == 0)) py--;
	  break;
	case XK_Down:
	case XK_KP_Down:
	  if((py < 24) && (px %2 == 0)) py++;
	  break;
	case XK_Left:
	case XK_KP_Left:
	  if((px > 0) && (py % 2 == 0)) px--;
	  break;
	case XK_KP_Right:
	case XK_Right:
	  if((px < 24) && (py % 2 == 0)) px++;
	  break;
	case XK_q:
	  return 0;
	case XK_Tab:
	  cheats = ! cheats;
	  invincible = 0;
	  break;
	case XK_F5:
	  if(cheats) has_obj = 1;
	  break;
	case XK_F6:
	  if(cheats) computers--;
	  break;
	case XK_F7:
	  if(cheats) turn += 5;
	  break;
	case XK_F8:
	  if(cheats) invincible = ! invincible;
	default:
	  break;
    }

    if((px == ox) && (py == oy)) has_obj = 1;

    // Make the computers' moves.
    for(i = 0; i < computers; i++) {
     if((cx[i] < 0) || (cy[i] < 0) || (cx[i] > 24) || (cy[i] > 24)) continue;
     switch(rand() % 2) {
      case 0:
	if((cx[i] > px) && (cy[i] % 2 == 0)) cx[i] -= (turn % 2) + 1; 
	if((cx[i] < px) && (cy[i] % 2 == 0)) cx[i] += (turn % 2) + 1;
	break;
      case 1:
	if((cy[i] > py) && (cx[i] % 2 == 0)) cy[i] -= (turn % 2) + 1;
	if((cy[i] < py) && (cx[i] % 2 == 0)) cy[i] += (turn % 2) + 1;
	break;
     }

     if((px == cx[i]) && (py == cy[i]) && (! invincible)) status = LOST;
     if((ox == cx[i]) && (oy == cy[i])) {
	ox = 2*(rand() % 12);
	oy = 2*(rand() % 12);
     }
   }

  }

  // The game is over; show the final screen.
  XSetForeground(dpy, gc, black);
  XFillRectangle(dpy, w, gc, 0, 0, 600, 600);

  if(cheats && (status == WON)) status = CHEATED;
  switch(status) {
    case LOST:
        XSetForeground(dpy, gc, red.pixel);
        XDrawString(dpy, w, gc, 200, 250, "You Lose!", 9);
	break;
    case WON:
	XSetForeground(dpy, gc, blue.pixel);
	XDrawString(dpy, w, gc, 200, 250, "You Win!", 8);
	break;
    case CHEATED:
	XSetForeground(dpy, gc, red.pixel);
	XDrawString(dpy, w, gc, 200, 250, "Cheaters Never Prosper", 22);
	break;
    default:
	return 0;
  }

  XFlush(dpy);

  // Wait for keypress before termination.

  e.type = 0;
  sleep(2);
  while(e.type != KeyPress) XNextEvent(dpy, &e);

  return 0;
} 
