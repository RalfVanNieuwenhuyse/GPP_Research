# Flow Fields

![ezgif com-video-to-gif](https://github.com/RalfVanNieuwenhuyse/GPP_Research/assets/104133171/d5a6f0c3-2e80-4528-9f25-da1acd22ad72)


## Introduction
### What are flow fields?
Flow fields are a way to guide agents to a destination by following the vectors in a vector field or map.
Agents do this by following the vectors that are in the vector field/map.

### When do we use flow fields
Flow fields are usually used where a lot of agents have to navigate to a targeted position. 
It is most likely that this method is used for RTS games and tycoon games with a lot of NPCs that navigate through the map.

Or to make simulations with particles for VFX like water or snow.

## How to make a flow field
A flow field actualy exist out of 3 fields:
Cost field, integration field and the flow field it self

### The Cost field
Each cell in the grid is represented by a value between 1 and 255. 
By default, these cells have a value of 1. 
All values between 2 and 254 are passable cells that should be avoided if they can. 
And all cells with value 255 are impassable and there the path has to find another way.   
In my implementation, I gave each terrain type a value: normal ground 1, mud 5, and water 255.

Whenever the cost field changes, everything has to be recalculated.

![afbeelding](https://user-images.githubusercontent.com/104133171/212130350-da925236-d533-41dd-9e6b-b27f0b7408af.png)

### The Integation field
This is where the most of the calculations are done.
To generate the integration field we use Dijkstra's algorihm.

Step 1: Reset all the cells to a high value(I chose 60000), I did this in a reset fuction where the other field get reset to. 

Step 2: The end node then gets it total cost set to 0 and gets added to the open list.

Step 3: The current node has been set to the first node on the open list. And gets removed from the list.

step 4:For each neighbor of the current node, their total cost is recalculated by adding the cost of reaching them through the current node. If the new cost is lower than the old cost and the cost is not 255, they are added to the back of the open list.

step 5: This algorithm continues until the open list is empty.

![afbeelding](https://user-images.githubusercontent.com/104133171/212133332-79afede5-42d2-4aa2-8c39-f88ca18129ec.png)

### The Flow field
And now we use the results of the integration field and use it to determine the direction of the vectors in the field.

This happens by ging trough every cell and comparing that cell's value to all of its neighbors to find the one with the lowest value.
I used a struct that contains 9 values: 8 for the directions and one if the direction is a 0 vector.

![afbeelding](https://user-images.githubusercontent.com/104133171/212134729-7b4fea4f-55a7-4e96-a565-f5541af9b1aa.png)

### The Actors
Now we update the actors movement according to the vector of the cell where the actor is standing in.

## Performance
this framework is 32 bit.  

FlowFields
10 X 20 cell's 200 agents: ~7 ms  
10 X 20 cell's 2000 agents: ~12 ms

navmesh a star

1 agent: ~7 ms


PC specs:

Intel(R) Core(TM) i5-9300H CPU @ 2.40GHz   2.40 GHz

16 GB RAM





## sources
https://www.youtube.com/watch?v=ZJZu3zLMYAc&ab_channel=PDN-PasDeNom  
https://leifnode.com/2013/12/flow-field-pathfinding/  
https://www.jdxdev.com/blog/2020/05/03/flowfields/  
https://howtorts.github.io/2014/01/04/basic-flow-fields.html  
https://medium.com/@willdavis84/flow-field-navigation-over-voxel-terrain-d4067b4c0e4b  
