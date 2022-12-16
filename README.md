# SpaceWars
SpaceWars is a basic shmup game made using C++ and SDL.

## Data-Oriented Approach
Goal of the project was to make a basic game with a data-oriented approach. In the start of runtime I create MaxEntityCount number of renderer-, position-, and velocity components. Entities are then created and given an id and enum type (player, bullet, enemy).

When handling logic I try to avoid as many unnecessary checks as possible to improve loop optimization, e.g. I get all the active enemies that are inside the viewport and then do rotation and collision updating, enemies outside the viewport still move towards player but do not need to be rendered and do not collide.

Inside systems data such as position, renderer_textures etc can be retrieved from the component arrays using the id of an entity.

## Collision
Enemy ships are stored in a spatial hash grid, they are stored in grid tiles based on their size and position on screen (can be stored in multiple tiles if they overlap several). When bullets move they check the tiles that they overlap and then do rectangle intersection against enemies found in the same tiles.
