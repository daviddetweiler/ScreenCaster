// DynamicTexture

#include "DynamicTexture.h"

// UTextures have a BPP of 4 (Red, Green, Blue, Alpha)
#define DYNAMIC_TEXTURE_BYTES_PER_PIXEL 4

void UDynamicTexture::Initialize(int32 InWidth, int32 InHeight, FLinearColor InClearColor, TextureFilter FilterMethod/* = TextureFilter::TF_Nearest*/)
{
	// Store the parameters
	TextureWidth = InWidth;
	TextureHeight = InHeight;
	ClearColor = InClearColor;

	// Create the UTexture2D to render to
	Texture = UTexture2D::CreateTransient(TextureWidth, TextureHeight);
	Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	Texture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap; // The VectorDisplacementMap is a raw RGBA8 format
	Texture->SRGB = 1;
	Texture->Filter = FilterMethod;
	Texture->UpdateResource();

	// Create the proxy object for updating the texture region
	UpdateTextureRegionProxy = MakeUnique<FUpdateTextureRegion2D>(0, 0, 0, 0, TextureWidth, TextureHeight);

	// Size of the image pixel buffer
	BufferSize = TextureWidth * TextureHeight * DYNAMIC_TEXTURE_BYTES_PER_PIXEL;
	PixelBuffer = MakeUnique<uint8[]>(BufferSize);

	// Initially clear the texture
	Clear();
}

void UDynamicTexture::Fill(FLinearColor Color)
{
	// Get the base pointer of the pixel buffer
	uint8* Ptr = PixelBuffer.Get();

	// Iterate over all pixels
	for (int i = 0; i < TextureWidth * TextureHeight; ++i)
	{
		// Set the pixel
		SetPixelInternal(Ptr, Color.R * 255, Color.G * 255, Color.B * 255, Color.A * 255);

		// Advance to the next pixel
		Ptr += DYNAMIC_TEXTURE_BYTES_PER_PIXEL;
	}
}

void UDynamicTexture::Clear()
{
	// Fill with the clear color
	Fill(ClearColor);
}

UTexture2D* UDynamicTexture::GetTextureResource()
{
	return Texture;
}

void UDynamicTexture::SetPixelInternal(uint8*& Ptr, uint8 Red, uint8 Green, uint8 Blue, uint8 Alpha)
{
	// Pixels are stored in BGRA format
	*Ptr = Blue;
	*(Ptr + 1) = Green;
	*(Ptr + 2) = Red;
	*(Ptr + 3) = Alpha;
}

uint8* UDynamicTexture::GetPointerToPixel(int32 X, int32 Y)
{
	// The calculation of the pointer address of a given pixel is
	// base + ((x + (y * width)) * bpp)
	return (PixelBuffer.Get() + ((X + (Y * TextureWidth)) * DYNAMIC_TEXTURE_BYTES_PER_PIXEL));
}

void UDynamicTexture::UpdateTexture()
{
	// Make sure the proxy and the texture is valid
	if (UpdateTextureRegionProxy.IsValid() && Texture)
	{
		// Update the texture's regions
		Texture->UpdateTextureRegions(
			0,											// Mip index
			1,											// Number of regions
			UpdateTextureRegionProxy.Get(),				// Region proxy
			TextureWidth * DYNAMIC_TEXTURE_BYTES_PER_PIXEL,	// Source data pitch
			DYNAMIC_TEXTURE_BYTES_PER_PIXEL,			// Bytes per pixel of source data
			PixelBuffer.Get()							// Buffer of pixels to set
		);
	}
}

int32 UDynamicTexture::GetWidth()
{
	return TextureWidth;
}

int32 UDynamicTexture::GetHeight()
{
	return TextureHeight;
}

uint8* UDynamicTexture::GetBuffer()
{
	return PixelBuffer.Get();
}

uint64 UDynamicTexture::GetBufferSize()
{
	return BufferSize;
}
