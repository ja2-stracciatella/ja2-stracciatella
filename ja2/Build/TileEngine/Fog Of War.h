#ifndef __FOG_OF_WAR_H
#define __FOG_OF_WAR_H


//Called after a map is loaded.  By keying on the MAPELEMENT_REVEALED value, we can easily
//determine what is hidden by the fog.
void InitializeFogInWorld();

//Removes and smooths the adjacent tiles.
void RemoveFogFromGridNo( UINT32 uiGridNo );

#endif
