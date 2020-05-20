#ifndef IMAGEDATA_HPP
#define IMAGEDATA_HPP

struct ImageData
{
private:
    uint8_t *m_data;
    void *owner;
    void (*freefunc)(void *);
public: //TODO remove
    int w;
    int h;
    int bpp;
    int pitch;
    GLenum compressed;
    int levels;
    int align;

public:
    ImageData();
    ImageData(int nw, int nh, int nbpp, int nlevels = 1, int nalign = 0, GLenum ncompressed = GL_FALSE);
    ImageData(int nw, int nh, int nbpp, uint8_t *data);
    ImageData(SDL_Surface *s);
    ~ImageData();
    void setdata(uint8_t *ndata, int nw, int nh, int nbpp, int nlevels = 1, int nalign = 0, GLenum ncompressed = GL_FALSE);
    void setdata(int nw, int nh, int nbpp, int nlevels = 1, int nalign = 0, GLenum ncompressed = GL_FALSE);
    int calclevelsize(int level) const;
    int calcsize() const;
    void cleanup();
    void replace(ImageData &d);
    void wrap(SDL_Surface *s);

    uint8_t const* data( void ) const;
    uint8_t * data( void );
};

#endif
