#include "Generation.h"
#include <stdlib.h>
#include <math.h>
#define random() rand()
#define srandom(x) srand(x)

Generation::Generation(void)
{
}


Generation::~Generation(void)
{
}

/*
 * randNum - Return a random floating point number such that
 *      (min <= return-value <= max)
 * 32,767 values are possible for any given range.
 */
static float randnum (float min, float max)
{
	int r;
    float	x;
    
	r = random ();
    x = (float)(r & 0x7fff) /
		(float)0x7fff;
    return (x * (max - min) + min);
} 


/*
 * fractRand is a useful interface to rand num.
 */
#define fractRand(v) randnum (-v, v)


/*
 * avgEndpoints - Given the i location and a stride to the data
 * values, return the average those data values. "i" can be thought of
 * as the data value in the centre of two line endpoints. We use
 * "stride" to get the values of the endpoints. Averaging them yields
 * the midpoint of the line.
 *
 * Called by fill1DFractArray.
 */
static float avgEndpoints (int i, int stride,TerrainClass::HeightMapType *fa)
{
    return ((float) (fa[i-stride].y +
		     fa[i+stride].y) * .5f);
}

/*
 * avgDiamondVals - Given the i,j location as the centre of a diamond,
 * average the data values at the four corners of the diamond and
 * return it. "Stride" represents the distance from the diamond centre
 * to a diamond corner.
 *
 * Called by fill2DFractArray.
 */
static float avgDiamondVals (int i, int j, int stride,
			     int size, int subSize, TerrainClass::HeightMapType *fa)
{
    /* In this diagram, our input stride is 1, the i,j location is
       indicated by "X", and the four value we want to average are
       "*"s:
           .   *   .

           *   X   *

           .   *   .
       */

    /* In order to support tiled surfaces which meet seamless at the
       edges (that is, they "wrap"), We need to be careful how we
       calculate averages when the i,j diamond centre lies on an edge
       of the array. The first four 'if' clauses handle these
       cases. The final 'else' clause handles the general case (in
       which i,j is not on an edge).
     */
    if (i == 0)
	return ((float) (fa[(i*size) + j-stride].y +
			 fa[(i*size) + j+stride].y +
			 fa[((subSize-stride)*size) + j].y +
			 fa[((i+stride)*size) + j].y) * .25f);
    else if (i == size-1)
	return ((float) (fa[(i*size) + j-stride].y +
			 fa[(i*size) + j+stride].y +
			 fa[((i-stride)*size) + j].y +
			 fa[((0+stride)*size) + j].y) * .25f);
    else if (j == 0)
	return ((float) (fa[((i-stride)*size) + j].y +
			 fa[((i+stride)*size) + j].y +
			 fa[(i*size) + j+stride].y +
			 fa[(i*size) + subSize-stride].y) * .25f);
    else if (j == size-1)
	return ((float) (fa[((i-stride)*size) + j].y +
			 fa[((i+stride)*size) + j].y +
			 fa[(i*size) + j-stride].y +
			 fa[(i*size) + 0+stride].y) * .25f);
    else
	return ((float) (fa[((i-stride)*size) + j].y +
			 fa[((i+stride)*size) + j].y +
			 fa[(i*size) + j-stride].y +
			 fa[(i*size) + j+stride].y) * .25f);
}


/*
 * avgSquareVals - Given the i,j location as the centre of a square,
 * average the data values at the four corners of the square and return
 * it. "Stride" represents half the length of one side of the square.
 *
 * Called by fill2DFractArray.
 */
static float avgSquareVals (int i, int j, int stride, int size, TerrainClass::HeightMapType *fa)
{
    /* In this diagram, our input stride is 1, the i,j location is
       indicated by "*", and the four value we want to average are
       "X"s:
           X   .   X

           .   *   .

           X   .   X
       */
    return ((float) (fa[((i-stride)*size) + j-stride].y +
		     fa[((i-stride)*size) + j+stride].y +
		     fa[((i+stride)*size) + j-stride].y +
		     fa[((i+stride)*size) + j+stride].y) * .25f);
}


bool Generation::MidPointDisplacement(TerrainClass::HeightMapType* mHeight, int size, int seedValue, float heightScale, float h)
{
	int	i, j;
    int	stride;
    int	oddline;
    int subSize;
	float ratio, scale;

    /* subSize is the dimension of the array in terms of connected line
       segments, while size is the dimension in terms of number of
       vertices's. */
    subSize = size;
    size++;
    
    /* initialize random number generator */
    srand(seedValue);
    
#ifdef DEBUG
    printf ("initialized\n");
    dump2DFractArray (fa, size);
#endif

	/* 
		Set up our roughness constants.
		Random numbers are always generated in the range 0.0 to 1.0.
		'scale' is multiplied by the random number.
		'ratio' is multiplied by 'scale' after each iteration
		to effectively reduce the random number range.
	*/
	ratio = (float) pow (2.0f,-h);
	scale = heightScale * ratio;

    /* Seed the first four values. For example, in a 4x4 array, we
       would initialize the data points indicated by '*':

           *   .   .   .   *

           .   .   .   .   .

           .   .   .   .   .

           .   .   .   .   .

           *   .   .   .   *

       In terms of the "diamond-square" algorithm, this gives us
       "squares".

       We want the four corners of the array to have the same
       point. This will allow us to tile the arrays next to each other
       such that they join seamlessly. */

    stride = subSize / 2;
    mHeight[(0*size)+0].y = 0.0f;
	mHeight[(subSize*size)+0].y = 0.0f;
	mHeight[(subSize*size)+subSize].y = 0.0f;
	mHeight[(0*size)+subSize].y = 0.0f;
    

    /* Now we add ever-increasing detail based on the "diamond" seeded
       values. We loop over stride, which gets cut in half at the
       bottom of the loop. Since it's an int, eventually division by 2
       will produce a zero result, terminating the loop. */
    while (stride) {
		/* Take the existing "square" data and produce "diamond"
		   data. On the first pass through with a 4x4 matrix, the
		   existing data is shown as "X"s, and we need to generate the
	       "*" now:

               X   .   .   .   X

               .   .   .   .   .

               .   .   *   .   .

               .   .   .   .   .

               X   .   .   .   X

	      It doesn't look like diamonds. What it actually is, for the
	      first pass, is the corners of four diamonds meeting at the
	      centre of the array. */
		for (i=stride; i<subSize; i+=stride) {
			for (j=stride; j<subSize; j+=stride) {
				mHeight[(i * size) + j].y = scale * fractRand (.5f) + avgSquareVals (i, j, stride, size, mHeight);
				j += stride;
			}
			i += stride;
		}

		/* Take the existing "diamond" data and make it into
	       "squares". Back to our 4X4 example: The first time we
	       encounter this code, the existing values are represented by
	       "X"s, and the values we want to generate here are "*"s:

               X   .   *   .   X

               .   .   .   .   .

               *   .   X   .   *

               .   .   .   .   .

               X   .   *   .   X

	       i and j represent our (x,y) position in the array. The
	       first value we want to generate is at (i=2,j=0), and we use
	       "odd line" and "stride" to increment j to the desired value.
	       */
		oddline = 0;
		for (i=0; i<subSize; i+=stride) {
		    oddline = (oddline == 0);
			for (j=0; j<subSize; j+=stride) {
				if ((oddline) && !j) j+=stride;

				/* i and j are setup. Call avgDiamondVals with the
				   current position. It will return the average of the
				   surrounding diamond data points. */
				mHeight[(i * size) + j].y =
					scale * fractRand (.5f) +
					avgDiamondVals (i, j, stride, size, subSize, mHeight);

				/* To wrap edges seamlessly, copy edge values around
				   to other side of array */
				if (i==0)
					mHeight[(subSize*size) + j] =
						mHeight[(i * size) + j];
				if (j==0)
					mHeight[(i*size) + subSize] =
						mHeight[(i * size) + j];

				j+=stride;
			}
		}

		/* reduce random number range. */
		scale *= ratio;
		stride >>= 1;
    }
	return true;
}
