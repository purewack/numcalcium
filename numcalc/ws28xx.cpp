#include "include/ws28xx.h"
#include "include/comms.h"

void ws_begin(uint8_t n){
  if(doubleBuffer) 
  {
	  free(doubleBuffer); 
  }

  numBytes = (n<<3) + n + 2; // 9 encoded bytes per pixel. 1 byte empty peamble to fix issue with WS_SPI_DEV MOSI and on byte at the end to clear down MOSI 
							// Note. (n<<3) +n is a fast way of doing n*9
  if((doubleBuffer = (uint8_t *)malloc(numBytes*2)))
  {
    numLEDs = n;	 
	pixels = doubleBuffer;
	// Only need to init the part of the double buffer which will be interacted with by the API e.g. setPixelColor
	*pixels=0;//clear the preamble byte
	*(pixels+numBytes-1)=0;// clear the post send cleardown byte.
	ws_clear();// Set the encoded data to all encoded zeros 
  } 
  else 
  {
    numLEDs = numBytes = 0;
  }

  SPI_2.setClockDivider(SPI_CLOCK_DIV16);// need bit rate of 400nS but closest we can do @ 72Mhz is 444ns (which is within spec)
  SPI_2.begin();
}

void ws_setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
 {
   uint8_t *bptr = pixels + (n<<3) + n +1;
   uint8_t *tPtr = (uint8_t *)encoderLookup + g*2 + g;// need to index 3 x g into the lookup
   
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;

   tPtr = (uint8_t *)encoderLookup + r*2 + r;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;   
   
   tPtr = (uint8_t *)encoderLookup + b*2 + b;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
 }

void ws_show(void) 
{
  SPI_2.dmaSendAsync(pixels,numBytes);// Start the DMA transfer of the current pixel buffer to the LEDs and return immediately.

  // Need to copy the last / current buffer to the other half of the double buffer as most API code does not rebuild the entire contents
  // from scratch. Often just a few pixels are changed e.g in a chaser effect
  
  if (pixels==doubleBuffer)
  {
	// pixels was using the first buffer
	pixels	= doubleBuffer+numBytes;  // set pixels to second buffer
	memcpy(pixels,doubleBuffer,numBytes);// copy first buffer to second buffer
  }
  else
  {
	// pixels was using the second buffer	  
	pixels	= doubleBuffer;  // set pixels to first buffer
	memcpy(pixels,doubleBuffer+numBytes,numBytes);	 // copy second buffer to first buffer 
  }	
}

void ws_clear() 
{
	uint8_t * bptr= pixels+1;// Note first byte in the buffer is a preable and is always zero. hence the +1
	uint8_t *tPtr;

	for(int i=0;i< (numLEDs *3);i++)
	{
	   tPtr = (uint8_t *)encoderLookup;
   	   *bptr++ = *tPtr++;
	   *bptr++ = *tPtr++;
	   *bptr++ = *tPtr++;	
	}
}
