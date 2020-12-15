#pragma once

// If Windows and not in Debug, this will run without a console window
// You can use this to output information when debugging using cout or cerr
#ifdef WIN32 
	#ifndef _DEBUG
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif
#endif

// Defines the number of munchies that should be onscreen
#define MUNCHIECOUNT 50

// Defines the amount of ghosts that are onscreen
#define GHOSTCOUNT 4

// Just need to include main header file
#include "S2D/S2D.h"

// Reduces the amount of typing by including all classes in S2D namespace
using namespace S2D;

struct Player
{
	bool dead;
	float speedMultiplier;
	int currentFrameTime;
	int direction;
	int frame;
	Rect* sourceRect;
	Texture2D* texture;
	Vector2* position;
};

struct Enemy
{
	bool dead;
	int currentFrameTime;
	int frame;
	Rect* rect;
	Rect* sourceRect;
	Texture2D* texture;
	Vector2* position;
};

struct MovingEnemy
{
	Vector2* position;
	Texture2D* texture;
	Rect* sourceRect;
	int direction;
	float speed;
};

struct Maze
{
	Rect* rect;
	Rect* sourceRect;
	Texture2D* texture;
};

// Declares the Pacman class which inherits from the Game class.
// This allows us to overload the Game class methods to help us
// load content, draw and update our game.
class Pacman : public Game
{
private:
	//Input methods
	void Input(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState);

	//Check methods
	void CheckPaused(Input::KeyboardState* state, Input:: Keys pauseKey);
	void CheckViewportCollision();
	void CheckGhostCollisions();
	void CheckMunchieCollisions();

	// Update methods
	void UpdatePacman(int elapsedTime);
	void UpdateMunchies(Enemy*, int elapsedTime);
	void UpdateCherry(int elapsedTime);
	void UpdateGhost(MovingEnemy*, int elapsedTime);

	// Data to represent Pacman
	Player* _pacman;

	// Data to represent Munchies
	Enemy* _munchies[MUNCHIECOUNT];

	// Data to represent Ghosts
	MovingEnemy* _ghosts[GHOSTCOUNT];

	// Data to represent Cherry
	Enemy* _cherry;

	// Position for String
	Vector2* _stringPosition;

	//Constant data for Game Variables
	const float _cPacmanSpeed;
	const int _cPacmanFrameTime;
	const int _cMunchieFrameTime;
	const int _cCherryFrameTime;

	// Data for Menu
	Texture2D* _menuBackground;
	Rect* _menuRectangle;
	Vector2* _menuStringPosition;
	bool _paused;
	bool _start;

	// Data for Sounds
	SoundEffect* _pop;

	// Key press data
	bool _pKeyDown;
	bool _spaceKeyDown;
	bool _rKeyDown;

public:
	/// <summary> Constructs the Pacman class. </summary>
	Pacman(int argc, char* argv[]);

	/// <summary> Destroys any data associated with Pacman class. </summary>
	virtual ~Pacman();

	/// <summary> All content should be loaded in this method. </summary>
	void virtual LoadContent();

	/// <summary> Called every frame - update game logic here. </summary>
	void virtual Update(int elapsedTime);

	/// <summary> Called every frame - draw game here. </summary>
	void virtual Draw(int elapsedTime);
};