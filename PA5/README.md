Programming Assignment 02
=========================

When a program runs, a cube will rotate but will not spin.  You must make the cube spin by selecting "Start spinning" from the menu.

Menu
----

Right click brings up the menu.  It has three selections: Start spinning, pause spinning and quit.

Spinning Direction
------------------

Pressing a keyboard key 'A' or 'a', or a left mouse click will reverse the spinning direction of the cube whenthe cube is already spinning.  If the cube is not spinning, it will have no effect.


Problems
--------

When I paused and resumed spinning the cube, the cube  would reset its spin angle.  Same with changing the direction of the spin - when changing the direction of the spin, the cube reset its angle before spinning instead of continuing from its current angle.  This problem was solved by changing the way spinning angle was calculated.
