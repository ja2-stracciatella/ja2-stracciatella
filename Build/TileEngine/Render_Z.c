static inline INT16 GetMapXYWorldY(INT32 WorldCellX, INT32 WorldCellY)
{
	INT16 RDistToCenterX = WorldCellX * CELL_X_SIZE - gCenterWorldX;
	INT16 RDistToCenterY = WorldCellY * CELL_Y_SIZE - gCenterWorldY;
	INT16 RScreenCenterY = RDistToCenterX + RDistToCenterY;
	return RScreenCenterY + gsCY - gsTLY;
}
