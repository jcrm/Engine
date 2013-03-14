#ifndef _Generation_h_
#define _Generation_h_

#include "terrainclass.h"
class Generation
{
public:
	Generation(void);
	~Generation(void);

	static bool MidPointDisplacement(TerrainClass::HeightMapType* mHeight, int size, int seedValue, float heightScale, float h);
};

#endif // _Generation_h_
