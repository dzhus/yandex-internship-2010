/// Unfinished attempt to implement digit recognition.
///
/// Copyright (C) 2010, 2011 by Dmitry Dzhus
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions
/// are met:
///
///    1. Redistributions of source code must retain the above
///    copyright notice, this list of conditions and the following
///    disclaimer.
///
///    2. Redistributions in binary form must reproduce the above
///    copyright notice, this list of conditions and the following
///    disclaimer in the documentation and/or other materials provided
///    with the distribution.
///
///    3. The name of the author may not be used to endorse or promote
///    products derived from this software without specific prior
///    written permission.

#include <iostream>
#include <vector>
#include <string>

/// True if (i, j) are coordinates of pixel outside of image with
/// given dimensions.
///
/// @important i and j are evaluated twice!
#define out_of_bounds(i, j, height, width)                              \
    ((((i) < 0) || ((j) < 0)) || (((i) >= (height)) || ((j) >= (width))))

/// Do body for every (i, j) in [0, 1, 2]^2.
/// 
/// @note From Lisp land with love.
#define for_neighbourhood(i, j, body)           \
    for (coord_t i = 0; i != 3; i++)            \
        for (coord_t j = 0; j != 3; j++)        \
            body;

using namespace std;

typedef unsigned char pixel_t;
typedef vector<pixel_t> image_row_t;
typedef vector<image_row_t> pixel_matrix_t;
typedef pixel_matrix_t::size_type coord_t;

/// Connected areas with area less than this value are considered
/// noise.
const unsigned int area_threshold = 20;

const double target_weight = .5;
const double weight_deviation = .05;

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
    /// Matrix with all pixels of image
    pixel_matrix_t pixels;

    unsigned int width;
    unsigned int height;

public:
    /// Distinctive characteristic props of the image
    struct ImageProperties
    {
        /// Total area of foreground pixels of the image
        const double area;
        
        /// Center of mass
        const Point com;
        
        /// Second moment about horizontal axis
        const double hor_moment;
        
        /// Second moment about vertical axis
        const double vert_moment;
        
        /// Second mixed moment
        const double mixed_moment;

        /// Foreground area to total image size ratio.
        const double weight;

        ImageProperties(double a, Point c, double hm, double vm, double mm, double w)
            :area(a), com(c), hor_moment(hm), vert_moment(vm), mixed_moment(mm),
             weight(w)
        {}
    };

public:
    Image(void)
        :width(0), height(0)
    {}

    /// Create new clear Image object with given dimensions
    Image(unsigned int iwidth, unsigned int iheight)
    {
        width = iwidth;
        height = iheight;
        
        image_row_t r;
        r.resize(width, 0);
        for (unsigned int i = 0; i!= height; i++)
            pixels.push_back(r);
    }

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
        
            for (unsigned int i = 0; i != width; i++)
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
        for (i = 0; i != height; i++)
        {
            for (j = 0; j != width; j++)
                out << (pixels[i][j] != 0);
            out << endl;
        }
    }
     
    unsigned int get_width(void)
    {
        return width;
    }

    unsigned int get_height(void)
    {
        return height;
    }

    /// Get structure populated with image props
    ///
    /// @see Image::ImageProperties
    ///
    /// @internal We don't use separate getter method for every moment
    /// to save time on matrix traversal.
    ImageProperties get_props(void) const
    {
        double a = 0, x = 0, y = 0;

        /// Find total area and center of mass
        /// @todo Write for_all(i, j, body) macro
        for (coord_t i = 0; i != height; i++)
            for (coord_t j = 0; j != width; j++)
                if (pixels[i][j])
                {
                    x += j;
                    y += i;
                    a++;
                }
        
        /// Find second moments
        Point com = Point(x / a, y / a);
        double hor_moment = 0, vert_moment = 0, mixed_moment = 0;
        int h, v;
        
        for (coord_t i = 0; i != height; i++)
            for (coord_t j = 0; j != width; j++)
                if (pixels[i][j])
                {
                    v = j - com.x;
                    h = i - com.y;
                    hor_moment += h * h / a;
                    vert_moment += v * v / a;
                    mixed_moment += h * v / a;
                }
    return ImageProperties(a, com, hor_moment, vert_moment, mixed_moment, 
                           a / (width * height));
    }

    /// Get value of pixel at coordinates (i, j) of the image.
    ///
    /// @internal Perhaps we should just store extra zero border
    /// around the actual image to avoid these comparisons every time
    /// we access image pixel.
    pixel_t get_pixel(coord_t i, coord_t j, pixel_t d = 0) const
    {
        if (out_of_bounds(i, j, height, width))
            return d;
        else
            return pixels[i][j];
    }
    
    /// Assign new value to pixal at coordinates (i, j)
    void set_pixel(pixel_t v, coord_t i, coord_t j)
    {
        if (!out_of_bounds(i, j, height, width))
            pixels[i][j] = v;
    }

    /// Resize image to the size of foreground bounding box
    void crop(void)
    {
        coord_t min_x = width, max_x = 0, min_y = height, max_y = 0;
        coord_t i, j;
        
        /// Calculate bounding box
        for (i = 0; i != height; i++)
            for (j = 0; j != width; j++)
                if (pixels[i][j])
                {
                    min_x = (min_x > j) ? j : min_x;
                    min_y = (min_y > i) ? i : min_y;

                    max_x = (max_x < j) ? j : max_x;
                    max_y = (max_y < i) ? i : max_y;
                }
        
        /// Cut borders
        pixels.erase(pixels.begin(), pixels.begin() + min_y);
        pixels.erase(pixels.begin() + max_y - min_y + 1, pixels.end());

        for (i = 0; i != pixels.size(); i++)
        {
            pixels[i].erase(pixels[i].begin(), pixels[i].begin() + min_x);
            pixels[i].erase(pixels[i].begin() + max_x - min_x + 1, pixels[i].end());
        }

        width = max_x - min_x + 1;
        height = max_y - min_y + 1;
    }

    /// Applies functional object m to every pixel of the image.
    Image& apply_mask(ImageMask &m)
    {
        coord_t i, j;
        
        Image copy(*this);

        for (i = 0; i != height; i++)
        {
            for (j = 0; j != width; j++)
                m(copy, i, j, *this);
        }
        return *this;
    }

    /// Move first found connected component to image t.
    ///
    /// Leftmost component is moved.
    ///
    /// @internal Recursive and slow
    ///
    /// @return True if component found, false otherwise.
    bool extract_connected(Image &t, coord_t i = 0, coord_t j = 0, bool start = true)
    {
        bool found = false;
        if (start)
        {
            /// Find any foreground pixel
            for (j = 0; j != width; j++)
            {
                for (i = 0; i != height; i++)
                    if (pixels[i][j])
                    {
                        found = true;
                        break;
                    }
                if (found)
                {
                    extract_connected(t, i, j, false);
                    break;
                }
            }
        }
        else
            if (pixels[i][j])
            {
                t.set_pixel(1, i, j);
                set_pixel(0, i, j);
                
                for_neighbourhood(m, n, extract_connected(t, i + m - 1, j + n - 1, false));
            }
        return found;
    }
};

class ErodeMask : public ImageMask
{
    void operator ()(const Image &source,
                     coord_t &row,
                     coord_t &col,
                     Image &target)
    {
        pixel_t target_pixel = true;
        
        for_neighbourhood(i, j, target_pixel &= source.get_pixel(row + i - 1, col + j - 1));

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
            for_neighbourhood(i, j, target.set_pixel(1, row + i - 1, col + j - 1));

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

/// *Try* to dilate or erode image until it gains weight within
/// provided bounds.
///
/// @important Works bad on small images due to large weight change
/// after each dilation/erosion.
void train_image(Image &i, double min_weight, double max_weight)
{
    DilateMask dilate;
    ErodeMask erode;
    Image::ImageProperties p = i.get_props();
    cout << p.weight << endl;

    while (i.get_props().weight < min_weight)
        i.apply_mask(dilate);

    while (i.get_props().weight > max_weight)
        i.apply_mask(erode);
}



int main(int argc, char* argv[])
{
    string s;
    Image i;
    bool extracted = false;
    ErodeMask m;
    DilateMask n;

    cin >> i;

    /// Read connected components of image from left to right
    do
    {
        Image j = Image(i.get_width(), i.get_height());
        extracted = i.extract_connected(j);
        if (extracted)
        {
            j.crop();
            train_image(j, target_weight - weight_deviation, target_weight + weight_deviation);
            
            Image::ImageProperties p = j.get_props();
            if (p.area > area_threshold)
            {
                cout << j;
                cout << p.area << " (" << p.com.x << ", " << p.com.y << ")";
                cout << " H: " << p.hor_moment << " V: " << p.vert_moment;
                cout << " M: " << p.mixed_moment;
                cout << " w: " << p.weight << endl;
                /// @todo Resize to canonical size, compare moments
            }
        }
    } while (extracted);

    return 0;
}
