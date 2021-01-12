# Ghost-Engine-Classic-2D

### Creating a Game Engine from Raycasting to Binary Space Partitioning. Everything runs on an Arduino Microcontroller and is then displayed on an OLED Display.


The important thing to know about the project is that I started with the raycasting algorithm for rendering the world (Like in Wolfenstein 3D). Then I used Binary Space Partitioning (Doom) and replaced the array data structure of the map with a binary tree, which contains the divided map and much more in the nodes.
The rendering is more complex. Visibility problems can be solved e.g. with traversal.



[Final Engine and Game on OLED Display](https://www.youtube.com/watch?v=XWTsxJl02wU)    
[Final Engine on LED Matrix](https://www.youtube.com/watch?v=WRVSrZ5XRVQ)                                             






Game:

The idea of the game is that the first player must place walls, enemies, start position and target position. The second player has to use a compass to find the target in the pseudo 3D environment without dying. He also has the ability to place walls to kill enemies.



<p align="center">
  <img src="/Media/Final_Game.png" width="400" alt="Final Game">
  <img src="/Media/Scene.png" width="400" alt="Scene from Engine">
  <img src="/Media/Visual_Scene.png" width="350" alt="Wall Rendering">
  <img src="/Media/Map_Editor.jpeg" width="408" alt="Map Editor">
  <img src="/Media/Wolfenstein 3D.jpeg" width="800" alt="Wolfenstein 3d">
</p>
