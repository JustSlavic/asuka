#include "asuka.hpp"


static void RenderGradient(Game_OffscreenBuffer* Buffer, int XOffset, int YOffset) {
    //                  Width->                                      Width * BytesPerPixel
    // BitmapMemory: 0  BB GG RR xx BB GG RR xx BB GG RR xx    ...   BB GG RR xx           <additional padding?>
    // BM + Pitch:   1  BB GG RR xx BB GG RR xx BB GG RR xx    ...   BB GG RR xx           <additional padding?>
    //
    // Pitch = Width * BytesPerPixel + Stride
    //

    int Pitch = Buffer->Width*Buffer->BytesPerPixel;
    uint8* Row = (uint8*)Buffer->Memory;

    for (int y = 0; y < Buffer->Height; y++) {
        uint32* Pixel = (uint32*) Row;
        for (int x = 0; x < Buffer->Width; x++) {
            //
            // Pixel in memory: 00 00 00 00
            //                  BB GG RR xx
            // BUT ARCHITECTURE IS LITTLE ENDIAN!!!
            // Pixel in registry: ARGB
            //
            uint8_t Blue = x + XOffset;
            uint8_t Green = y + YOffset;

            *Pixel = (Blue) | (Green << 8);
            Pixel++;
        }

        Row += Buffer->Pitch;
    }
}


void Game_UpdateAndRender(Game_OffscreenBuffer* Buffer, int XOffset, int YOffset) {
    RenderGradient(Buffer, XOffset, YOffset);
}
