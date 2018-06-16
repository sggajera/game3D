/*____________________________________________________________________
|
| File: img_clr.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

void    Put_Image_Pixel_Color (byte *image_data, int x, int y, int dx, gxColor color);
gxColor Get_Image_Pixel_Color (byte *image_data, int x, int y, int dx);
