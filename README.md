<<<<<<< HEAD
# Sweep and Prune Algorithm for Collision Detection

This is a **Sweep and Prune** algorithm for collision detection, developed for a Megadrive homebrew game using the **SGDK** devkit.

## Acknowledgments

Thanks to [Leanrada's Notes on Sweep and Prune](https://leanrada.com/notes/sweep-and-prune/), a must-read for understanding the Sweep and Prune algorithm!

## Performance Insights

This algorithm shows some benefits when handling more than **10 sprites**. For fewer sprites, it's recommended to stick with the classic **AABB (Axis-Aligned Bounding Box)** test.

### Benchmark Results (as of November 2024)

- **At 10 sprites:**
  - AABB => **20% CPU**
  - Sweep and Prune => **20% CPU**

- **At 20 sprites:**
  - AABB => **50% CPU**
  - Sweep and Prune => **40% CPU**

- **At 30 sprites:**
  - AABB => **90% CPU**
  - Sweep and Prune => **60% CPU**

- **At 40 sprites:**
  - AABB => **140% CPU**
  - Sweep and Prune => **80% CPU**

## Possible Improvements

There are likely improvements that can be made to the **Sweep and Prune** algorithm for better performance, especially by using a BST instead of a linked list, and also in optimizing the sorting algorithm.
