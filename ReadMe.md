### Graphical Maze
----
* Use **Stack** data structure to solve find exit Path
* Stack itself keeps track of previous location
* Before Visting new location in Maze **Push** it in Stack and if next position is not exit location **Pop** previous location

#### Algorithm/Rules For Traversal
---

|Decrement Col| |Increment Col| |
|:-------:|:---:|:--------:|:-------:|
|N.A|-1,0 |N.A|**Decrement Row**|
|0,-1|0,0 |0,1| |
|N.A|1,0 |N.A|**Increment Row**|

---

* Consider Maze as **2D Array** 
* Store Starting Position & Current Direction in Stack (It may be possible Start=End)
* For Each position we have to Visit all possible directions (No diagonal Movement So Max 4)
* Prepare 2D array for book keeping purpose and mark each position visited
* Push Row,Col,Dir in Stack iff Position is not Visited
* Manipluate Row & Col using above logic and Traverse Maze
* Check is New Row,Col is Exit position or Not if not then increment direction
* If all direction are visited then Pop Previous Position Direction
* Continue above till Exit Position is Found

#### Implementation
---
* Paste a Texture over Quad
* Read **sampleMaze.txt** and init Maze Each **1** corresponds to Brick and **0** as open path
* Use each value and prepare 8x8 portion of Texture
* Use Values form Path LinkList in Display Color that 8x8 Texture Portion to Green and Conditionally to Orange(glGetTexImage(), glTexSubImage2D() does everthing for me)

##### Robustness
---
* Works well with happy and Single Successfull Maze Path

#### Video
---
![video](file.mp4)

---

![video](file1.mp4)