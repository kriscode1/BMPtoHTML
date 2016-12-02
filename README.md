# BMPtoHTML
Converts a BMP image to an HTML table of colored cells.

Usage:

BMPtoHTML.exe \[input_bmp_filename] \[output_html_filename]

The output HTML file contains a table of cells with a width and height of 1 pixel. Each td tag in the table is given a background color to recreate the BMP image. This program is a proof of concept showing that html tables can be constructed to display an image, without giving the client an image file. 

The first example image tested is 571 KB in size, and the corresponding output HTML file is 5.38 MB (which is why I can't paste it here). The browser also takes a noticable number of seconds to render the HTML. The large increase in file size, long rendering time, and complete abuse of HTML tables likely makes the downsides of using this technique outweigh any upsides. It's amusing to see it work though. 
