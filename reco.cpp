#include <iostream>
#include <vector>
#include <string>

using namespace std;

typedef bool pixel_t;
typedef vector<pixel_t> image_row_t;
typedef vector<image_row_t> pixel_matrix_t;
typedef pixel_matrix_t::size_type coord_t;

struct Point
{
    coord_t x;
    coord_t y;

    Point(coord_t ix, coord_t iy)
        :x(ix), y(iy)
    {}
};

class Image;

/// Class of functional objects which represent mask operations on
/// Images.
class ImageMask
{
public:
    /// Implementations of this method must set new pixel values on
    /// target image given the source image and current position of
    /// mask center.
    virtual void operator ()(const Image &source,
                                coord_t &row,
                                coord_t &col,
                                Image &target) = 0;
};

/// Binary image
class Image
{
private:
    pixel_matrix_t pixels;
    unsigned int width;
    unsigned int height;

public:
    Image()
        :width(0), height(0)
    {}

    /// Read image from stream.
    ///
    /// All lines read into the image must have equal length.
    void read(istream &in)
    {
        string s;

        while (in)
        {
            image_row_t r;
            in >> s;

            width = s.length();

            r.resize(width, 0);
        
            for (unsigned int i = 0; i != s.length(); i++)
                /// Simple threshold binarization
                r[i] = (s[i] == '#' || s[i] == '@');
            
            pixels.push_back(r);
            height++;
        }
    }

    /// Send whole image to stream
    void write(ostream &out)
    {
        coord_t i, j;
        cout << width << " " << height << endl;
        for (i = 0; i != pixels.size(); i++)
        {
            for (j = 0; j != pixels[i].size(); j++)
                out << (pixels[i][j] != 0);
            out << endl;
        }
    }

    /// Calculate total area of foreground pixels on image
    unsigned int get_area()
    {
        unsigned int area = 0;

        for (coord_t i = 0; i != pixels.size(); i++)
            for (coord_t j = 0; j != pixels[i].size(); j++)
                area += pixels[i][j];

        return area;
    }

    /// Calculate center of mass
    Point get_mass_center()
    {
        unsigned int x = 0, y = 0, area = get_area();
        for (coord_t i = 0; i != pixels.size(); i++)
            for (coord_t j = 0; j != pixels[i].size(); j++)
                if (pixels[i][j])
                {
                    x += j;
                    y += i;
                }
        return Point(x / area, y / area);
    }

    /// Get value of pixel at coordinates (i, j) of the image.
    ///
    /// @internal Perhaps we should just store extra zero border
    /// around the actual image to avoid these comparisons every time
    /// we access image pixel.
    pixel_t get_pixel(coord_t i, coord_t j, pixel_t d = 0) const
    {
        if (((i < 0) || (j < 0)) || ((i >= height) || (j >= width)))
            return d;
        else
            return pixels[i][j];
    }
    
    /// Assign new value to pixal at coordinates (i, j)
    void set_pixel(pixel_t v, coord_t i, coord_t j)
    {
        if (!(((i < 0) || (j < 0)) || ((i >= height) || (j >= width))))
            pixels[i][j] = v;
    }

    /// Applies functional object m to every pixel of the image.
    Image& apply_mask(ImageMask &m)
    {
        coord_t i, j;
        
        Image copy(*this);

        for (i = 0; i != pixels.size(); i++)
        {
            for (j = 0; j != pixels[i].size(); j++)
                m(copy, i, j, *this);
        }
        return *this;
    }
};

class ErodeMask : public ImageMask
{
    void operator ()(const Image &source,
                     coord_t &row,
                     coord_t &col,
                     Image &target)
    {
        bool target_pixel = true;
        
        for (coord_t i = 0; i != 2; i++)
            for (coord_t j = 0; j != 2; j++)
                target_pixel &= source.get_pixel(row + i - 1, col + j - 1);

        target.set_pixel(target_pixel, row, col);
    }
};

class DilateMask : public ImageMask
{
    void operator ()(const Image &source,
                     coord_t &row,
                     coord_t &col,
                     Image &target)
    {
        if (source.get_pixel(row, col))
            for (coord_t i = 0; i != 2; i++)
                for (coord_t j = 0; j != 2; j++)
                    target.set_pixel(1, row + i - 1, col + j - 1);

    }
};

istream& operator >>(istream &in, Image &i)
{
    i.read(in);
    return in;
}

ostream& operator <<(ostream &out, Image &i)
{
    i.write(out);
    return out;
}

int main(int argc, char* argv[])
{
    string s;
    Image i, j, k;
    ErodeMask m;
    DilateMask n;
    /// Read first row to find out image width
    while (cin)
        cin >> i;

    j = i;
    cout << i;
    cout << i.get_area() << " " << i.get_mass_center().x << " " << i.get_mass_center().y << endl;

    i.apply_mask(m).apply_mask(n);
    cout << i;
    j.apply_mask(n).apply_mask(m);
    

    return 0;
}
