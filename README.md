# GLFW Transparent Blur

The goal here is to create a window that will have the characteristic frosted glass look. Currently it has a basic implementation on Windows using the undocumented function `SetWindowCompositionAttribute` which enables Acrylic effects for the window.

The window is actually just a normal undecorated window with a transparent framebuffer. Thus, moving it, resizing it, and drawing the title bar is handled in software. Showcase below:

![](https://i.stack.imgur.com/1EG8j.gif)
