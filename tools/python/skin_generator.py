#!/usr/bin/env python

import sys
import os
import shutil
from cairosvg import svg2png


def generate_skin(style_suffix, skin_type, dpi_level, symbol_size,
                  styles_dir, tmp_dir, output_dir):
    png_symbols_path = os.path.join(styles_dir, 'clear', 'style' + style_suffix, dpi_level + skin_type)
    svg_symbols_path = os.path.join(styles_dir, 'clear', 'style' + style_suffix, 'symbols' + skin_type)
    if os.path.exists(png_symbols_path):
        shutil.copytree(png_symbols_path, tmp_dir)
    for file in os.listdir(svg_symbols_path):
        if file.endswith('.svg'):
            svg_path = os.path.join(svg_symbols_path, file)
            output_png_path = os.path.join(tmp_dir, file[:-4] + '.png')
            svg2png(url=svg_path, write_to=output_png_path, output_width=symbol_size, output_height=symbol_size)

if len(sys.argv) < 2:
    print('Usage: {0} <path_to_omim/data/styles>'.format(sys.argv[0]))
    sys.exit() 

path_to_styles = sys.argv[1]
if not os.path.isdir(path_to_styles):
    print('Invalid path to styles folder')
    sys.exit()

style_suffixes = ['-clear', '-night']
style_suffixes_output = ['_clear', '_dark']
skins = {
    # default skin
    '' : {
        # size of a symbol for specified DPI level.
        'mdpi' : 18, 'hdpi' : 27, 'xhdpi' : 36, 'xxhdpi' : 54, '6plus' : 54, 'xxxhdpi' : 64
    },
    # ads skin
    '-ad' : {
        # size of a symbol for specified DPI level.
        'mdpi' : 22, 'hdpi' : 34, 'xhdpi' : 44, 'xxhdpi' : 68, '6plus' : 68, 'xxxhdpi' : 78
    }
}

tmp_dir = 'tmp'
for i in range(0, len(style_suffixes)):
    for (skin_type, dpi_levels) in skins.items():
        for (dpi_level, symbol_size) in dpi_levels.items():
            output_dir = os.path.join(path_to_styles, '..', 'resources-' + dpi_level + style_suffixes_output[i])
            if os.path.exists(tmp_dir):
                shutil.rmtree(tmp_dir)
            if not os.path.exists(output_dir):
                os.makedirs(output_dir)
            print("Generating for: {0}, {1}, {2}".format(style_suffixes[i], dpi_level, skin_type))
            generate_skin(style_suffixes[i], skin_type, dpi_level, symbol_size, path_to_styles, tmp_dir, output_dir)
            sys.exit()
if os.path.exists(tmp_dir):
    shutil.rmtree(tmp_dir)
