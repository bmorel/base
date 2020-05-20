#include <algorithm>

#include <GL/gl.h>
#include <SDL2/SDL.h>

#include "imagedata.hpp"

ImageData::ImageData()
    : m_data(nullptr), owner(nullptr), freefunc(nullptr)
{}


ImageData::ImageData(int nw, int nh, int nbpp, int nlevels, int nalign, GLenum ncompressed)
    : m_data(nullptr), owner(nullptr), freefunc(nullptr)
{
    setdata(nw, nh, nbpp, nlevels, nalign, ncompressed);
}

ImageData::ImageData(int nw, int nh, int nbpp, uint8_t *data)
    : m_data(nullptr), owner(nullptr), freefunc(nullptr)
{
    setdata(m_data, nw, nh, nbpp);
}

ImageData::ImageData(SDL_Surface *s)
{
    wrap(s);
}

ImageData::~ImageData()
{
    cleanup();
}

void ImageData::setdata(uint8_t *ndata, int nw, int nh, int nbpp, int nlevels, int nalign, GLenum ncompressed)
{
    if( !ndata )
		{
        setdata( nw,  nh,  nbpp,  nlevels,  nalign, ncompressed);
		    return;
		}

    cleanup();
    w = nw;
    h = nh;
    bpp = nbpp;
    levels = nlevels;
    align = nalign;
    pitch = align ? 0 : w*bpp;
    compressed = ncompressed;
    m_data = ndata;
}

void ImageData::setdata(int nw, int nh, int nbpp, int nlevels, int nalign, GLenum ncompressed)
{
    cleanup();
    w = nw;
    h = nh;
    bpp = nbpp;
    levels = nlevels;
    align = nalign;
    pitch = align ? 0 : w*bpp;
    compressed = ncompressed;
    m_data = new uint8_t[calcsize()];
    owner = this;
    freefunc = nullptr;
}

int ImageData::calclevelsize(int level) const
{
    return ((std::max(w>>level, 1)+align-1)/align)*((std::max(h>>level, 1)+align-1)/align)*bpp;
}

int ImageData::calcsize() const
{
    if(!align)
    {
        return w*h*bpp;
    }
    int lw = w, lh = h, size = 0;
    for( int i = 0; i < levels; ++i )
    {
        if(lw<=0)
        {
            lw = 1;
        }
        if(lh<=0)
        {
            lh = 1;
        }
        size += ((lw+align-1)/align)*((lh+align-1)/align)*bpp;
        if(lw*lh==1)
        {
            break;
        }
        lw >>= 1;
        lh >>= 1;
    }
    return size;
}

void ImageData::cleanup()
{
    if(owner==this)
    {
        delete[] m_data;
        m_data = nullptr;
    }
    else if(freefunc)
    {
        (*freefunc)(owner);
        owner = nullptr;
        freefunc = nullptr;
    }
}

void ImageData::replace(ImageData &d)
{
    cleanup();
    *this = d;
    if(owner == &d)
    {
        owner = this;
    }
    d.m_data   = nullptr;
    d.owner    = nullptr;
    d.freefunc = nullptr;
}

void ImageData::wrap(SDL_Surface *s)
{
    setdata(static_cast<uint8_t*>( s->pixels ), s->w, s->h, s->format->BytesPerPixel);
    pitch = s->pitch;
    owner = s;
    freefunc = (void (*)(void *))SDL_FreeSurface;
}

uint8_t const* ImageData::data( void ) const
{
	return m_data;
}

uint8_t * ImageData::data( void )
{
	return m_data;
}
