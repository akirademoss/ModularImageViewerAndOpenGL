# ModularImageViewerAndOpenGL

## Basic overview
After playing around with JUCE for a couple days, I realized there were some major drawbacks to the layout system.  After some researching (okay, I admit, just a lot of google searching) I was fortunate enough to encounter several clever folks had drafted up  some solutions to this issue so here is a basic proof of concept illustrating the usefulness of these dock-able windows developed by [Jim](https://github.com/jcredland) with a tangible use case developed by myself. If you view the [Demo Video](https://www.youtube.com/watch?v=wiGaL7ElxRE), you will notice the window will update when an image file is selected.  In future work, we would like to additionally update the window so that the OpenGL View updates when a .obj file is selected, but for now we demonstrate the proof of concept with an image viewer.

## Initial Screen
![initial screen](https://user-images.githubusercontent.com/8731829/37601381-e1f986ea-2b57-11e8-89e2-a5f29903f2bf.png)

**Figure 1:**  Initial start-up screen. Future updates include removing tabs and starting at a screen similar to image **3)**.


##  Window Dock
![modularscreen demo1](https://user-images.githubusercontent.com/8731829/37601400-ec4f1024-2b57-11e8-9bd4-c3dba542917a.png)

**Figure 2:**  Demonstration of the docking window created by [Jim](https://github.com/jcredland) and found [here](https://github.com/jcredland/dockable-windows).  As mentioned in the description, dockable windows shows how windows can be dragged away from the main application window, creating a new desktop window, and then dragged back to the main application. This is similar to photoshop and visual studio.

##  Window locked into position 
![modularscreen demo2](https://user-images.githubusercontent.com/8731829/37601423-f944da70-2b57-11e8-9c4a-4edfbc0db0f9.png)

**Figure 3:**  What we would image the initial screen to look like ideally.



## File browser use-case
![imageviewer](https://user-images.githubusercontent.com/8731829/37601437-007b7b50-2b58-11e8-8b6b-6f85cce4ba2e.png)

**Figure 4:**  Demonstrating image selection functionality.
