import os
import sys
import imageio
import argparse
import numpy as np
from skimage.transform import resize
import matplotlib.pyplot as plt

"""
To install dependencies, run:
pip3 install imageio numpy scikit-image matplotlib
"""


class RET:
    OK = 0
    FAIL = 1


# Printing options
def eprint(*s):
    args = s
    print("[error] ".rjust(10), *args)


def iprint(*s):
    args = s
    print("[info] ".rjust(10), *args)


def make_parser():
    parser=argparse.ArgumentParser()

    parser.add_argument("-i", "--input", required=True, help="Input image")
    parser.add_argument("-m", "--mode", choices=['image'], required=True, help="Generator mode")
    parser.add_argument("-o", "--output", required=True, help="Output HTML file")
    parser.add_argument("-w", "--width", required=True, type=int, help="Desired width of the image")

    return parser


def process_image(input_path, desired_width):
    ret = RET.OK
    im = None

    if ret == RET.OK:
        im = imageio.imread(input_path)
        iprint("Image size:", im.shape)
        
        # Remove transparency channel
        if im.shape[2] == 4:
            im[im[:,:,3] != 255] = [255, 255, 255, 0]
            im = im[:, :, :3]

        if im.shape[2] != 3:
            eprint("Image must have 3 or 4 channels")
            ret = RET.FAIL

    if ret == RET.OK:
        # Resize to fixed width
        output_width = desired_width * 10
        output_height = int(output_width * im.shape[0] / im.shape[1])
        im = resize(im, (output_height, output_width))
        iprint("Resized to:", im.shape)

        # Pad to multiple of 10 with light gray
        fill_value = 200
        pad_h = im.shape[1] % 10
        pad_v = im.shape[0] % 10
        if pad_h: pad_h = 10 - pad_h
        if pad_v: pad_v = 10 - pad_v
        im = np.pad(im, [(0, pad_v), (0, pad_h), (0, 0)], mode='constant', constant_values=fill_value*1./255)
        iprint("Size after padding:", im.shape)
        
        # Downscale after padding
        im = resize(im, (im.shape[0] / 10, im.shape[1] / 10))
        iprint("Downscaled to:", im.shape)

    return ret, im


def get_class_from_color(r, g, b):
    cl = ""
    min_thr = 20
    max_thr = 255 - 20

    if r > max_thr and g < min_thr and b < min_thr:
        cl = "color-almost-red"
    elif r < min_thr and g > max_thr and b < min_thr:
        cl = "color-almost-green"
    elif r < min_thr and g < min_thr and b > max_thr:
        cl = "color-almost-blue"
    elif r < min_thr and g < min_thr and b < min_thr:
        cl = "color-almost-black"
    elif r > max_thr and g > max_thr and b > max_thr:
        cl = "color-almost-white"
    elif r > max_thr and g > max_thr and b < min_thr:
        cl = "color-almost-yellow"
    elif r < min_thr and g > max_thr and b > max_thr:
        cl = "color-almost-cyan"
    elif r > max_thr and g < min_thr and b > max_thr:
        cl = "color-almost-magenta"
    else:
        cl = "color-other"

    return cl


def generate_html_from_image(image, title):
    ret = RET.OK
    h = ""

    # Formatting does not matter at this step
    if ret == RET.OK:
        # Head
        h += '<html xmlns="http://www.w3.org/1999/xhtml">'
        h += '<head>' + '<title>%s</title>' % (title) + '</head>'

        # Body
        h += '<body style="margin: 0;">'

        # Containers
        h += '<div style="margin: auto; width: 100%; height: 100%;">'
        h += '<div style="position: relative; left:50%; top: 50%; float: left;transform: translate(-50%,-50%);">'

        # One div per image pixel
        for i in range(0, image.shape[0]):
            for j in range(0, image.shape[1]):
                div_height_px = 20
                div_width_px = 20
                tr_h = 50
                tr_v = 50
                style = ""

                # Style attributes
                color = image[i][j] * 255
                size = 'width: %dpx;height: %dpx;' % (div_width_px, div_height_px)

                style += '-webkit-transform: translate(-%d%%,-%d%%);float: left;' % (tr_h, tr_v)
                style += size
                if j == 0: style += 'clear:left;'
                style += 'background-color: rgb(%d, %d, %d);' % (color[0], color[1], color[2])

                # Avoid Firefox misalignment
                if j % 2 == 1: 
                    tr_h += 5
                if i % 2 == 0:
                    tr_v -= 5
                style += '-moz-transform: translate(-%d%%,-%d%%);float: left;' % (tr_h, tr_v)

                # Class
                cl = get_class_from_color(color[0], color[1], color[2])

                # Text
                text_style = "font-family: monospace; display:block;text-align:center;"
                r = (i + j) % 4
                if r == 0:
                    text_content = '<b>o</b>'
                elif r == 1:
                    text_content = '<em>x</em>'
                elif r == 2:
                    text_content = '<div><u>o</u></div>'
                else:
                    text_content = '<div class="uppercase">I</div>'
                text = '<span class="text" style="%s">%s</span>' % (text_style, text_content)

                # Compose string
                h += '<div class="%s" style="%s">%s</div>' % (cl, style, text)

        # Closing tags
        h += '</div></div></body></html>'

    return ret, h


def main():
    ret = RET.OK
    
    # Parse arguments
    parser = make_parser()
    args = parser.parse_args()
    iprint(args)

    if ret == RET.OK:
        if not os.path.isfile(args.input):
            eprint("Input file does not exist")
            ret = RET.FAIL

    # Load image
    if ret == RET.OK:
        if (args.mode == "image"):
            ret, im = process_image(args.input, args.width)
        else:
            eprint("Mode not implemented")
            ret = RET.FAIL

    # Generate HTML structure
    if ret == RET.OK:
        # imgplot = plt.imshow(im)
        # plt.show()
        if (args.mode == "image"):
            ret, html = generate_html_from_image(im, os.path.basename(args.input))

    # Save to file
    if ret == RET.OK:
        try:
            with open(args.output, 'w') as fo:
                fo.write(html)
        except Exception as e:
            eprint(str(e))
            ret = RET.FAIL

    if ret == RET.OK:
        iprint("File generated:", args.output)

    return ret


if __name__ == "__main__":
    ret = main()
    sys.exit(ret)
