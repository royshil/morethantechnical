GrabCut implementation source code Copyright(c) 2005-2006 Justin Talbot

All Rights Reserved.
For educational use only; commercial use expressly forbidden.
NO WARRANTY, express or implied, for this software.

Uses Graphcut implementation by Vladimir Kolmogorov (vnk@cs.cornell.edu), 2001.

Requires: OpenGL, GLUT, and OpenCV libraries to compile and run.

Usage: grabcut.exe <ppm filename>

Use mouse to drag a rectangle around the foreground portion of the displayed image.
Then use the following keys

'1': Show image
'2': Show GMM component assignment. Red shades are foreground components; green are background components.
'3': Show N-link weights. White is a large average N-link weight around a pixel, black is a low average N-link weight.
'4': Show T-link weights. Red component is foreground T-link weight, green component is background T-link weight.

' ' (space bar): Show/hide the computed alpha mask.

'o': Run one step of the GrabCut refinement algorithm
'r': Run the GrabCut refinement algorithm to convergence.
'l': Run the Orchard-Bouman clustering algorithm again. (It's run during initialization automatically.)

Escape: Stop refinement algorithm (after pressing 'r')

'q': Quit.