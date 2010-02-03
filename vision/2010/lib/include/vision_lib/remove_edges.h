#ifndef __SEAWOLF_VISION_LIB_REMOVE_EDGES_INCLUDE_H
#define __SEAWOLF_VISION_LIB_REMOVE_EDGES_INCLUDE_H

void remove_edges_init();
IplImage* remove_edges(IplImage* img, IplImage* edge, int rmin, int rmax, int bmin, int bmax, int gmin, int gmax);
void remove_edges_free();

#endif
