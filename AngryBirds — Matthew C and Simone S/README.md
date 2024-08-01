## Game Design Document
## Section 0: Summary

Title: Angry Birds 
Matthew and Simone, roles are listed below in terms of features.
Launch projectile birds to eliminate all the enemies at different positions!

## Section 1: Gameplay

Our game will progress similarly to archery, taking the form of a simplified angry birds as while the player will have to hit the enemy pigs they won’t be able to interact with other blocks. Therefore, the player will be pulling back a slingshot which will determine the angle and velocity of the bird which is being shot at enemy pigs. The player will win the game by eliminating all the enemy pigs with the amount of ammunition they have, which is indicated by the number of birds they have. The player will lose if they do not eliminate all the enemy pigs within the amount of birds they have. While we are not incorporating a scoring system, our way of determining if the user wins or not will be based on the amount of enemy pigs remaining on the scene. The control of this game will be with the mouse; if it is clicked in the general area of where the slingshot is, the user will drag and release at some point in the screen to simulate a slingshot. This will record a vector for the bird’s motion, which will determine the projectile motion the bird will soar in. This implementation incorporates gravity, so the bird will follow a parabolic motion. Then, the bird will collide with the enemy, which will use our collision handlers. The game will have one level and feature a play screen beforehand. If the player loses, they will go back to the start sceen. We will use sprites for our game, which will be layered on top of our polygon that will interact with our other elements for collision detection and other physics engine elements. 

## Section 2: Feature Set
(When we had our meeting with Adam, they told us to create level 5 features. 
We were told that the level 5 features are additional aspects to our game but
don't affect functionality. Because we are a group of 2, the level 5 features are
there to make our game look a bit better, but are not the 4 features we are 
being graded on. I am not sure if Adam mentioned this to the TAs, so I wanted to
leave a little note here about our situation. Thank you!)

- implement a mouse handler for the slingshot (1)
- projectile of the bird based on the mouse handler (1)
- Evaluates collision to deal body removal (2)
- making background and sprites (2)
- Initialize the birds inside the sling for better graphics (3)
- Initialize the enemy pigs in their respective positions for the level (3)
- shot marker (4)
- implement a play screen for the game, using button handler (4)
- Update collision detection to deal with HP changes (5)
- score tracker (5)

We also added some additional features in our games that we did not list in our
feature set. These include a reset button, a game over/win screen, and music. 
These were last minute design choices. While we added an HP parameter to the
enemy body struct, we decided to not continue with the HP idea. This was because
making the user hit the enemy more than once was too difficult and took away from 
our game.

## Section 3: Timeline

Matthew:

Projectile of the Bird 
When working with the projectile of the bird, data from the mouse handler will be evaluated to give the bird in the slingshot an initial velocity at some angle with respect to the x-axis. From there, the newtonian_gravity_force_creator will be called to alter the bird’s velocity across. Lastly, the scene_tick will change the bird’s position as time passes, using this changing velocity.

Level design (Birds) 
This feature will initialize all the birds in the scene. For some number of shots remaining, I will initialize all the birds in their positions in the slingshot. This will also handle the reset button in which the birds need to be added to the scene again. The same applies to the shot marker. 

Sprites and Background
This feature will gather all the file path information for the sprites and background. With this information, I will write init methods for the sprites and background to process the images. This will ease the process of creating different levels and writing in emscripten_init. 

Shot Marker/Game Over
This feature will display the amount of shots the user has left. In the top left corner, there will be an image that tracks the amount of plays the user has left. Once a shot is made, the image will be altered to show one less bird. This will also be used as a marker for the game being over, the condition if the player loses. 

Score Marker 
This feature will display a text at the top of the screen that tracks how many points the user has. If the user successfully hits a pig,
then their score will go up. 

HP Parameter
This will alter our body.c file to create a new struct, enemy_body_t. This will use the inheritance pattern where the two parameters will be a body_t and a double for the HP. This will help us keep track of when we should remove the enemies from the scene based on collisions.


Simone:

- implement a mouse handler, for pulling back slingshot (1) (Done by: 5/25)
	- must include vectors and angle for slingshot which will be stored so that Matt can implement the physics of their projection
	- new mouse handler so that will store the previous x and y position and the new one so that a vector and an angle can be calculated 
	- in sdl_is_done edit it to detect a press and hold as an event so one case when sd mouse event is down (record the initial x and y position) and one inside this case that is if the mouse event is up then return both the initial and final and mouse handler. 
- implement collision handlers(2) (Done by: 6/5)
	- must include vectors and angle for slingshot which will be stored so that Matt can implement the physics of their projection
	- new mouse handler so that will store the previous x and y position and the new one so that a vector and an angle can be calculated 
	- in sdl_is_done edit it to detect a press and hold as an event so one case when sd mouse event is down (record the initial x and y position) and one inside this case that is if the mouse event is up then return both the initial and final and mouse handler. 
- Design of a level, including where the targets are (3) (Done by: 6/2)
	- Writing sdl_init for the assets based on the above 
- Pause/Start screen (4) (Done by: 6/7)
	- changing scene so that it has a new parameter of what background we are on which will whether it is paused 
	- creating new assets for the pause screen
	- Editing the button handler so that it can change the scene
-  Evaluates collision to deal with HP changes and body removal (5) 
	- Edit the physics collision handler so, that instead of just the impulse being added now the HP must also be deducted
		- The HP must be decremented by a certain amount based on the force



Timeline:  
5/25:   
Simone: Mouse Handler (Slingshot)  
Matthew: HP Parameter  
5/27:  
Matthew: Sprites and Background  
6/2:  
Simone: Level Design  
Matthew: Bird Projectile  
Matthew: Level Design  
6/5:  
Simone: Collision Handler  
6/7:  
Matthew: Shot Marker/Game Over  
Simone: Pause Screen  
6/9:  
Matthew: Score Tracker  


## Section 4: Disaster Recovery

Matthew:
If I fall behind, I will firstly communicate that with my partner. Having open communication about the progress on our respective portions of the game is incredibly important for making the game creation process smooth. By communicating, my partner and I can discuss the situation and create next steps to mitigate the work we left behind. Depending on the severity, we may have to change some deadlines around and fix our timeline. If the “disaster” is much bigger, we may have to repivot our game and change some implementations in our game. While I doubt it would ever reach this point, in that situation, we would have to simplify our game according to our timeline. If there are anticipated moments where one of us cannot reach our deadline, just letting my partner know with ample time is important. From there, we can assess the severity to see if we just have to push a deadline back, maybe help the other with conceptual ideas, or restructure some aspect of the game. Ultimately, the most important aspect of disaster recovery will have to be communication. 


Simone:

If I fall behind, firstly, there should be some buffer time in how I have scheduled things out. This is why I left the last not as important task to have the most time, over a week. However, ideally this should not happen and I don’t plan on it. The other thing I will do is make sure to communicate with my partner so that maybe we can work together to debug what is happening and get us back on schedule. Additionally, during the tasks I will make sure to see how I am doing in comparison to my set deadline and if I feel like I am lagging behind I will talk to my partner about it ahead of time, so that if it is a conceptual issue or debugging issue they can maybe lend some advice. This will ensure that we do not get into a situation where hopefully I am behind and my partner has no idea and is also relying on my work. 


