This is a Sweep and Prune algorithm for collision detection.
Developped for a Megadrive homebrew game using  SGDK devkit.

Thanks goes to https://leanrada.com/notes/sweep-and-prune/
a must read for Sweep And Prune understanding !

This Algorithm show some benefits above 20 sprites collisions
Under 20 sprites, you'd better go with classic AABB test !

As of november 2024, the benchmark shows that :

At 10 sprites, AABB => 20% CPU and Sweep And Prune is ~25% CPU.
At 20 sprites, AABB and Sweep And Prune are ~50% CPU.
At 30 sprites, AABB => 90% CPU and Sweep And Prune is 80% CPU.
At 40 sprites, AABB => 140% CPU and Sweep And Prune is 100% CPU.

Probably some improvments can be done for this SAP algorithm !

