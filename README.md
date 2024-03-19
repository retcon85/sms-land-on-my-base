# Land on my base (and tell me that you love me)

A little homage to a number of classic video games including [Lunar Lander](https://en.wikipedia.org/wiki/Lunar_Lander_(video_game_genre)) and derivatives like _Marslander_ for the BBC Micro.

The idea is to land the craft safely on the landing pad, which is only just big enough to accept it.

The physics is mildly realistic, simulating a gravitional constant, and a cheeky "solar wind" which blows the craft off course to the left or right.

The controls are simple:
- Press either action button to apply thrust to the lander
- Use the left or right direction keys to change the thrust angle.

There is a small indicator next to the craft which shows the direction from which thrust will be applied. For example, when the indicator is directly underneath the craft, thrust will be applied vertically upwards, sending it away from the surface. Conversely, when the indicator is 90 degrees to the left of the craft, thrust will be applied entirely in a left to right direction.

Gravity and solar wind vary from level to level, and you can capture the level code at the start of each level to prove to your friends how much of an astronaut boss you are.

I made this game with the incredible [Devkitsms](https://github.com/sverx/devkitSMS) for the [2024 SMSPower! coding competition](https://www.smspower.org/forums/19973-Competitions2024) and I made it in a hurry.

Stuff I still want to add:

- A title screen
- Some music
- Some box art
- A more sophisticated HUD
- Simulated fuel levels
- A score which accumulates
- Automatic zooming in and out of terrain like in the [Lunar Lander](https://en.wikipedia.org/wiki/Lunar_Lander_(video_game_genre)) arcade game.
